/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  aci_lst
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
|  Purpose :  Process lists in ACI
+----------------------------------------------------------------------------- 
*/ 

#ifndef ACI_LST_C
#define ACI_LST_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/

#include "aci_lst.h"
#include "aci_mem.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*==== TYPES ======================================================*/


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : new_list           |
+--------------------------------------------------------------------+

  PURPOSE : create a new list.
*/

GLOBAL T_ACI_LIST *new_list (void)
{
  T_ACI_LIST *new_elem = NULL;

  ACI_MALLOC(new_elem, sizeof(T_ACI_LIST));
  new_elem->msg = NULL;
  new_elem->next = NULL;

  return (new_elem);
}

GLOBAL USHORT get_list_count (T_ACI_LIST *list)
{
  T_ACI_LIST *curr;
  USHORT count = 0;

  curr = list;
  while (curr NEQ NULL)
  {
    if (curr->msg NEQ NULL)
      count++;

    curr = curr->next;
  }

  return (count);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : insert_list        |
+--------------------------------------------------------------------+

  PURPOSE : insert a new element in an existing list.
            - list: pointer to the list where the element is to be added
            - buffer: pointer to the buffer containing the datas that are to
                      be written in the new element.
            - buffer_len: length of the datas to be stored.

*/

GLOBAL BOOL insert_list (T_ACI_LIST *list, void *buffer)
{
  T_ACI_LIST *current;
  T_ACI_LIST *new_elem;

  if (list EQ NULL)
    return (FALSE);

  if (list->msg EQ NULL)
  {
    list->msg = buffer;
    return (TRUE);
  }

  ACI_MALLOC(new_elem, sizeof(T_ACI_LIST));

  new_elem->msg = buffer;
  new_elem->next = NULL;

  current = list;
  while (current->next NEQ NULL)
  {
    current = (T_ACI_LIST *)current->next;
  }
  current->next = new_elem;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : find_element       |
+--------------------------------------------------------------------+

  PURPOSE : find an element in a list:
            - search_list: list to be scanned.
            - criterium: search criterium. (source Id for instance)
            - test_criterium: function(criterium, element) that returns TRUE if
                              the criterium is found in the element.

*/

GLOBAL void *find_element (T_ACI_LIST *search_list,
                           UBYTE criterium,
                           T_LIST_FIND_FCT test_criterium)
{
  T_ACI_LIST    *current;

  if ((search_list EQ NULL) OR (search_list->msg EQ NULL))
    return NULL;

  current = search_list;

  do
  {
    if (current->msg NEQ NULL)
    {
      if (test_criterium(criterium, current->msg))
        return (current->msg);
      else
        current = current->next;
    }
  }
  while( current NEQ NULL );

  return NULL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : psi_find_element       |
+--------------------------------------------------------------------+

  PURPOSE : find an element in a list:
            - search_list: list to be scanned.
            - criterium: search criterium. (source Id for instance)
            - test_criterium: function(criterium, element) that returns TRUE if
                              the criterium is found in the element.

*/
#ifdef FF_PSI
GLOBAL void *psi_find_element (T_ACI_LIST *search_list,
                           U32 criterium,
                           T_LIST_FIND_FCT_PSI test_criterium)
{
  T_ACI_LIST    *current;

  if ((search_list EQ NULL) OR (search_list->msg EQ NULL))
    return NULL;

  current = search_list;

  do
  {
    if (current->msg NEQ NULL)
    {
      if (test_criterium(criterium, current->msg))
        return (current->msg);
      else
        current = current->next;
    }
  }
  while( current NEQ NULL );

  return NULL;
}
#endif /*FF_PSI*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : find_next_element  |
+--------------------------------------------------------------------+

  PURPOSE : find the next element in a list:
            - search_list: list to be scanned.
            - previous element
            - criterium: search criterium. (source Id for instance)
            - test_criterium: function(criterium, element) that returns TRUE if
                              the criterium is found in the element.

*/
GLOBAL void *find_next_element (T_ACI_LIST *search_list,
                                void *prev_elem,
                                UBYTE criterium,
                                T_LIST_FIND_FCT test_criterium)
{
  T_ACI_LIST    *current;

  if ((search_list EQ NULL) OR (search_list->msg EQ NULL))
    return NULL;

  current = search_list;

  if (prev_elem NEQ NULL)
  {
    while (current NEQ NULL)
    {
      if (current->msg EQ prev_elem)
        break;

      current = current->next;
    }
    if ((current EQ NULL) OR (current->next EQ NULL))
      return (NULL);

    current = current->next;
  }

  do
  {
    if( test_criterium(criterium, current->msg) )
      return (current->msg);
    else
      current = current->next;
  }
  while( current NEQ NULL );

  return NULL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : find_next_element  |
+--------------------------------------------------------------------+

  PURPOSE : find the next element in a list:
            - search_list: list to be scanned.
            - previous element
            - criterium: search criterium. (source Id for instance)
            - test_criterium: function(criterium, element) that returns TRUE if
                              the criterium is found in the element.

*/
#ifdef FF_PSI
GLOBAL void *psi_find_next_element (T_ACI_LIST *search_list,
                                void *prev_elem,
                                U32 criterium,
                                T_LIST_FIND_FCT_PSI test_criterium)
{
  T_ACI_LIST    *current;

  if ((search_list EQ NULL) OR (search_list->msg EQ NULL))
    return NULL;

  current = search_list;

  if (prev_elem NEQ NULL)
  {
    while (current NEQ NULL)
    {
      if (current->msg EQ prev_elem)
        break;

      current = current->next;
    }
    if ((current EQ NULL) OR (current->next EQ NULL))
      return (NULL);

    current = current->next;
  }

  do
  {
    if( test_criterium(criterium, current->msg) )
      return (current->msg);
    else
      current = current->next;
  }
  while( current NEQ NULL );

  return NULL;
}
#endif /*FF_PSI*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : remove_element     |
+--------------------------------------------------------------------+

  PURPOSE : remove a element from the list.

*/
GLOBAL void *remove_element (T_ACI_LIST *search_list,
                             UBYTE criterium,
                             T_LIST_FIND_FCT test_criterium)
{
  T_ACI_LIST *previous = NULL,
             *current  = search_list,
             *next     = NULL;
  void       *msg = NULL;

  current = search_list;
  while (current NEQ NULL)
  {
    if (test_criterium (criterium, current->msg))
    {
      msg = current->msg;
      break;
    }
    previous = current;
    current = current->next;
  }

  if (current EQ NULL)
    return (NULL);

  if (msg EQ NULL)
    return (NULL);

  /* the first element is to delete */
  if (previous EQ NULL)
  {
    /* the header contains the element */
    if (current->next EQ NULL)
    {
      /* one element in list */
      current->msg = NULL;
    }
    else
    {
      /* actually deletes the second element and redirects the pointers */
      next = current->next;
      current->msg = next->msg;
      current->next = next->next;

      ACI_MFREE (next);    }
  }
  else
  {
    previous->next = current->next;

    ACI_MFREE (current);
  }

  return (msg);
}

#ifdef FF_PSI
GLOBAL void *psi_remove_element (T_ACI_LIST *search_list,
                             U32 criterium,
                             T_LIST_FIND_FCT_PSI test_criterium)
{
  T_ACI_LIST *previous = NULL,
             *current  = search_list,
             *next     = NULL;
  void       *msg = NULL;

  current = search_list;
  while (current NEQ NULL)
  {
    if (test_criterium (criterium, current->msg))
    {
      msg = current->msg;
      break;
    }
    previous = current;
    current = current->next;
  }

  if (current EQ NULL)
    return (NULL);

  if (msg EQ NULL)
    return (NULL);

  /* the first element is to delete */
  if (previous EQ NULL)
  {
    /* the header contains the element */
    if (current->next EQ NULL)
    {
      /* one element in list */
      current->msg = NULL;
    }
    else
    {
      /* actually deletes the second element and redirects the pointers */
      next = current->next;
      current->msg = next->msg;
      current->next = next->next;

      ACI_MFREE (next);    }
  }
  else
  {
    previous->next = current->next;

    ACI_MFREE (current);
  }

  return (msg);
}

#endif /*FF_PSI*/


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)           MODULE  : ACI_LST               |
| STATE   : code                     ROUTINE : remove_first_element  |
+--------------------------------------------------------------------+

  PURPOSE : get and remove the first element from the list

*/
GLOBAL void *remove_first_element (T_ACI_LIST *search_list)
{
  T_ACI_LIST *next = NULL;
  void       *msg  = NULL;

  if ((search_list EQ NULL) OR (search_list->msg EQ NULL))
    return (NULL);

  msg = search_list->msg;
  search_list->msg = NULL;

  if (search_list->next EQ NULL)
    return (msg);

  next = search_list->next;
  search_list->next = next->next;
  search_list->msg = next->msg;

  ACI_MFREE (next);

  return (msg);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : get_first_element  |
+--------------------------------------------------------------------+

  PURPOSE : get the next element from the list
            - search_list: list to be scanned.
            - previous element
*/

GLOBAL void *get_next_element (T_ACI_LIST *search_list,
                               void       *prev_elem)
{
  T_ACI_LIST    *current;

  if ((search_list EQ NULL) OR (search_list->msg EQ NULL))
    return NULL;

  if (prev_elem EQ NULL)
  {
    /* return first element  */
    return (search_list->msg);
  }

  current = search_list;

  /* search the previous element */
  while (current NEQ NULL)
  {
    if (current->msg EQ prev_elem)
    {
      break;
    }
    current = current->next;
  }

  /* last entry found or previous entry not found */
  if ((current EQ NULL) OR (current->next EQ NULL))
  {
    return (NULL);
  }

  return (current->next->msg);
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST             |
| STATE   : code                        ROUTINE : insert_shift_list   |
+---------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL BOOL insert_shift_list (T_ACI_LIST *list, void *buffer)
{
  T_ACI_LIST *current;
  T_ACI_LIST *new_elem;
  void *old_buf;

  if (list EQ NULL)
    return (FALSE);

  if (list->msg EQ NULL)
  {
    list->msg = buffer;
    return (TRUE);
  }
  else if (list->next EQ NULL)
    return (FALSE);

  current = list;

  old_buf = list->msg;
  list->msg = list->next->msg;

  new_elem = list->next;
  list->next = list->next->next;
  

  new_elem->msg = buffer;
  new_elem->next = NULL;
  
  while (current->next NEQ NULL)
  {
    current = (T_ACI_LIST *)current->next;
  }
  current->next = new_elem;
  
  ACI_MFREE (old_buf);
  
  return TRUE;
}
/**********************************************************************/

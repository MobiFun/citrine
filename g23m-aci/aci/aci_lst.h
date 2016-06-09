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

#ifndef ACI_LST_H
#define ACI_LST_H


/*==== TYPES ======================================================*/

typedef struct ACI_LIST_S
{
  void               *msg;
  struct ACI_LIST_S  *next;
} T_ACI_LIST;

typedef BOOL  T_LIST_FIND_FCT (UBYTE criterium, void *elem);
typedef BOOL  T_LIST_FIND_FCT_PSI (U32 criterium, void *elem);

/*==== PROTOTYPES =================================================*/

/*==== EXPORT =====================================================*/

EXTERN USHORT get_list_count      (T_ACI_LIST *list);
EXTERN T_ACI_LIST *new_list       (void);
EXTERN BOOL insert_list           (T_ACI_LIST       *list, 
                                   void             *buffer);
EXTERN BOOL insert_shift_list (T_ACI_LIST *list, void *buffer);

EXTERN void *get_next_element     (T_ACI_LIST       *search_list, 
                                   void             *prev_elem);

EXTERN void *find_next_element    (T_ACI_LIST       *search_list, 
                                   void             *prev_elem,
                                   UBYTE            criterium, 
                                   T_LIST_FIND_FCT  test_criterium);

EXTERN void *find_element         (T_ACI_LIST       *search_list, 
                                   UBYTE            criterium, 
                                   T_LIST_FIND_FCT  test_criterium);


EXTERN void *remove_first_element (T_ACI_LIST       *search_list);
EXTERN void *remove_element       (T_ACI_LIST       *search_list, 
                                   UBYTE            criterium, 
                                   T_LIST_FIND_FCT  test_criterium);
#ifdef FF_PSI
EXTERN void *psi_find_next_element    (T_ACI_LIST       *search_list, 
                                   void             *prev_elem,
                                   U32           criterium, 
                                   T_LIST_FIND_FCT_PSI  test_criterium);
EXTERN void *psi_find_element (T_ACI_LIST *search_list,
                           U32 criterium,
                           T_LIST_FIND_FCT_PSI test_criterium);
EXTERN void *psi_remove_element       (T_ACI_LIST       *search_list, 
                                   U32            criterium, 
                                   T_LIST_FIND_FCT_PSI test_criterium);
#endif /*FF_PSI*/

#endif

/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  
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
|  Purpose :  Contains macros for the unified implementation of the
|             Protocol Stack
+----------------------------------------------------------------------------- 
*/ 

#ifndef MACDEF_H
#define MACDEF_H

/*
 * Get/Set state for one incarnation. Init state has to be called first.
 */
#undef GET_STATE
#undef SET_STATE

#ifdef TRACE_STATE
  #define INIT_STATE(P,S)     ENTITY_DATA->P name = SERVICE_NAME_##P;         \
                              ENTITY_DATA->P state = S;                       \
                              ENTITY_DATA->P state_name = #S;

  #define SET_STATE(P,S)      {ENTITY_DATA->P state = S;                      \
                               vsi_o_ttrace (VSI_CALLER                       \
                                             TC_STATE,                        \
                                             #P ":%s -> " #S,                 \
                                             ENTITY_DATA->P state_name );     \
                               ENTITY_DATA->P state_name = #S;}

  #define GET_STATE(P)        (vsi_o_ttrace (VSI_CALLER                       \
                                             TC_STATE,                        \
                                #P ":%s", ENTITY_DATA->P state_name ),        \
                                ENTITY_DATA->P state)

#else
  #define INIT_STATE(P,S)     {ENTITY_DATA->P state = S;}
  #define SET_STATE(P,S)      {ENTITY_DATA->P state = S;}
  #define GET_STATE(P)        (ENTITY_DATA->P state)
#endif /* TRACE_STATE */


/*
 * MFREE_DESC() is used to free the given descriptor, along with all following
 * descriptors.
 *
 * Parameters:  d  -- "pointer" to the descriptor, either of type T_desc *, or
 *                    an ULONG
 */
#define MFREE_DESC(d)         { T_desc *desc = (T_desc *)d;                   \
                                T_desc *next_desc;                            \
                                while (desc NEQ NULL)                         \
                                {                                             \
                                  next_desc = (T_desc *)desc->next;           \
                                  MFREE (desc);                               \
                                  desc = next_desc;                           \
                                }                                             \
                              }


/*
 * PFREE_DESC() is used to free the given primitive, along with all
 * descriptors in the descriptor list of the primitive.
 *
 * Parameters:  p  -- pointer to the primitive containing a descriptor list
 *                    (if it is an ULONG, it has to be casted to the pointer
 *                    because the type cannot be determined by the macro!)
 */
#define PFREE_DESC(p)         { MFREE_DESC ((p)->desc_list.first);            \
                                PFREE ((p));                                  \
                              }


/*
 * MFREE_DESC() is used to free the given descriptor, along with all following
 * descriptors.
 *
 * Parameters:  d  -- "pointer" to the descriptor, either of type T_desc *, or
 *                    an ULONG
 */
#define MFREE_DESC2(d)        { T_desc2 *desc2 = (T_desc2 *)d;                   \
                                T_desc2 *next_desc2;                            \
                                while (desc2 NEQ NULL)                         \
                                {                                             \
                                  next_desc2 = (T_desc2 *)desc2->next;           \
                                  MFREE (desc2);                               \
                                  desc2 = next_desc2;                           \
                                }                                             \
                              }


/*
 * PFREE_DESC() is used to free the given primitive, along with all
 * descriptors in the descriptor list of the primitive.
 *
 * Parameters:  p  -- pointer to the primitive containing a descriptor list
 *                    (if it is an ULONG, it has to be casted to the pointer
 *                    because the type cannot be determined by the macro!)
 */
#define PFREE_DESC2(p)        { MFREE_DESC2 ((p)->desc_list2.first);            \
                                PFREE ((p));                                   \
                              }


#endif /* !MACDEF_H */

/*
+------------------------------------------------------------------------------
|  File:       idle.h
+------------------------------------------------------------------------------
|                 Copyright Texas Instruments 2002
|                 All rights reserved.
|
+------------------------------------------------------------------------------
| Purpose:     Definitions for the IDLE entity.
| $Identity:$
+------------------------------------------------------------------------------
*/

#ifndef IDLE_H
#define IDLE_H

/*==== INCLUDES ==============================================================*/

/*==== CONSTS ================================================================*/

#ifdef VSI_CALLER
 #undef VSI_CALLER
 #define VSI_CALLER        idle_handle,
 #define VSI_CALLER_SINGLE idle_handle
#endif

#define hCommTST          idle_hCommTST   /* Communication handle to TST       */

#define IDLE_REQ   0x00000010
#define IDLE_CNF   0x00010010


#ifndef __T_IDLE_CNF__
#define __T_IDLE_CNF__
typedef struct
{
  U8                        dummy;                       /*<  0:  1>                              */
} T_IDLE_CNF;
#endif

#ifndef __T_IDLE_REQ__
#define __T_IDLE_REQ__
typedef struct
{
  U8                        dummy;                       /*<  0:  1>                              */
} T_IDLE_REQ;
#endif

/*==== TYPES =================================================================*/

/*==== EXPORTS ===============================================================*/

#ifdef IDLE_PEI_C

/* Communication handles */
T_HANDLE                 idle_handle;
T_HANDLE                 hCommTST = VSI_ERROR;

#else /* IDLE_PEI_C */

extern T_HANDLE          idle_handle;
extern T_HANDLE          hCommTST;

#endif /* IDLE_PEI_C */

#endif /* IDLE_H */

/*==== END OF FILE ===========================================================*/

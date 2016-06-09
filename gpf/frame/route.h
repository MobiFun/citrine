/* 
+----------------------------------------------------------------------------- 
|  File:       route.h
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
|  Purpose :  Type definitions and prototypes for routing.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __ROUTE_H__
#define __ROUTE_H__

#define RT_DUPLICATE    0x01
#define RT_REDIRECT     0x02
#define RT_DESTROY      0x04
#define RT_COMMAND_MASK (RT_DUPLICATE|RT_REDIRECT|RT_DESTROY)

#define RT_CLEAR_ENTRY         0x10
#define RT_ALL_DESTINATION     0x20

typedef enum { RT_STORE = 1, RT_DELETE } RT_ENTRY;
typedef enum { RT_PRIMITIVE_TYPE = 1, RT_SIGNAL_TYPE, RT_TIMEOUT_TYPE } RT_MESSAGE_TYPE;

typedef struct _T_ROUTING_ENTRY
{
  T_HANDLE  SndTaskHandle;    /* sender of a mesage */
  T_HANDLE	OldDestComHandle; /* original destination queue handle */
  T_HANDLE	OldDestTaskHandle;/* original destination task handle */
  T_HANDLE	NewDestComHandle;	/* new destination queue handle */
  RT_MESSAGE_TYPE MsgType;    /* message type (primitive,timeout,signal) */
  ULONG	  opcStatus;			    /* mask for messages to be routed */
  ULONG 	opcMask;			      /* relevant bits in opcStatus */
  int     Command;			      /* redirect/duplicate */
  char ExtDest[RESOURCE_NAMELEN];
  struct _T_ROUTING_ENTRY * pNextEntry;
  struct _T_ROUTING_ENTRY * pPrevEntry;
} T_FRM_ROUTING_TABLE_ENTRY;

#define RT_OK        0
#define RT_ERROR   (-1)
#define RT_NO_MEM  (-2)

#define ROUTE_ALL  (T_HANDLE)0xFFFF

#define RT_CLEAR_TOKEN    "CLEAR"
#define RT_ALL_TOKEN      "ALL"

#define RT_SIGNAL_TOKEN   "S"
#define RT_TIMEOUT_TOKEN  "T"

GLOBAL SHORT rt_Init (void);
GLOBAL SHORT rt_RoutingModify ( T_HANDLE TaskHandle, char *Command, char *String);
GLOBAL SHORT rt_ConnectRoute ( T_HANDLE TaskHandle, char *Destination);
GLOBAL SHORT rt_DisconnectRoute ( T_HANDLE TaskHandle, char *Destination);
GLOBAL SHORT rt_Route ( T_HANDLE SndTaskHandle, T_HANDLE RcvComHandle, USHORT Prio, ULONG Suspend, OS_QDATA *Msg );
GLOBAL SHORT rt_RouteRead ( T_HANDLE TaskHandle, char *token );               
GLOBAL void rt_ExtPrimitive ( T_HANDLE TaskHandle, T_HANDLE DestComHandle, T_HANDLE OrgDestComHandle, char *ExtDest, OS_QDATA *Msg ) ;
GLOBAL SHORT rt_isolate_entity ( T_HANDLE caller, char *entity );
GLOBAL int rt_desclist_to_sdu ( T_HANDLE caller, T_HANDLE dst, T_PRIM_HEADER *prim, T_PRIM_HEADER **sdu_prim );
               

#endif /* ROUTE_H */

/* 
+------------------------------------------------------------------------------
|  File:       cust_os.h
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
|  Purpose :  Definitions for the communication between L1 and L23 entities. 
|             This interface file provides an abstraction of the VSI 
|             interface, e.g. to avoid that macros like PALLOC need to be used
|             by the customer.
+----------------------------------------------------------------------------- 
*/ 
#ifndef CUST_OS_H
#define CUST_OS_H

/*==== INCLUDES =============================================================*/

/*==== CONSTS ===============================================================*/

#define OSX_OK     0
#define OSX_ERROR -1

#define L1S_TRACE_DISABLE           "L1S_TRACE_DISABLE"
#define L1S_TRACE_ENABLE            "L1S_TRACE_ENABLE"
#define NO_SPECIAL_MPHC_RXLEV_REQ   "NO_SPEC_MPHC_RXLEV_REQ"

/*==== TYPES ================================================================*/

/* Necessary defintions*/
typedef enum
{
  L1_QUEUE,    /* internal L1 communication          */
  DL_QUEUE,    /* L1->DL                             */
  RR_QUEUE,    /* L1->RR                             */
  GRR_QUEUE,   /* L1->GRR                            */
  LLC_QUEUE,   /* L1->LLC e.g. ciphering via CCI     */
  SNDCP_QUEUE, /* L1->SNDCP e.g. compression via CCI */
  GPF_ACI_QUEUE,   /* L1->ACI                        */
  MAX_OSX_QUEUE
} T_ENUM_OS_QUEUE;

typedef T_ENUM_OS_QUEUE ENUM_OS_QUEUE;

typedef struct {
       char dummychar;
} DummyStruct;

typedef struct xSignalHeaderStruct
{
  int           SignalCode;
  int           _dummy1;
  int           _dummy2;
  int           _dummy3;
  DummyStruct   *SigP;
  int           _dummy4;
} xSignalHeaderRec;

typedef struct
{
  int caller;
  int queue_handle;
  unsigned short queue_type;
} T_OSX_REGISTER;


/*==== EXPORTS ==============================================================*/

/*
 * =================================================
 * functional interface to the GPF frame environment
 * =================================================
 */

/*
 * initialize queue_type to queue_handle conversion table
 */
void _osx_init ( void );

/*
 * dynamic osx configuration
 */
int _osx_config ( const char *config );

/*
 * open an cust_os communication channel
 */
#ifdef _OSX_ON_OS_
int               _osx_open       (int,unsigned short,int,int);
#else
int               _osx_open       (int,unsigned short,int);
#endif

/*
 * allocation of a primitive container with a specific size e.g. sizeof (T_XXX_REQ)
 */
xSignalHeaderRec* osx_alloc_prim   (unsigned long);

xSignalHeaderRec* int_osx_alloc_prim    (int,unsigned long, int);

/*
 * allocation of a memory section with a specific size
 */
void*             osx_alloc_mem    (unsigned long);

void*             int_osx_alloc_mem     (int,unsigned long);

/*
 * deallocation of one primitive container
 */
void              osx_free_prim    (xSignalHeaderRec*);

void              int_osx_free_prim     (int,xSignalHeaderRec*);

/*
 * deallocation of one memory section
 */
void              osx_free_mem     (void*);

void              int_osx_free_mem      (int,void*);

/*
 * reception of a primitive. This function should be called from a thread. It suspends
 * the thread on an empty queue. Queue is specified by an idetifier, 
 */
xSignalHeaderRec* osx_receive_prim (T_ENUM_OS_QUEUE);

/*
 * reception of a primitive. This function should be called from a thread. It suspends
 * the thread on an empty queue. Queue is specified by its handle
 */
xSignalHeaderRec *int_osx_receive_prim  (int,int);

/*
 * send a primtive to a queue specified by an identifier (no frame handle!)
 */
void              osx_send_prim   (xSignalHeaderRec*, T_ENUM_OS_QUEUE);

/*
 * send a primtive to a queue specified by a handle
 */
void              int_osx_send_prim  (int,xSignalHeaderRec*, int);

/*
 * send a signal with a special signal code (ULONG) and a pointer to a specific memory
 * area to a queue specified by an identifier
 */
void              osx_send_sig  (unsigned long, void *, T_ENUM_OS_QUEUE);

/*
 * send a signal with a special signal code (ULONG) and a pointer to a specific memory
 * area to a queue specified by its handle
 */
void               int_osx_send_sig  (int,unsigned long,void*,int);

/*
 * ================================================
 * customizations for the L1 from Texas Instruments
 * ================================================
 */
#define L1C1_QUEUE  L1_QUEUE
#define DLL1_QUEUE  DL_QUEUE
#define RRM1_QUEUE  RR_QUEUE
#define GRRM1_QUEUE GRR_QUEUE
#define BACK_QUEUE  L1_QUEUE
#define ACI_QUEUE   GPF_ACI_QUEUE

#define os_alloc_sig   osx_alloc_prim
#define os_free_sig    osx_free_prim
#define os_send_sig    osx_send_prim
#define os_receive_sig osx_receive_prim

#define MEM_BUF_SIZE(SIG_PAR_SIZE) (SIG_PAR_SIZE + sizeof (xSignalHeaderRec))

#endif /* CUST_OS_H */

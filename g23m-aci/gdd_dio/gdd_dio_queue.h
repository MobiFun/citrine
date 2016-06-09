/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_queue.h
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
|  Purpose  : Definitions for gdd_dio_queue.c
+-----------------------------------------------------------------------------
*/

#ifndef GDD_DIO_QUEUE_H
#define GDD_DIO_QUEUE_H

/*==== INCLUDES =============================================================*/

#include "gdd.h"

#include "p_dio.h"


/*==== DEFINITIONS ==========================================================*/

/**
 * Maximum numbers of simultaneous DIO buffers provided by PSI.
 * Might be increased at one stage in the future.
 */
#define GDD_MAX_DIO_BUF   2

/**
 * Definition of buffer queue
 */
typedef struct
{
  T_dio_buffer * elems[GDD_MAX_DIO_BUF];
  U16 next_in;
  U16 next_out;  
} T_GDD_BUF_QUEUE;


/*==== EXPORTS ===============================================================*/

/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_queue_clear
+------------------------------------------------------------------------------
| Description : Clear a queue
|
| Parameters  : queue            - pointer to queue to be cleared
|
+------------------------------------------------------------------------------
*/
EXTERN void gdd_dio_queue_clear(T_GDD_BUF_QUEUE *queue);


/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_queue_is_empty
+------------------------------------------------------------------------------
| Description : Check if a queue is empty
|
| Parameters  : queue            - pointer to queue to be checked
|
| Return      : TRUE if queue is empty, FALSE otherwise
+------------------------------------------------------------------------------
*/
EXTERN BOOL gdd_dio_queue_is_empty(const T_GDD_BUF_QUEUE *queue);


/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_queue_is_full
+------------------------------------------------------------------------------
| Description : Check if a queue is full
|
| Parameters  : queue            - pointer to queue to be checked
|
| Return      : TRUE if queue is full, FALSE otherwise
+------------------------------------------------------------------------------
*/
EXTERN BOOL gdd_dio_queue_is_full(const T_GDD_BUF_QUEUE *queue);


/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_enqueue
+------------------------------------------------------------------------------
| Description : Enqueue an item into the queue
|
| Parameters  : elem             - item to be enqueued
|               queue            - pointer to queue to be modified
|               pos              - to return the position of the enqueued item
|
| Return      : TRUE if item is enqueued, FALSE if the queue is already full
+------------------------------------------------------------------------------
*/
EXTERN BOOL gdd_dio_enqueue(T_dio_buffer *elem, T_GDD_BUF_QUEUE *queue, /*out*/U16 *pos);


/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_dequeue
+------------------------------------------------------------------------------
| Description : Dequeue an item into the queue
|
| Parameters  : queue            - pointer to queue to be modified
|               elem             - to return the dequeued item
|
| Return      : TRUE if item is dequeued, FALSE if the queue is empty
+------------------------------------------------------------------------------
*/
EXTERN BOOL gdd_dio_dequeue(T_GDD_BUF_QUEUE *queue, /*out*/T_dio_buffer **elem);

/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_queue_peek_next_for_dequeue
+------------------------------------------------------------------------------
| Description : Retrieves the element that is next to dequeue without actually
|               dequeuing it.
|
| Parameters  : queue            - pointer to queue
|               pos              - position of element
|               elem             - to output element value
|
| Return      : TRUE if item is available, FALSE if the queue is empty
+------------------------------------------------------------------------------
*/
EXTERN BOOL gdd_dio_queue_peek_next_for_dequeue
(const T_GDD_BUF_QUEUE *queue, T_dio_buffer **elem);

/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_queue_peek
+------------------------------------------------------------------------------
| Description : Retrieve element at specified position
|
| Parameters  : queue            - pointer to queue
|               pos              - position of element
|               elem             - to output element value
|
| Attention   : This function does not do bounds checking
+------------------------------------------------------------------------------
*/
EXTERN void gdd_dio_queue_peek(const T_GDD_BUF_QUEUE *queue, U16 pos, /*out*/T_dio_buffer **elem);

#endif /* !GDD_DIO_QUEUE_H */


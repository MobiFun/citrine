/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_queue.c
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
|  Purpose  : Implements the queue functions for the DIO buffer FIFO
+-----------------------------------------------------------------------------
*/


#define ENTITY_GDD_DIO

/*==== INCLUDES =============================================================*/

#include "typedefs.h"   /* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */

#include <string.h>  /* for memset */

#include "gdd_dio_queue.h"

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/


GLOBAL void gdd_dio_queue_clear(T_GDD_BUF_QUEUE *queue)
{
  queue->next_in = 0;
  queue->next_out = 0;
  memset(queue->elems, 0, sizeof(T_dio_buffer *) * GDD_MAX_DIO_BUF);
}


BOOL gdd_dio_queue_is_empty(const T_GDD_BUF_QUEUE *queue)
{
  return (queue->next_in == queue->next_out) &&
         (0 == queue->elems[queue->next_out]);
}


GLOBAL BOOL gdd_dio_queue_is_full(const T_GDD_BUF_QUEUE *queue)
{
  return (queue->next_in == queue->next_out) && 
          !(0 == queue->elems[queue->next_out]);
}


GLOBAL BOOL gdd_dio_dequeue(T_GDD_BUF_QUEUE *queue, T_dio_buffer **elem)
{
  if(gdd_dio_queue_is_empty(queue))
  {
    return FALSE;
  }
  else
  {
    (*elem) = queue->elems[queue->next_out];
    queue->elems[queue->next_out] = 0;
    ++(queue->next_out);
    if(queue->next_out EQ GDD_MAX_DIO_BUF)
      queue->next_out = 0;
    return TRUE;
  }
}

GLOBAL BOOL gdd_dio_queue_peek_next_for_dequeue
(const T_GDD_BUF_QUEUE *queue, /*out*/T_dio_buffer **elem)
{
  if(gdd_dio_queue_is_empty(queue))
  {
    return FALSE;
  }
  else
  {
    (*elem) = queue->elems[queue->next_out];
    return TRUE;
  }
}

GLOBAL BOOL gdd_dio_enqueue(T_dio_buffer *elem, T_GDD_BUF_QUEUE *queue, /*out*/U16 *pos)
{
  if(gdd_dio_queue_is_full(queue))
  {
    return FALSE;
  }
  else
  {
    queue->elems[queue->next_in] = elem;
    (*pos) = queue->next_in;
    ++(queue->next_in);
    if(queue->next_in EQ GDD_MAX_DIO_BUF)
      queue->next_in = 0;
    return TRUE;
  }
}


GLOBAL void gdd_dio_queue_peek(const T_GDD_BUF_QUEUE *queue, U16 pos, /*out*/T_dio_buffer **elem)
{
  (*elem) = queue->elems[pos];
}

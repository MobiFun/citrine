/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_drxf.c
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
|  Purpose  : This modul is part of the entity gdd_dio and implements the
|             drx service functions.
+-----------------------------------------------------------------------------
*/


#define ENTITY_GDD_DIO

/*==== INCLUDES =============================================================*/

#include "typedefs.h"   /* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */

/* GDD stuff */
#include "gdd_dio.h"       /* to get the global entity definitions */
#include "gdd_dio_data.h"  /* to get internal data structures */

#include "gdd_dio_con_mgr.h"
#include "gdd_dio_queue.h"
#include "gdd_dio_drxf.h"
#include "gdd_dio_txf.h"


/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/


GLOBAL void gdd_dio_drx_sig_receive_data(T_GDD_CON_HANDLE con_handle, T_dio_buffer * buf)
{
  T_GDD_DIO_CON_DATA * con_data;
  GDD_RESULT result;
  
  TRACE_FUNCTION("[GDD] gdd_dio_drx_sig_receive_data()");
  
  con_data = get_con_data_from_handle(con_handle);
  
  result = con_data->rcv_cb(con_handle, (T_GDD_BUF *)buf);
  if(result EQ GDD_OK)
  {
    gdd_dio_send_signal_to_dio(con_data, DRV_SIGTYPE_WRITE);
  }
  else
  {
  /* Do nothing - the client must come back to use by calling
    gdd_signal_ready_rcv. */
  }
}

GLOBAL void gdd_dio_drx_ready_to_rcv(T_GDD_CON_HANDLE  con_handle)
{
  T_GDD_DIO_CON_DATA * con_data;
  T_dio_buffer * buf;
  GDD_RESULT result;
  
  TRACE_FUNCTION("[GDD] gdd_dio_drx_ready_to_rcv()");
  
  con_data = get_con_data_from_handle(con_handle);
  
  if(gdd_dio_queue_peek_next_for_dequeue(&(con_data->tx_queue), &buf))
  {
    /* Send data to client */  
    result = con_data->rcv_cb(con_handle, (T_GDD_BUF *)buf);
    if(result EQ GDD_OK)
    {
      gdd_dio_send_signal_to_dio(con_data, DRV_SIGTYPE_WRITE);
    }
    else
    {
    /* Do nothing - the client must come back again later by calling
      gdd_signal_ready_rcv. */
    }
  }
  else
  {
    TRACE_ERROR("[GDD] Failed to peek buffer");
  }
}

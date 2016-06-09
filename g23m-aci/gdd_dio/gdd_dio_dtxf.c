/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_txf.c
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
|             dtx service functions.
+-----------------------------------------------------------------------------
*/


#define ENTITY_GDD_DIO

/*==== INCLUDES =============================================================*/

#include "typedefs.h"   /* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */

/* GDD stuff */
#include "gdd_dio.h"       /* to get the global entity definitions */
#include "gdd_dio_con_mgr.h"

#include "gdd_dio_dtxf.h"
#include "gdd_dio_rxf.h"
#include "gdd_dio_drxf.h" /* Needed for allocate_gdd_desc_list() etc */

#include "gdd_dio_queue.h"

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/


GLOBAL GDD_RESULT gdd_dio_dtx_get_send_buffer
(
 T_GDD_CON_HANDLE  con_handle,
 T_GDD_BUF **      send_buf,
 U16               data_size
 )
{
  T_GDD_INST_ID inst;
  T_GDD_DIO_DATA * gdd_dio_data;
  T_GDD_DIO_CON_DATA * con_data;
  T_dio_buffer       * dio_buf;
  
  TRACE_USER_CLASS(TC_FUNC_DATA_FLOW, "[GDD] gdd_dio_dtx_get_send_buffer()");
  
  inst = (T_GDD_INST_ID)inst_num_from_dev_id(con_handle);
  
  /**
  * Do the necessary checks.
  */
  if(inst < 0)
  {
    TRACE_ERROR("[GDD] Invalid connection handle");
    return GDD_INVALID_PARAMS;
  }
  
  if(inst >= GDD_NUM_INSTS)
  {
    TRACE_ERROR("[GDD] inst id out of bounds");
    return GDD_INTERNAL_ERROR;
  }
  gdd_dio_data = &gdd_dio_data_base[inst];
  
  if(gdd_dio_init_flag[inst] EQ FALSE)
  {
    TRACE_ERROR("[GDD] Instance not initialized");
    return GDD_INVALID_PARAMS;
  }
  
  if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_READY)
  {
    TRACE_ERROR("[GDD] DIO driver not initialized");
    return GDD_INTERNAL_ERROR;
  }  
  
  con_data = get_con_data(gdd_dio_data, con_handle);
  if(con_data EQ NULL)
  {
    TRACE_ERROR("[GDD] Invalid connection handle");
    return GDD_INVALID_PARAMS;
  }
  
  if(data_size EQ 0 || data_size > con_data->dio_cap.mtu_data)
  {
    TRACE_ERROR("[GDD] requested data_size (MTU size) out of range");
    return GDD_INVALID_PARAMS;
  }
  
  if( con_data->con_state EQ GDD_DIO_CON_CONNECT ||
    con_data->con_state EQ GDD_DIO_CON_SENDING ||
    gdd_dio_queue_peek_next_for_dequeue(&con_data->rx_queue, &dio_buf) EQ FALSE)
  {
  /* Set the flag indicating the somebody (the client) is waiting for a send
  buffer. As a consequence, the signal GDD_SIGTYPE_SEND_BUF_AVAILABLE
    will be sent as soon as we receive a new RX buffer from PSI. */
    char * reason;
    if(con_data->con_state EQ GDD_DIO_CON_CONNECT)
      reason = "con_state=GDD_DIO_CON_CONNECT";
    else if(con_data->con_state EQ GDD_DIO_CON_SENDING)
      reason = "con_state=GDD_DIO_CON_SENDING";
    else
      reason = "no buffer available in RX queue";
    
    con_data->wait_send_buf = TRUE;
    
    TRACE_EVENT_P2("[GDD] Cannot return buffer [con_handle=0x%4x: %s]",
      con_handle, reason);
    
    return GDD_NO_BUF_AVAILABLE;
  }
  else
  {
  /* Make sure that buffer can hold what is requested.
  We must take into account that the first 2-byte segment holding
    the protocol ID which is not part of the pay-load */
    if(data_size > (dio_buf->length-2))
    {
      TRACE_ERROR("[GDD] Requested buffer size too large");
      return GDD_REQ_BUF_TOO_LARGE;
    }
    else
    {
      /* Setup current descriptor list and pass it back to the client */
      (*send_buf) = (T_GDD_BUF *)dio_buf;
      
      con_data->con_state = GDD_DIO_CON_SENDING;
      
      return GDD_OK;
    }
  }
}


 GLOBAL GDD_RESULT gdd_dio_dtx_send_buffer(T_GDD_CON_HANDLE con_handle, T_GDD_BUF * buf)
 {
#ifdef GDD_MAKE_DTX_CONTEXT_SWITCH
   U32 signal = GDD_DIO_SIGNAL_SEND_DATA | (U32)con_handle;
#endif /* GDD_MAKE_DTX_CONTEXT_SWITCH */
   T_GDD_INST_ID inst;
   T_GDD_DIO_DATA * gdd_dio_data;
   T_GDD_DIO_CON_DATA * con_data;
   T_dio_buffer * dio_buf;
   
   TRACE_USER_CLASS(TC_FUNC_DATA_FLOW, "[GDD] gdd_dio_dtx_send_buffer()");
   
   inst = (T_GDD_INST_ID)inst_num_from_dev_id(con_handle);
   
   /**
   * Do the necessary checks.
   */
   if(inst < 0)
   {
     TRACE_ERROR("[GDD] Invalid connection handle");
     return GDD_INVALID_PARAMS;
   }
   else if(inst >= GDD_NUM_INSTS)
   {
     TRACE_ERROR("[GDD] inst id out of bounds");
     return GDD_INTERNAL_ERROR;
   }
   gdd_dio_data = &gdd_dio_data_base[inst];
   
   if(gdd_dio_init_flag[inst] EQ FALSE)
   {
     TRACE_ERROR("[GDD] Instance not initialized");
     return GDD_ALREADY_INITIALIZED;
   }
   
   if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_READY)
   {
     TRACE_ERROR("[GDD] DIO driver not initialized");
     return GDD_INTERNAL_ERROR;
   }  
   
   con_data = get_con_data(gdd_dio_data, con_handle);
   if(con_data EQ NULL)
   {
     TRACE_ERROR("[GDD] Invalid connection handle");
     return GDD_INVALID_PARAMS;
   }
   
   /* The pointer which is next for dequeue must be the one which
   corresponds to the buffer for sending ! */
   if(gdd_dio_queue_peek_next_for_dequeue(&con_data->rx_queue, &dio_buf) EQ FALSE)
   {
     return GDD_INTERNAL_ERROR;
   } 
#ifdef GDD_MAKE_DTX_CONTEXT_SWITCH
#ifdef MEMORY_SUPERVISION  
   vsi_c_ssend(hCommGDD_DIO, signal, (T_VOID_STRUCT*)dio_buf, sizeof(T_VOID_STRUCT*), __FILE__, __LINE__);
#else
   vsi_c_ssend(hCommGDD_DIO, signal, (T_VOID_STRUCT*)dio_buf, sizeof(T_VOID_STRUCT*));
#endif
   return GDD_OK;
#else /* GDD_MAKE_DTX_CONTEXT_SWITCH */
   /* Call the corresponding RX function directly instead of sending signal */
   gdd_dio_rx_sig_send_data(con_handle, dio_buf);
   return GDD_OK;
#endif /* GDD_MAKE_DTX_CONTEXT_SWITCH */  


 }


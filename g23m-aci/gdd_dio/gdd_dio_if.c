/* 
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
+-----------------------------------------------------------------------------
|  Copyright 2004 Texas Instruments Berlin, AG
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
|  Purpose :  Implementation of the GDD interface with DIOv4 
+-----------------------------------------------------------------------------
*/

#define GDD_DIO_IF_C

/*==== INCLUDES =============================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "pei.h"

#include "gdd.h"
#include "gdd_dio_data.h"
#include "gdd_dio.h"

#include "gdd_dio_con_mgr.h"
#include "gdd_dio_queue.h"

#include "gdd_dio_dtxf.h"
#include "gdd_dio_drxf.h"


/*==== CONSTANTS ============================================================*/

/*==== TYPES ================================================================*/

/*==== PROTOTYPES ===========================================================*/

/*==== GLOBAL VARS ==========================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== LOCAL FUNCTIONS=======================================================*/

/*==== EXPORTED FUNCTIONS====================================================*/

/** gdd_init_dio - see header "gdd.h" for comment and description */

GDD_RESULT gdd_init_dio
(
 T_GDD_INST_ID     inst,
 void *            mem,
 U16               num_con
 )
{
  T_GDD_DIO_DATA * gdd_dio_data;
  
  TRACE_FUNCTION("[GDD] gdd_init_dio()");
  
  /* Check params */
  if(inst < GDD_INST_BAT || inst >= GDD_NUM_INSTS)
  {
    TRACE_ERROR("[GDD] Instance id out of bounds");
    return GDD_INVALID_PARAMS;
  }
  
  gdd_dio_data = &gdd_dio_data_base[inst];
  
  if(gdd_dio_init_flag[inst])
  {
    TRACE_ERROR("[GDD] Attempted to call gdd_init_dio() twice for same instance");
    return GDD_ALREADY_INITIALIZED;
  }
  
  if(num_con < 1 || num_con > gdd_dio_data->max_con)
  {
    TRACE_ERROR("[GDD] Number of connections out of bounds");
    return GDD_INVALID_PARAMS;
  }
  
  if(mem)
  {
    gdd_dio_data->con_arr = mem;
    gdd_dio_data->con_arr_mem_allocated = FALSE;
  }
  else
  {
    gdd_dio_data->con_arr = 0;
    MALLOC(gdd_dio_data->con_arr, GDD_DIO_SIZEOF_CONDATA * num_con);
    if(gdd_dio_data->con_arr EQ 0)
    {
      TRACE_ERROR("[GDD] memory allocation failed");
      return GDD_NO_MEMORY;
    }
    
    gdd_dio_data->con_arr_mem_allocated = TRUE;
  }
  
  gdd_dio_data->max_con = num_con;
  
  /* Initialize the connection manager */
  gdd_dio_con_mgr_init(gdd_dio_data);
  
  gdd_dio_init_flag[inst] = TRUE;
  
  return GDD_OK;
}


/** gdd_deinit  - see header "gdd.h" for comment and description*/
GDD_RESULT gdd_deinit_dio
(
 T_GDD_INST_ID     inst
 )
{
  T_GDD_DIO_DATA * gdd_dio_data = 0;
  
  TRACE_FUNCTION("[GDD] gdd_deinit_dio()");
  
  if(inst < GDD_INST_BAT || inst >= GDD_NUM_INSTS)
  {
    TRACE_ERROR("[GDD] instance id out of bounds");
    return GDD_INVALID_PARAMS;
  }
  
  gdd_dio_data = &gdd_dio_data_base[inst];
  
  
  /* If we allocated our own memory, free it. */
  if(gdd_dio_data->con_arr_mem_allocated EQ FALSE)
  {
    MFREE(gdd_dio_data->con_arr);
    gdd_dio_data->con_arr = 0;
    gdd_dio_data->con_arr_mem_allocated = FALSE;
  }
  
  gdd_dio_init_flag[inst] = FALSE;
  
  return GDD_OK;
}


/** gdd_connect - see header "gdd.h" for comment and description */
GDD_RESULT gdd_connect_dio
(
 T_GDD_INST_ID     inst,
 T_GDD_CON_HANDLE * con_handle,
 const T_GDD_CAP * cap,
 T_GDD_RECEIVE_DATA_CB rcv_cb,
 T_GDD_SIGNAL_CB   sig_cb
 )
{
  T_GDD_DIO_DATA * gdd_dio_data;
  
  TRACE_FUNCTION("[GDD] gdd_connect_dio()");
  
  /**
  * Do the necessary checks.
  */
  
  if(inst < GDD_INST_BAT || inst >= GDD_NUM_INSTS)
  {
    TRACE_ERROR("[GDD] instance id out of bounds");
    return GDD_INVALID_PARAMS;
  }
  
  gdd_dio_data = &gdd_dio_data_base[inst];
  
  if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_READY)
  {
    TRACE_ERROR("[GDD] DIO driver not initialized");
    return DRV_INTERNAL_ERROR;
  }  
  
  /* Check if we are already initialized. If we not, we do it ourselfs! */
  /* This is specific to the DIO implementation of GDD */ 
  if(gdd_dio_init_flag[inst] EQ FALSE)
  {
    gdd_init_dio(inst, 0, gdd_dio_data->max_con);
  }
  
  if(con_handle EQ 0)
  {
    TRACE_ERROR("[GDD] Connection handle pointer cannot be 0");
    return GDD_INVALID_PARAMS;
  }
  
  if(cap EQ 0)
  {
    TRACE_ERROR("[GDD] Capabilities pointer cannot be 0");
    return GDD_INVALID_PARAMS;
  }
  
  if(rcv_cb EQ 0 || sig_cb EQ 0)
  {
    TRACE_ERROR("[GDD] Callback function pointer cannot be 0");
    return GDD_INVALID_PARAMS;
  }
  
  /**
  * We are ready to create the DIO connection
  */
  return gdd_dio_con_mgr_new(gdd_dio_data, con_handle, cap, rcv_cb, sig_cb);
}

/** gdd_disconnect - see header "gdd.h" for comment and description */
GDD_RESULT gdd_disconnect_dio
(
 T_GDD_CON_HANDLE  con_handle
 )
{
  if(con_handle EQ 0)
  {
    TRACE_ERROR("[GDD] Connection handle cannot be 0");
    return GDD_INVALID_PARAMS;
  }
  
  /*
  * Inform the connection manager
  */
  return gdd_dio_con_mgr_close(con_handle);
}


/** gdd_get_send_buffer - see header "gdd.h" for comment and description */
GDD_RESULT gdd_get_send_buffer_dio
(
 T_GDD_CON_HANDLE  con_handle,
 T_GDD_BUF **      buf,
 U16               data_size
 )
{
  TRACE_USER_CLASS(TC_FUNC_DATA_FLOW, "[GDD] gdd_get_send_buffer_dio()");
  
  if(con_handle EQ 0)
  {
    TRACE_ERROR("[GDD] Connection handle cannot be 0");
    return GDD_INVALID_PARAMS;
  }
  if(buf EQ NULL)
  {
    TRACE_ERROR("[GDD] buf cannot be NULL");
    return GDD_INVALID_PARAMS;
  }
  
  return gdd_dio_dtx_get_send_buffer(con_handle, buf, data_size);
}


/** gdd_send_data - see header "gdd.h" for comment and description */
GDD_RESULT gdd_send_data_dio
(
 T_GDD_CON_HANDLE  con_handle,
 T_GDD_BUF *       buf
 )
{
  TRACE_USER_CLASS(TC_FUNC_DATA_FLOW, "[GDD] gdd_send_data_dio()");
  
  if(con_handle EQ 0)
  {
    TRACE_ERROR("[GDD] Connection handle cannot be 0");
    return GDD_INVALID_PARAMS;
  }
  if(buf EQ NULL)
  {
    TRACE_ERROR("[GDD] buf cannot be NULL");
    return GDD_INVALID_PARAMS;
  }
  
  return gdd_dio_dtx_send_buffer(con_handle, buf);
}


/** gdd_signal_ready_rcv - see header "gdd.h" for comment and description */
void gdd_signal_ready_rcv_dio( T_GDD_CON_HANDLE  con_handle )
{
  TRACE_FUNCTION("[GDD] gdd_signal_ready_rcv_dio()");
  
  if(con_handle NEQ 0)
  {
    return;
  }
  
  gdd_dio_drx_ready_to_rcv(con_handle);
}


/** Exported function table for DIO implementation of GDD */
T_GDD_FUNC gdd_func_dio =
{
  gdd_init_dio,
  gdd_deinit_dio,
  gdd_connect_dio,
  gdd_disconnect_dio,
  gdd_get_send_buffer_dio,
  gdd_send_data_dio,
  gdd_signal_ready_rcv_dio
};


/*==== EXPORTED HELPER FUNCTIONS=============================================*/

S32 gdd_write_buf_with_pid(const U8 * src_buf,
                           U16 src_size,
                           T_GDD_BUF * dest_buf,
                           U8 protocol_id)
{
  int idx_seg;
  int remaining;

  /* Check that we have more than one segment and that the first one
     is the PID segment which. */
  TRACE_ASSERT(dest_buf->c_segment > 1);
  TRACE_ASSERT(dest_buf->ptr_gdd_segment[0].c_data EQ GDD_DIO_PID_SEG_SIZE);

  if( dest_buf->c_segment < 2 ||
      dest_buf->ptr_gdd_segment[0].c_data NEQ GDD_DIO_PID_SEG_SIZE )
  {
    return -1;
  }

  /* Set the protocol ID, which is carried in the first buffer segment */
  /* Note: the DTI2 interface in the stack uses only 8 bits for the p_id. */
  dest_buf->ptr_gdd_segment[0].ptr_data[0] = 0x0;/* not used, see note above */
  dest_buf->ptr_gdd_segment[0].ptr_data[1] = protocol_id;


  remaining = src_size;

  for(idx_seg=1; idx_seg<dest_buf->c_segment; ++idx_seg)
  {
    T_GDD_SEGMENT * seg = &(dest_buf->ptr_gdd_segment[idx_seg]);
    int bytes_to_copy = remaining < seg->c_data ? remaining : seg->c_data;

    memcpy(seg->ptr_data, src_buf + src_size - remaining, bytes_to_copy);
    remaining -= bytes_to_copy;
    if(remaining EQ 0) 
      break;
  }

  dest_buf->length = src_size - remaining + GDD_DIO_PID_SEG_SIZE;
  return remaining;
}

#define DTI_PID_IP (0x21)  /* simple ip packet (IPv4) */

S32 gdd_write_buf(const U8 * src_buf, U16 src_size, T_GDD_BUF * dest_buf)
{
  return gdd_write_buf_with_pid(src_buf, src_size, dest_buf, DTI_PID_IP);
}

S32 gdd_read_buf(const T_GDD_BUF * src_buf, U8 * dest_buf, U16 dest_size)
{
  int idx_seg;
  int remaining;

  /* Check that we have more than one segment and that the first one
     is the PID segment which. */

  TRACE_ASSERT(src_buf->c_segment > 1);
  TRACE_ASSERT(src_buf->ptr_gdd_segment[0].c_data EQ GDD_DIO_PID_SEG_SIZE);
  if( src_buf->c_segment < 2 ||
      src_buf->ptr_gdd_segment[0].c_data NEQ GDD_DIO_PID_SEG_SIZE )
  {
    return -1;
  }
  
  remaining = src_buf->length - GDD_DIO_PID_SEG_SIZE;
  if(remaining > dest_size)
  {
    remaining = dest_size;
  } 

  for(idx_seg=1; idx_seg<src_buf->c_segment; ++idx_seg)
  {
    T_GDD_SEGMENT * seg = &(src_buf->ptr_gdd_segment[idx_seg]);
    int bytes_to_copy = remaining < seg->c_data ? remaining : seg->c_data;
    
    memcpy(dest_buf + dest_size - remaining, seg->ptr_data, bytes_to_copy);
    remaining -= bytes_to_copy;
    if(remaining EQ 0)
      break;
  }

  return remaining;
}

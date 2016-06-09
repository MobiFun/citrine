/* 
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :  BAT
+-----------------------------------------------------------------------------
|  Copyright 2005 Texas Instruments Berlin, AG
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
|  Purpose :  This Modul holds the functions
|             for the L2P interface
+-----------------------------------------------------------------------------
*/

#define _BAT_L2P_C_

/*==== INCLUDES =============================================================*/
#include "typedefs.h"
#include "gdd.h"
#include "gdd_sys.h"       /* to get semaphore access */
#include "l2p_types.h"
#include "l2p.h"
#include "bat.h"
#include "bat_ctrl.h"
#include "bat_intern.h"

/*==== DEFINES ===========================================================*/
#define ENTER_CRITICAL_SECTION(sem) if (bat_enter_critical_section(sem))return (NULL);
#define LEAVE_CRITICAL_SECTION(sem) if (bat_leave_critical_section(sem))return -1;

/*==== LOCAL VARS ===========================================================*/


/*==== LOCAL FUNCTIONS =====================================================*/

LOCAL void bat_semaphore_err (void)
{
  static U8 out = 0;
  if (!out)
  {
    out = 1;
    BAT_TRACE_ERROR("semaphore error");
  }
}


LOCAL int bat_enter_critical_section (T_HANDLE sem)
{
  if (gdd_sys_sem_down (sem) NEQ VSI_OK)
  {
    bat_semaphore_err();
    return -1;
  }
  else
  {
    return 0;
  }
}


LOCAL int bat_leave_critical_section (T_HANDLE sem)
{
  if (gdd_sys_sem_up(sem) NEQ VSI_OK)
  {
    bat_semaphore_err();
    return -1;
  }
  else
  {
    return 0;
  }
}

/*
 * we have to maintain some stuff for L2P_Receive().
 */
LOCAL void bat_update_l2p_mt (T_BAT_instance_maintain *inst_mt, T_GDD_SEGMENT *desc, U16 length, BOOL substract)
{
  BAT_TRACE_EVENT_P1("bat_update_l2p_mt(): segment len is %d", length);
  inst_mt->l2p_mt.desc = desc;
  if (substract)
  {
    inst_mt->l2p_mt.length = inst_mt->l2p_mt.length - length;
  }
  else
  {
    inst_mt->l2p_mt.length = length;
  }
}


LOCAL T_GDD_SEGMENT *bat_get_from_l2p_mt_desc (T_BAT_instance_maintain *inst_mt)
{
  return (inst_mt->l2p_mt.desc);
}

LOCAL U16 bat_get_from_l2p_mt_length (T_BAT_instance_maintain *inst_mt)
{
  return (inst_mt->l2p_mt.length);
}



/*
+----------------------------------------------------------------------------+
| PROJECT :                    MODULE  : BINARY AT COMMAND LIBRARY           |
| STATE   : code               ROUTINE : bat_l2p_receive                     |
+----------------------------------------------------------------------------+
PURPOSE :
  This function is a wrapper around L2P_Receive to have all L2P related stuff
  in one module.  
*/

BOOL bat_l2p_receive (U8 inst_hndl, T_GDD_BUF *buf)
{
  T_BAT_instance_maintain *inst_mt = NULL;
  T_GDD_SEGMENT  *seg = NULL;
  U16          length = 0;
  T_L2P_STATUS l2p_status;
  
  if (bat_get_instance_from_instance_handle(inst_hndl, &inst_mt))
  {
    BAT_TRACE_ERROR ("bat_l2p_receive(): inst_hndl is not correct!");
    return (FALSE);
  }

  inst_mt->buffer.gdd_buf_rcv = buf;
  BAT_TRACE_EVENT_P2 ("bat_l2p_receive(): data length: %d, has c_segment %d", buf->length, buf->c_segment);

  seg = buf->ptr_gdd_segment;
  ++seg; /* Skip first segment which is the protocol ID and not needed */
  
  length = buf->length;

  bat_update_l2p_mt (inst_mt, seg, length, FALSE);

  /* 
   * for the case that the BAT Module at ACI was not able to send BAT responses/indications,
   * it builds up a linked list of responses/indications, 
   * which then is sent at once when BAT Module was able again.
   * That is why we have here to iterate over the GDD list.
   */
  do
  {
    /*
     * within L2P_Receive() is called bat_l2p_get_next_buf_seg() if neccessary (fragmentation)
     * and finally it is called bat_l2p_message_rxd(). 
     * Within bat_l2p_message_rxd() is called the application's callback.
     */
    BAT_TRACE_EVENT("bat_l2p_receive() loop...");
    l2p_status = L2P_Receive(inst_hndl, seg->ptr_data, seg+1, length, seg->c_data);
    switch (l2p_status)
    {
      case (L2P_STAT_SUCCESS):
      {
        /* for the next iteration get the updated parameter for L2P_Receive() */
        seg   = bat_get_from_l2p_mt_desc(inst_mt);
        length = bat_get_from_l2p_mt_length(inst_mt);
        break;
      }
      default: /* any error */
      {
        BAT_TRACE_EVENT_P1("bat_l2p_receive(): L2P status = %d", l2p_status);
        return (FALSE);
      }
    }   
  } while(seg);

  return (TRUE);
}


/*==== functions exported to L2P ============================================*/

/*
 * This function is called by L2P_Send() and L2P_Receive()
 */
void *bat_l2p_get_next_buf_seg (U8 inst_hndl, void *seg_hdr, void **seg_hdr_ptr,  U16 *segSize)
{
  T_BAT_instance_maintain *inst_mt = NULL;
  T_GDD_SEGMENT *gdd_seg = (T_GDD_SEGMENT *)seg_hdr;
  T_GDD_SEGMENT *seg = NULL;
  
  if (seg_hdr EQ NULL)
  {
    BAT_TRACE_EVENT ("bat_l2p_get_next_buf_seg(): GDD buffer is complete");
    return (NULL);
  }
  
  if (bat_get_instance_from_instance_handle(inst_hndl, &inst_mt))
  {
    BAT_TRACE_ERROR ("bat_l2p_get_next_buf_seg(): inst_hndl is not correct!");
    return (NULL);
  }
 
 /* check whether we are in RX context (it is called for TX as well) */
  seg = bat_get_from_l2p_mt_desc(inst_mt);
  
  if (seg AND ((seg+1) EQ gdd_seg)) 
  {
    bat_update_l2p_mt(inst_mt, gdd_seg+1, gdd_seg->c_data, TRUE);
  }
  
  return (void*)(gdd_seg->ptr_data);  
}


void *bat_l2p_get_tx_buffer(T_BAT_instance inst_hndl, U16 data_size, 
                            void **seg_hdr_ptr,  U16 *total_size, U16 *seg_size)
{
  T_BAT_instance_maintain *inst_mt = NULL;
  T_GDD_BUF *buf = NULL;
  T_GDD_SEGMENT * seg = NULL;

   BAT_TRACE_FUNCTION ("bat_l2p_get_tx_buffer()");

  if (bat_get_instance_from_instance_handle(inst_hndl, &inst_mt))
  {
    BAT_TRACE_ERROR ("bat_l2p_get_tx_buffer(): inst_hndl is not correct!");
    return (NULL);
  }

  if (data_size >= (int)(inst_mt->config->adapter.cap.dio_cap.mtu_size))
  {
    BAT_TRACE_ERROR ("bat_l2p_get_tx_buffer(): requested buffer too big!");
    return (NULL);
  }

  /*
   * according to GR we have to make 
   * gdd_get_send_buffer() and gdd_send_data() "atomic",
   * which means we release the semaphore in bat_l2p_send_frame()
   */
  ENTER_CRITICAL_SECTION(inst_mt->sem_BAT);
  
  if (inst_mt->config->adapter.gdd_if.gdd_get_send_buffer 
      ((T_GDD_CON_HANDLE)(inst_mt->con_handle), &buf, data_size) NEQ GDD_OK)
  {
    BAT_TRACE_ERROR ("bat_l2p_get_tx_buffer(): call to get gdd buffer failed!");    
    return (NULL);
  }
  /* To memorize the buffer address for later sending */
  inst_mt->buffer.gdd_buf = buf;
  /* the first 2 bytes are not used */
  *total_size = (U16)(buf->length - 2);
  /*store the pointer pointing to the next seg, ignore the first ptr bec it is for other purpose*/

  seg = buf->ptr_gdd_segment;
  ++seg; /* Skip first segment which is the protocol ID and not needed */
  *seg_hdr_ptr = (void*)(seg+1);
  *seg_size = seg->c_data;

  return (void*)(seg->ptr_data);
}


int bat_l2p_send_frame(T_BAT_instance inst_hndl)
{
  T_BAT_instance_maintain *inst_mt = NULL;

   BAT_TRACE_FUNCTION ("bat_l2p_send_frame()");


  if (bat_get_instance_from_instance_handle(inst_hndl, &inst_mt))
  {
    BAT_TRACE_ERROR ("bat_l2p_send_frame(): inst_hndl is not correct!");
    return (0);
  }
  
  if (inst_mt->config->adapter.gdd_if.gdd_send_data
      ((T_GDD_CON_HANDLE)(inst_mt->con_handle), inst_mt->buffer.gdd_buf) NEQ GDD_OK)
  {
    BAT_TRACE_ERROR ("bat_l2p_send_frame(): call to send gdd buffer failed!");  
    LEAVE_CRITICAL_SECTION(inst_mt->sem_BAT);
    return (-1);
  }
  
  LEAVE_CRITICAL_SECTION(inst_mt->sem_BAT);
  /*return OK*/
  return (0);
}


void *bat_l2p_get_rx_buffer(U8 inst_hndl)
{
  T_BAT_instance_maintain *inst_mt = NULL;

  BAT_TRACE_FUNCTION ("bat_l2p_get_rx_buffer()");

  if (bat_get_instance_from_instance_handle(inst_hndl, &inst_mt))
  {
    BAT_TRACE_ERROR ("bat_l2p_get_rx_buffer(): inst_hndl is not correct!");
    return (NULL);
  }
  /* set the state of the buffer */
  bat_change_buffer_state(inst_mt, BAT_BUF_FILLING);

  return (inst_mt->buffer.data);  
}


void bat_l2p_message_rxd (U8 inst_hndl, U8 client_id, U32 data_tag, void *data_ptr, U16 data_size)
{
  T_BAT_instance_maintain *inst_mt = NULL;
  T_BATC_confirm cnf;
  T_BAT_return ret = BAT_ERROR;
  T_GDD_SEGMENT * seg = NULL;

  data_size = data_size;  /* prevent compiler warnings */

  BAT_TRACE_FUNCTION ("bat_l2p_message_rxd()");

  if (bat_get_instance_from_instance_handle(inst_hndl, &inst_mt))
  {
    BAT_TRACE_ERROR ("bat_l2p_message_rxd(): inst_hndl is not correct!");
    return;
  }

  bat_change_buffer_state(inst_mt, BAT_BUF_FILLED);

  switch (client_id)
  {
    case (BAT_CONTROL_CHANNEL):
    {
      cnf.rsp.ptr_bat_open_client_cnf = data_ptr;
      cnf.rsp_params = (T_BATC_rsp_param)data_tag;
      ret = bat_control_confirm_rcv (inst_hndl, cnf);
      break;
    }
    case (BAT_BROADCAST_CHANNEL):
    {
      inst_mt->buffer.rsp.ctrl_response = (T_BAT_ctrl_response)data_tag;
      inst_mt->buffer.rsp.response.ptr_at_ok = data_ptr;
      ret = bat_unsolicited_code_rcv (inst_hndl, &(inst_mt->buffer.rsp));
      break;
    }
    default:
    {
      inst_mt->buffer.rsp.ctrl_response = (T_BAT_ctrl_response)data_tag;
      inst_mt->buffer.rsp.response.ptr_at_ok = data_ptr;
      ret = bat_command_response_rcv (inst_hndl, client_id, &(inst_mt->buffer.rsp));
      break;  
    }
  }
  switch (ret)
  {
    case (BAT_OK):
    case (BAT_BUSY_RESOURCE):
    {
      break;
    }
    case (BAT_ERROR):
    {
      BAT_TRACE_ERROR ("bat_l2p_message_rxd(): received data have not been processed!");
      break;
    }
  }

  seg = bat_get_from_l2p_mt_desc(inst_mt);
  
  if (seg)
  {
    /* Get next segment */
    int cur_idx = seg - inst_mt->buffer.gdd_buf_rcv->ptr_gdd_segment;
    T_GDD_SEGMENT * next_seg;
   
    BAT_TRACE_EVENT_P1("buffer starts at 0x%08x", inst_mt->buffer.gdd_buf_rcv->ptr_gdd_segment);
    BAT_TRACE_EVENT_P1("seg is 0x%08x", seg);

    BAT_TRACE_EVENT_P1("cur_idx is %d", cur_idx);

    if(cur_idx >= inst_mt->buffer.gdd_buf_rcv->c_segment-1)
      next_seg = 0;
    else
      next_seg = ++seg;

    if (next_seg EQ NULL)
    {
      bat_update_l2p_mt(inst_mt, NULL, 0, FALSE);
    }
    else
    {
      /* 
       * update to the next GDD descriptor, which has a L2P header
       * and reduce the overall L2P data length by the currently processed data length
       */
      bat_update_l2p_mt(inst_mt, next_seg, seg->c_data, TRUE);
    }
  }  
}


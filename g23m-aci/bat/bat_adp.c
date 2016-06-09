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
|  Purpose :  This Modul holds the functions used by GDD
|             for the binary AT command library 
+-----------------------------------------------------------------------------
*/

#define _BAT_ADP_C_

#include "typedefs.h"
#include "gdd.h"
#include "l2p_types.h"
#include "l2p.h"
#include "bat.h"
#include "bat_ctrl.h"
#include "bat_intern.h"


/*
+----------------------------------------------------------------------------+
| PROJECT :                    MODULE  : BINARY AT COMMAND LIBRARY           |
| STATE   : code               ROUTINE : bat_gdd_receive_data_cb             |
+----------------------------------------------------------------------------+
PURPOSE :
  This function is called by GDD when itself received new data from PSI.
  It will pass a GDD buffer (linked list), which has the following speciality:
   - the first list element is reserved and BAT Lib has to follow the next pointer
     to see the "first" user data.
   - such a user data element is passed by BAT Lib to L2P. L2P added at the sender
     side its own header, which maintains possibly fragmentation. If the received
     data have been fragmented by L2P at the sender side, then L2P itself iterates 
     over the GDD linked list by calling bat_l2p_get_next_buf_seg() here at receiver side. 
     When L2P has reassembled from several segments/list element the complete user data, 
     then it passes this complete data to BAT Lib.
   - BUT THERE IS AN ADDITIONAL SCENARIO:
     ACI was not able to send the data to PSI/GDD. Then ACI also builds up a linked list
     until it is able to send to PSI/GDD.
     Every list element represents one response/indication, which itself could be fragmented by L2P.
     Assume the following scenario:
       The connection between ACI and PSI was not available.
       ACI received three indications from the protocol stack, where the second one is too large so that
       L2P must split it into two segments. That means that ACI has built a linked list of four
       elements, which represents three logical data unit. The first list element belongs to the
       first indication, the second and third list element belongs to the second indication
       and the fourth list element belongs to the third indication.
*/

GDD_RESULT bat_gdd_receive_data_cb (T_GDD_CON_HANDLE con, T_GDD_BUF *buf)
{
  T_BAT_instance inst_hndl = BAT_BROADCAST_CHANNEL;
  T_BAT_instance_maintain *inst_mt = NULL;

  BAT_TRACE_FUNCTION ("bat_gdd_receive_data_cb()");
  BAT_TRACE_EVENT_P2 ("data received from GDD has length: %d, has c_segment %d", buf->length, buf->c_segment);

  /* if the info is for an unknown instance, return OK, GDD can only handle ok and busy, 
   so in error cases, GDD_OK will be returned */
  if (bat_get_instance_from_gdd_handle(con, &inst_hndl, &inst_mt))
  {
    /* if instance is unknown, BAT Lib ignores the data and returns OK to GDD  */
    BAT_TRACE_ERROR("bat_gdd_receive_data_cb(): Data from unknown instance received!");
    return (GDD_OK);
  }

  /* if the instance is not initialized */
  if (inst_mt->instance_state EQ BAT_INSTANCE_IDLE)
  {
    /* if instance is not initialized, BAT Lib ignores the data and returns OK to GDD  */
    BAT_TRACE_ERROR ("bat_gdd_receive_data_cb(): Data is received from GDD for an uninitialized instance.");
    return (GDD_OK);
  }

  /* check if the receiver buffer is free, if not return a busy signal to GDD, BAT Lib will
     send a ready signal to GDD when the buffer is freed (BAT Lib will receive ctrl info from)
     app, see bat_ctrl() */
  if (inst_mt->buffer.buf_st EQ BAT_BUF_FILLED)
  {
    BAT_TRACE_EVENT ("bat_gdd_receive_data_cb(): receive buffer is not yet freed!");
    return (GDD_BUSY);  
  }

  bat_l2p_receive(inst_hndl, buf);
    
  return (GDD_OK);
}


LOCAL void bat_check_clients_for_busy (T_BAT_instance_maintain *instance, T_BAT_instance inst_hndl)
{
  T_BAT_client_maintain *clnt_mt   = NULL;
  T_BAT_client           clnt_hndl = BAT_INVALID_CLIENT_HANDLE;
  int i;
  
  for (i = 0; i < instance->max_client_num; i++)
  {
    clnt_hndl = MAKE_CLNT_HNDL(i, inst_hndl);
    bat_get_client_from_client_handle(clnt_hndl, &clnt_mt);
    switch (clnt_mt->client_state)
    {
      case (BAT_CLIENT_BUSY):
      {
        clnt_mt->signal_cb (clnt_hndl, BAT_READY_RESOURCE);
        bat_change_client_state(clnt_mt, BAT_CLIENT_READY);
        break;
      }
      case (BAT_CLIENT_SENDING_AND_BUSY):
      {
        clnt_mt->signal_cb (clnt_hndl, BAT_READY_RESOURCE);
        bat_change_client_state(clnt_mt, BAT_CLIENT_SENDING);
        break;
      }
      default:
        break;              
    }
  }
}

/*
+----------------------------------------------------------------------------+
| PROJECT :                    MODULE  : BINARY AT COMMAND LIBRARY           |
| STATE   : code               ROUTINE : bat_gdd_signal_cb                   |
+----------------------------------------------------------------------------+
PURPOSE :
  This function is called by GDD for signalling of control information between
  BAT Lib and GDD.
  
*/
void bat_gdd_signal_cb(T_GDD_CON_HANDLE con, T_GDD_SIGNAL signal)
{
  T_BAT_instance_maintain *instance = NULL;
  T_BAT_instance inst_hndl = BAT_INVALID_INSTANCE_HANDLE;
  T_BATC_signal ctrl_data;
  T_BATC_max_clients max_clnt;
  
  BAT_TRACE_FUNCTION ("bat_gdd_signal_cb()");
  BAT_TRACE_EVENT_P1 ("bat_gdd_signal_cb(): con_hndl searched for is %x", con);
  
  if (bat_get_instance_from_gdd_handle(con, &inst_hndl, &instance))
  {
    BAT_TRACE_ERROR ("bat_gdd_signal_cb(): gdd handle is not found, ignore signal");
    return;
  }

  switch (signal.sig)
  {
    case (GDD_SIGTYPE_CONNECTION_OPENED):    
    case (GDD_SIGTYPE_SEND_BUF_AVAILABLE):
    {
      BAT_TRACE_EVENT ("bat_gdd_signal_cb(): GDD_SIGTYPE_SEND_BUF_AVAILABLE received from GDD");
      
      switch (instance->instance_state)
      {
        case (BAT_INSTANCE_IDLE):
        {
          /* update the instance state */
          bat_change_instance_state(instance, BAT_INSTANCE_ACTIVATING);

          /* BAT sends out the control info to ACI*/
          ctrl_data.ctrl_params = BATC_MAX_CLIENTS;
          max_clnt.num_clients = instance->max_client_num;
          ctrl_data.params.ptr_max_clients = &max_clnt;
  
          if (bat_send_ctrl_data(inst_hndl, &ctrl_data) NEQ BAT_OK)
          {
            BAT_TRACE_ERROR ("bat_gdd_signal_cb(): BAT new fails to send ctrl info, connection can not be open");

            if (instance->config->adapter.gdd_if.gdd_disconnect(con))
            {
              BAT_TRACE_ERROR ("bat_gdd_signal_cb(): GDD returns error when disconnecting.");
            }
            instance->instance_signal_cb(BAT_NEW_INSTANCE_FAIL);
            bat_deinit_instance_pointer(inst_hndl);
          }
          break;
        }  
        /* flow control for client channels, GDD state has be changed from BUSY to READY */      
        case (BAT_INSTANCE_READY):
        {
          bat_check_clients_for_busy(instance, inst_hndl);
          break; 
        }
        /* flow control for instance, bat_open() hasn't been succesful yet */
        case (BAT_INSTANCE_BUSY):
        {
          instance->instance_signal_cb (BAT_READY_RESOURCE);
          bat_change_instance_state(instance, BAT_INSTANCE_READY);
          break;      
        }
        default:
        {
          return;
        }
      }
      break;
    }
    case (GDD_SIGTYPE_CONNECTION_FAILED):
    {
      BAT_TRACE_ERROR("bat_gdd_signal_cb(): GDD_SIGTYPE_CONNECTION_OPENED_FAILED received from GDD");
      if (instance->config->adapter.gdd_if.gdd_disconnect(con))
      {
        BAT_TRACE_ERROR ("bat_gdd_signal_cb(): GDD returns error when disconnecting.");
      }
      instance->instance_signal_cb (BAT_NEW_INSTANCE_FAIL);
      bat_deinit_instance_pointer(inst_hndl);
      L2P_Remove (inst_hndl);
      break;
    }
    /* ignore other signal types */
    default:
    {
      break;
    }
  }
  return;
}



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
|             for the binary AT command library at APPlication side
+-----------------------------------------------------------------------------
*/
#ifndef _BAT_APP_C_
#define _BAT_APP_C_

/*==== INCLUDES =============================================================*/
#include <string.h>
#include <math.h>

#include "typedefs.h"
#include "gdd.h"
#include "gdd_sys.h"
#include "l2p_types.h"
#include "l2p.h"
#include "bat.h"
#include "bat_ctrl.h"
#include "bat_intern.h"

/*==== DEFINES==== ==========================================================*/
#define BAT_NOT_INITIALIZED FALSE
#define BAT_INITIALIZED TRUE


/*==== GLOBAL VARS ==========================================================*/


/*==== EXTERN VARS ==========================================================*/

LOCAL void cvt_to_str (unsigned char input_num, char *num_str)
{
  int i;
 memset(num_str, 0, 3);

  for (i = 0; i < 3; i++)
  {
    if(input_num/pow(10,i) >0)
    {
      *(num_str+(2-i)) = (char)(input_num/pow(10,i))+'0'; 
    }
  }
  return;
}

/* create dummy functions to make the interface alligned with BT */
GLOBAL T_BAT_return bat_init (void *mem, unsigned char num)
{
  BAT_TRACE_FUNCTION ("bat_init()");
  return (BAT_OK);
}

GLOBAL T_BAT_return bat_deinit (void)
{
  BAT_TRACE_FUNCTION ("bat_deinit()");
  return (BAT_OK);
}

/*
+----------------------------------------------------------------------------+
| PROJECT :                    MODULE  : BINARY AT COMMAND LIBRARY           |
| STATE   : code               ROUTINE : bat_new                             |
+----------------------------------------------------------------------------+
PURPOSE :
  This function is used to create a new instance of BAT Lib. It provides BAT Lib
  the following information: an output parameter to pass back the instance handle
  to the application; a pointer to the memory, which is allocated by the application
  framework, for internal maintenance; the number of clients to maintain; the
  appropriate configuration, a signal call back function pointer
  and an unsolicited result code call back function pointer.
*/

GLOBAL T_BAT_return bat_new (T_BAT_instance  *inst_hndl,
                             void            *mem,
                             unsigned char   num,
                             T_BAT_config    *config,
                             void            (*instance_signal_cb)(T_BAT_signal signal))
{
  int                       i;
  T_BAT_client_maintain     *clnt_mt    = NULL;
  T_BAT_instance_maintain   *inst_mt    = NULL;
  CHAR                      sem_str[] = "SEM_BAT000"; /* long enough to hold "SEM_BATxxx" */

  U32 mem_l2p;

  BAT_TRACE_FUNCTION ("bat_new()");

  /* check if the parameters are correct */
  if ((mem EQ NULL) OR (num > BAT_CONTROL_CHANNEL) OR
      (config EQ NULL) OR (instance_signal_cb EQ NULL))
  {
    BAT_TRACE_ERROR ("bat_new(): input parameter incorrect!");
    return (BAT_ERROR);
  }
  /* init allocated memory */
  memset(mem, 0, num * BAT_CLIENT_SIZE + BAT_INSTANCE_SIZE);

  /* init global params */
  if (bat_init_global_params())
  {
    return (BAT_ERROR);
  }

  /* get the free slot to store the instance pointer */
  if (bat_get_new_instance(inst_hndl) NEQ BAT_OK)
  {
    return (BAT_ERROR);
  }

  /* init the instance maintainance data */
  inst_mt = (T_BAT_instance_maintain *)mem;

  /* init semaphore */
  cvt_to_str(*inst_hndl, &sem_str[7]);
  inst_mt->sem_BAT = gdd_sys_sem_open(sem_str, 1);
    
  if (inst_mt->sem_BAT EQ (-1))
  {
    BAT_TRACE_ERROR("can´t open semaphore \"SEM_BAT\"");
    return (BAT_ERROR);
  }

  /* init the pointer in the pointer list */
  bat_init_instance_pointer (*inst_hndl, inst_mt);

  inst_mt->instance_signal_cb = instance_signal_cb;
  inst_mt->unsolicited_result_cb = NULL;
  inst_mt->max_client_num = num;
  inst_mt->config = config;
  bat_change_instance_state(inst_mt,BAT_INSTANCE_IDLE);
  bat_change_buffer_state(inst_mt, BAT_BUF_EMPTY);

  BAT_TRACE_EVENT_P1 ("bat_new(): BAT INST add is: %04X", mem);

  /* init the clients */
  for (i = 0; i < num; i ++)
  {
    clnt_mt = (T_BAT_client_maintain *)((U32)sizeof(T_BAT_client_maintain)*i + (U32)inst_mt +
                                        (U32)sizeof (T_BAT_instance_maintain));
    BAT_TRACE_EVENT_P2 ("bat_new(): BAT CLNT %d ' add is: %04X", i, clnt_mt);

    bat_change_client_state(clnt_mt, BAT_CLIENT_IDLE);
  }

  /* configure the L2P */
  mem_l2p = (U32)inst_mt+sizeof(T_BAT_instance_maintain) + num*sizeof(T_BAT_client_maintain);
  BAT_TRACE_EVENT_P1("bat_new(): The address passed to L2P is 0x%4x", mem_l2p);
  L2P_Configure(*inst_hndl, (void*)((U32)inst_mt+sizeof(T_BAT_instance_maintain)+num*sizeof(T_BAT_client_maintain)),
                config->l2p.protocol_id, (U16)(config->adapter.cap.dio_cap.mtu_size), bat_l2p_get_tx_buffer,
                bat_l2p_get_rx_buffer, bat_l2p_get_next_buf_seg, bat_l2p_send_frame, bat_l2p_message_rxd);

  /* call GDD to create the connection, if ERROR is returned the app should call bat_new later*/
  if ((config->adapter.gdd_if.gdd_connect(GDD_INST_BAT, (T_GDD_CON_HANDLE*)&(inst_mt->con_handle),
                                   &(config->adapter.cap), bat_gdd_receive_data_cb,
                                   bat_gdd_signal_cb))NEQ GDD_OK)
  {
    BAT_TRACE_ERROR ("bat_new(): Error returned from gdd_connect().");
    bat_deinit_instance_pointer(*inst_hndl);
    L2P_Remove (*inst_hndl);
    return (BAT_ERROR);
  }

  /* BAT Lib is awaiting 2 confirmations. One from GDD and one from ACI, the confirmations will be handled in the cbs */

  return (BAT_OK);
}

/*
+----------------------------------------------------------------------------+
| PROJECT :                    MODULE  : BINARY AT COMMAND LIBRARY           |
| STATE   : code               ROUTINE : bat_delete                          |
+----------------------------------------------------------------------------+
PURPOSE :
  This function is used to delete the BAT Lib instance created by function
  bat_new().The provided callback function is used to inform the application of
  the finial result. If the application receives an indication of successfully
  deleting of the BAT Lib instance, the memory allocated to this instance can
  be freed by the application. Please note that the application should call
  bat_close() to close all the clients before deleting the BAT Lib instance.
*/

GLOBAL T_BAT_return bat_delete (T_BAT_instance inst_hndl)
{
  T_BAT_instance_maintain *inst_mt = NULL;

  BAT_TRACE_FUNCTION ("bat_delete()");

  /* check if inst_hndl is valid */
  if (inst_hndl EQ BAT_INVALID_INSTANCE_HANDLE)
  {
    return (BAT_ERROR);
  }

  /* the instance can not be deleted before all clients are closed */
  if (bat_check_if_all_clients_are_closed(inst_hndl) EQ FALSE)
  {
    return (BAT_ERROR);
  }
  
  /* get instance from instance handle */
  if (bat_get_instance_from_instance_handle (inst_hndl, &inst_mt) EQ BAT_ERROR)
  {
    return (BAT_ERROR);
  }
  
  /* call GDD to inform the close of an instance */
  if (inst_mt->config->adapter.gdd_if.gdd_disconnect((T_GDD_CON_HANDLE)(inst_mt->con_handle)) NEQ GDD_OK)
  {
    BAT_TRACE_ERROR ("bat_delete(): Error returned from gdd_connect(), instance cannot be deleted.");
    return (BAT_ERROR);
  }
  bat_change_instance_state(inst_mt,BAT_INSTANCE_IDLE);
  
  /* remove the l2p maintainance data */
  L2P_Remove (inst_hndl);
  /*BAT deinit the instance*/
  bat_deinit_instance_pointer (inst_hndl);

  if (inst_mt->sem_BAT NEQ (-1))
  {
    gdd_sys_sem_close(inst_mt->sem_BAT);
  }

  /* deinit the BAT Lib if the last instance has been deleted */
  bat_deinit_global_params();

  return (BAT_OK);
}

/*
+----------------------------------------------------------------------------+
| PROJECT :                    MODULE  : BINARY AT COMMAND LIBRARY           |
| STATE   : code               ROUTINE : bat_open                            |
+----------------------------------------------------------------------------+
PURPOSE :
  This function is used to open a new binary AT command client path. This
  function passes a unique client handle to the application by the output
  parameter clnt_hndl. This client handle is used by bat_send(), bat_close()
  and bat_cntrl().The bat_open() function is re-entrant and can be
  called in serial within one instance.
*/

GLOBAL T_BAT_return bat_open( T_BAT_instance instance,
                              T_BAT_client *clnt_hndl,
                              int(*response_cb)( T_BAT_client,T_BAT_cmd_response*),
                              void (*signal_cb)( T_BAT_client, T_BAT_signal))
{
  T_BAT_client_maintain     *clnt_mt  = NULL;
  T_BAT_instance_maintain   *inst_mt  = NULL;
  T_BAT_return              ret = BAT_ERROR;
  T_BATC_signal             ctrl_sig;
  T_BATC_open_client        param;

  BAT_TRACE_FUNCTION ("bat_open()");

  /* find the correct instance */
  if (bat_get_instance_from_instance_handle (instance, &inst_mt) EQ BAT_ERROR)
  {
    return (BAT_ERROR);
  }
  
  /* bat_open should be called after bat instance is created */
  if (inst_mt->instance_state < BAT_INSTANCE_READY)
  {
    BAT_TRACE_EVENT_P1 ("ERROR: the instance state is %d", inst_mt->instance_state);
    return (BAT_ERROR);
  }

  /* check if one more client can be created */
  if (bat_get_new_client(instance, clnt_hndl) EQ BAT_ERROR)
  {
    return (BAT_ERROR);
  }
  
  /* set the necessary states before sending the data to BAT ACI, this is bec it can
     happen that the callback is used before function bat_send_ctrl_data() returns  */
  if (bat_init_new_client (*clnt_hndl, response_cb, signal_cb) EQ BAT_ERROR)
  {
    return (BAT_ERROR);
  }
  
  if (bat_get_client_from_client_handle(*clnt_hndl, &clnt_mt) EQ BAT_ERROR)
  {
    return (BAT_ERROR);
  }
  
  bat_change_client_state(clnt_mt, BAT_CLIENT_ACTIVATING);

  param.client_id = (U32)(GET_CLNT_ID_FROM_CLNT_HANDLE(*clnt_hndl));
  ctrl_sig.ctrl_params = BATC_OPEN_CLIENT;
  ctrl_sig.params.ptr_open_client = &param;

  /* send control data to BAT ACI */
  ret = bat_send_ctrl_data(instance, &ctrl_sig);

  switch (ret)
  {
    case (BAT_BUSY_RESOURCE):
    {
      /* instance state shows that the bat_open() call receives Busy */
      bat_change_instance_state(inst_mt, BAT_INSTANCE_BUSY);
      clnt_mt->client_state   = BAT_CLIENT_IDLE;
      clnt_mt->signal_cb      = NULL;
      clnt_mt->response_cb    = NULL;
      break;
    }
    case (BAT_ERROR):
    {
      /* change back the necessary states if error happens */
      bat_change_client_state(clnt_mt, BAT_CLIENT_IDLE);
      clnt_mt->signal_cb    = NULL;
      clnt_mt->response_cb  = NULL;
      break;
    }
    case (BAT_OK):
    default:
    {
      break;
    }
  }
  return (ret);
}

/*
+----------------------------------------------------------------------------+
| PROJECT :                    MODULE  : BINARY AT COMMAND LIBRARY           |
| STATE   : code               ROUTINE : bat_uns_open                        |
+----------------------------------------------------------------------------+
PURPOSE :
  This function is used to open a path to receive unsolicited result code.
  This function passes a unique client handle to the application by the output
  parameter client. This client handle is used by bat_close()and bat_cntrl().
*/

GLOBAL T_BAT_return bat_uns_open (T_BAT_instance instance,
        T_BAT_client  *client,
        int(*unsolicited_result_cb)( T_BAT_client client, T_BAT_cmd_response *response))
{
  T_BAT_instance_maintain *inst_mt = NULL;

  BAT_TRACE_FUNCTION ("bat_uns_open()");

  /* find the correct instance */
  if (bat_get_instance_from_instance_handle (instance, &inst_mt) EQ BAT_ERROR)
  {
    return (BAT_ERROR);
  }

  /* this func should be called after bat instance is created */
  if (inst_mt->instance_state < BAT_INSTANCE_READY)
  {
    BAT_TRACE_EVENT_P1 ("ERROR: the instance is not yet initialized! It's state is",
                        inst_mt->instance_state);
    return (BAT_ERROR);
  }

  inst_mt->unsolicited_result_cb = unsolicited_result_cb;
  *client = MAKE_UNS_CLNT_HNDL(instance);

  return (BAT_OK);
}


/*
+----------------------------------------------------------------------------+
| PROJECT :                    MODULE  : BINARY AT COMMAND LIBRARY           |
| STATE   : code               ROUTINE : bat_close                           |
+----------------------------------------------------------------------------+
PURPOSE :
  This function is used to close a binary AT command channel. This closing
  process includes closing of the connection to the modem. If the close process
  has been successfully performed, BAT Lib will call the signal callback
  function provided by bat_open() on the application side to indicate. The
  application can delete the BAT Lib instance only after successfully closing
  all the clients by calling bat_close().
*/

GLOBAL T_BAT_return bat_close  (T_BAT_client clnt_hndl)
{
  T_BAT_instance_maintain   *inst_mt  = NULL;
  T_BAT_client_maintain     *clnt_mt  = NULL;
  T_BATC_signal             ctrl_sig;
  T_BATC_close_client       param;
  T_BAT_return              reslt = BAT_ERROR;

  BAT_TRACE_FUNCTION ("bat_close()");

  if (bat_get_instance_from_instance_handle(GET_INST_HNDL_FROM_CLNT_HANDLE(clnt_hndl), &inst_mt))
  {
    return (BAT_ERROR);
  }
  
  /* if the channel to close is unsolicited code channel */
  if ((GET_CLNT_ID_FROM_CLNT_HANDLE(clnt_hndl) EQ BAT_BROADCAST_CHANNEL))
  {
    inst_mt->unsolicited_result_cb = NULL;
    /* if there is still data in the buffer for this channel, just ignore the data and empty the buffer */
    if ((inst_mt->buffer.buf_st EQ BAT_BUF_FILLED) AND (inst_mt->buffer.dest EQ clnt_hndl))
    {
      bat_change_buffer_state(inst_mt, BAT_BUF_EMPTY);
      inst_mt->config->adapter.gdd_if.gdd_signal_ready_rcv ((T_GDD_CON_HANDLE)(inst_mt->con_handle));
    }
    return (BAT_OK);
  }

  /* If the channel to close is a normal client channel */
  if (bat_get_client_from_client_handle(clnt_hndl, &clnt_mt) OR
      clnt_mt->client_state < BAT_CLIENT_ACTIVATING)
  {
    BAT_TRACE_ERROR ("ERROR: parameter wrong or BAT client is not yet open.");
    return (BAT_ERROR);
  }

  param.client_id = GET_CLNT_ID_FROM_CLNT_HANDLE(clnt_hndl);

  ctrl_sig.ctrl_params = BATC_CLOSE_CLIENT;
  ctrl_sig.params.ptr_close_client = &param;

  /* send control data to BAT ACI */
  reslt = bat_send_ctrl_data(GET_INST_HNDL_FROM_CLNT_HANDLE(clnt_hndl), &ctrl_sig);

  /* change back the necessary states if GDD busy */
  switch (reslt)
  {
    case (BAT_BUSY_RESOURCE):
    {
      bat_change_client_state(clnt_mt, BAT_CLIENT_BUSY);
      return (BAT_BUSY_RESOURCE);
    }
    case (BAT_ERROR):
    {
      return (BAT_ERROR);
    }
    case (BAT_OK):
    {
      break;
    }    
  }

  /* client state is now changed to Idle, the close function always performs, no asynchronized signal */
  bat_change_client_state(clnt_mt, BAT_CLIENT_IDLE);

  /* if the buffer is filled with data for this client, free it and send a ready signal to GDD */
  if ((inst_mt->buffer.buf_st EQ BAT_BUF_FILLED) AND (inst_mt->buffer.dest EQ clnt_hndl))
  {
    bat_change_buffer_state(inst_mt, BAT_BUF_EMPTY);
    inst_mt->config->adapter.gdd_if.gdd_signal_ready_rcv ((T_GDD_CON_HANDLE)(inst_mt->con_handle));
  }

  return (BAT_OK);
}

/*
+----------------------------------------------------------------------------+
| PROJECT :                    MODULE  : BINARY AT COMMAND LIBRARY           |
| STATE   : code               ROUTINE : bat_close                           |
+----------------------------------------------------------------------------+
PURPOSE :
  This function is used to send a BAT command. The final result is received
  asynchronously by response_cb(). 
*/

GLOBAL T_BAT_return bat_send   (T_BAT_client clnt_hndl, T_BAT_cmd_send *cmd)
{
  T_BAT_client_maintain *clnt_mt = NULL;
  T_BAT_return ret = BAT_ERROR;

  BAT_TRACE_FUNCTION ("bat_send()");

  /* find the client */
  if (bat_get_client_from_client_handle(clnt_hndl, &clnt_mt) NEQ BAT_OK)
  {
    BAT_TRACE_EVENT_P1("bat_send(): unknown client handle %d", clnt_hndl);
    return (BAT_ERROR);
  }

  if (clnt_mt->client_state NEQ BAT_CLIENT_READY)
  {
    TRACE_EVENT_P1("bat_send(): client is not in state READY, but in %d", clnt_mt->client_state);
    return (BAT_ERROR);
  }

  if (cmd EQ NULL)
  {
    TRACE_EVENT("bat_send(): cmd EQ NULL");
    return (BAT_ERROR);
  }

  bat_change_client_state(clnt_mt, BAT_CLIENT_SENDING);
  /* send the data (get buffer, fill buffer and send) */
  ret = bat_send_cmd_data(clnt_hndl, cmd);

  switch (ret)
  {
    case (BAT_BUSY_RESOURCE):
    {
      bat_change_client_state(clnt_mt, BAT_CLIENT_BUSY);
      break;
    }
    case (BAT_ERROR):
    {
      bat_change_client_state(clnt_mt, BAT_CLIENT_READY);
      break;
    }
    default:
    {
      break;
    }
  }
  return (ret);
}

/*
+----------------------------------------------------------------------------+
| PROJECT :                    MODULE  : BINARY AT COMMAND LIBRARY           |
| STATE   : code               ROUTINE : bat_close                           |
+----------------------------------------------------------------------------+
PURPOSE :
  This function is used to send control information, which is not directly
  related to a binary command request or response, to the BAT library. E.g.
  telling the BAT Lib to stop the currently running BAT command or to inform
  the BAT Lib that the application is no longer at BAT_BUSY_RESOURCE state,
  but is able to receive more data.
*/

GLOBAL T_BAT_return bat_ctrl (T_BAT_client clnt_hndl, T_BAT_ctrl *ctrl)
{
  T_BAT_instance_maintain *inst_mt = NULL;
  T_BAT_client_maintain   *clnt_mt = NULL;
  T_BATC_signal           ctrl_sig;
  T_BATC_abort_cmd        param;
  T_BAT_return ret = BAT_ERROR;

  BAT_TRACE_FUNCTION ("bat_ctrl()");

  /* check the ctrl param */
  if (ctrl EQ NULL)
  {
    return (BAT_ERROR);
  }

  /* get the correct instance */
  if (bat_get_instance_from_instance_handle (GET_INST_HNDL_FROM_CLNT_HANDLE(clnt_hndl), &inst_mt))
  {
    return (BAT_ERROR);
  }

  /* abort the running cmd */
  if (ctrl->event EQ BAT_ABORT)
  {
    /* get the correct client */
    if (bat_get_client_from_client_handle (clnt_hndl, &clnt_mt))
    {
      return (BAT_ERROR);
    }

    if (clnt_mt->client_state < BAT_CLIENT_SENDING)
    {
      BAT_TRACE_ERROR ("bat_ctrl(): No running cmd to abort.");
      return (BAT_ERROR);
    }

    /* set the abort cmd */
    param.client_id = GET_CLNT_ID_FROM_CLNT_HANDLE(clnt_hndl);
    ctrl_sig.ctrl_params = BATC_ABORT_CMD;
    ctrl_sig.params.ptr_abort_cmd = &param;

    ret = bat_send_ctrl_data(GET_INST_HNDL_FROM_CLNT_HANDLE(clnt_hndl), &ctrl_sig);

    switch (ret)
    {
      case (BAT_BUSY_RESOURCE):
      {
        /* change back the client state if the command can not be aborted */
        bat_change_client_state(clnt_mt, BAT_CLIENT_SENDING_AND_BUSY);
       break;
      }
      case (BAT_ERROR):
      {
        BAT_TRACE_ERROR ("bat_ctrl(): sending of ctrl data FAILED!");
        break;
      }
      default:
      {
        break;
      }
    }
    return (ret);
  }

  /* this is a ready signal */
  if (ctrl->event EQ BAT_APP_READY_RESOURCE)
  {
    if ((inst_mt->buffer.buf_st EQ BAT_BUF_FILLED) AND (inst_mt->buffer.dest EQ clnt_hndl))
    {
      BAT_TRACE_EVENT("bat_ctrl(): Ready signal received, buffered data will be sent!");

      /* handle the case when the buffer is filled with unsolicited code */
      if ((clnt_hndl EQ MAKE_UNS_CLNT_HNDL(GET_INST_HNDL_FROM_CLNT_HANDLE(clnt_hndl))))
      {
        if (inst_mt->unsolicited_result_cb (clnt_hndl, &(inst_mt->buffer.rsp)) NEQ BAT_BUSY_RESOURCE)
        {
          bat_change_buffer_state(inst_mt, BAT_BUF_EMPTY);

          inst_mt->config->adapter.gdd_if.gdd_signal_ready_rcv ((T_GDD_CON_HANDLE)(inst_mt->con_handle));
          BAT_TRACE_EVENT("bat_ctrl(): Buffer sent to APP and the ready signal sent to GDD!");
        }
        return (BAT_OK);
      }
      else /* signal the GDD that the BAT Lib is able to receive more data */
      {
        /* get the correct client */
        if (bat_get_client_from_client_handle (clnt_hndl, &clnt_mt))
        {
          return (BAT_ERROR);
        }
        if (clnt_mt->response_cb(clnt_hndl, &(inst_mt->buffer.rsp)) NEQ BAT_BUSY_RESOURCE)
        {
          bat_change_buffer_state(inst_mt, BAT_BUF_EMPTY);
          bat_change_client_state(clnt_mt, BAT_CLIENT_READY);

          inst_mt->config->adapter.gdd_if.gdd_signal_ready_rcv ((T_GDD_CON_HANDLE)(inst_mt->con_handle));
          BAT_TRACE_EVENT("bat_ctrl(): Buffer sent to APP and the ready signal sent to GDD!");
        }
        return (BAT_OK);
      }
    }
  }
  return (BAT_ERROR);
}


#endif




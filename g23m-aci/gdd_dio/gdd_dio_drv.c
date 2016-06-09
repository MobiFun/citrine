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
|  Purpose :  DIOv4 GDD functional interfaces 
+-----------------------------------------------------------------------------
*/

#define GDD_DIO_DRV_C

/*==== INCLUDES =============================================================*/

#include <string.h>
#include "typedefs.h"   /* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */

/* DIO stuff */
#include "dio.h"
#include "dio_il/dio_drv.h"

/* GDD_DIO stuff */

/* We must define P_DIO_H, in order to avoid inclusion of p_dio.h.
   This is necessary due to the special fact, that "dio.h", which is included
   in the header "dio_io/dio_drv.h", mirrors the header "p_dio.h". */
#define P_DIO_H

#include "gdd_dio.h"
#include "gdd_dio_rxf.h"
#include "gdd_dio_txf.h"
#include "gdd_dio_queue.h"
#include "gdd_dio_con_mgr.h"  /* for gdd_dio_con_mgr_mark_dead() */




/*==== CONSTANTS ============================================================*/

/*
 * SEE_TRACE_DURING_SYS_INIT, if defined, allows the showing of traces during
 * system initialization.  The definition should be removed for normal 
 * operation.
 * SEE_DIO_READ_WRITE_DATA, if defined, displays the data content of DIO_READ and
 * DIO_WRITE.  The definition should be removed for normal operation.
 */
/* #define SEE_TRACE_DURING_SYS_INIT */
/* #define SEE_DIO_READ_WRITE_DATA */

/*==== DEFINITIONS ==========================================================*/

#ifdef SEE_TRACE_DURING_SYS_INIT

#ifdef TRACE_FUNCTION
#undef TRACE_FUNCTION
#define TRACE_FUNCTION  TRACE_ERROR
#endif /* #ifdef TRACE_FUNCTION */

#ifdef TRACE_EVENT
#undef TRACE_EVENT
#define TRACE_EVENT  TRACE_ERROR
#endif /* #ifdef TRACE_EVENT */

#endif /* #ifdef SEE_TRACE_DURING_SYS_INIT */


#define IS_DRV_NUM_BAT(n)         (((n&DIO_DRV_MASK)==DIO_DRV_BAT)?TRUE:FALSE)
#define IS_DRV_NUM_APP(n)         (((n&DIO_DRV_MASK)==DIO_DRV_APP)?TRUE:FALSE)
#define IS_DIO_TYPE_PKT(n)        (((n&DIO_TYPE_MASK)==DIO_TYPE_PKT)?TRUE:FALSE)


/* The following driver definitions should be removed as soon as the DIO IL
   contains these definitiones */
#ifndef DIO_DRV_TCP
#define DIO_DRV_TCP      0x0A000000 /* driver number - TCP/IP adapter         */
#endif /* DIO_DRV_TCP */
#ifndef DIO_DRV_SOCK
#define DIO_DRV_SOCK     0x0B000000 /* driver number - Socket adapter         */
#endif /* DIO_DRV_SOCK */
#ifndef DIO_DRV_SOCKCFG
#define DIO_DRV_SOCKCFG  0x0C000000 /* driver number - Socket conf. adapter   */
#endif /* DIO_DRV_SOCKCFG */



/*==== TYPES ================================================================*/


/*==== PROTOTYPES GDD DIO INTERFACE FUNCTIONS ================================*/

void dio_user_ready_gdd(U32 device_range_start, U32 device_range_end,
                        U16 drv_handle, T_DRV_CB_FUNC signal_callback);
U16 dio_user_not_ready_gdd(U32 device_range_start, U32 device_range_end);
void dio_exit_gdd(void);
U16 dio_set_rx_buffer_gdd(U32 device, T_dio_buffer * buffer);
U16 dio_read_gdd(U32 device, T_DIO_CTRL * control_info, T_dio_buffer **buffer);
U16 dio_write_gdd(U32 device, T_DIO_CTRL * control_info, T_dio_buffer *buffer);
U16 dio_get_tx_buffer_gdd(U32 device, T_dio_buffer ** buffer);
U16 dio_clear_gdd(U32 device);
U16 dio_flush_gdd(U32 device);
U16 dio_get_capabilities_gdd(U32 device, T_DIO_CAP ** capabilities);
U16 dio_set_config_gdd(U32 device, T_DIO_DCB * dcb);
U16 dio_get_config_gdd(U32 device, T_DIO_DCB * dcb);
U16 dio_close_device_gdd(U32 device);


/*==== GLOBAL VARS ==========================================================*/

/*==== LOCAL VARS ===========================================================*/


static T_DIO_FUNC dio_func_bat =
{ 
                              dio_user_ready_gdd,
                              dio_user_not_ready_gdd,
                              dio_exit_gdd,
                              dio_set_rx_buffer_gdd,
                              dio_read_gdd,
                              dio_write_gdd,
                              dio_get_tx_buffer_gdd,
                              dio_clear_gdd,
                              dio_flush_gdd,
                              dio_get_capabilities_gdd,
                              dio_set_config_gdd,
                              dio_get_config_gdd,
  dio_close_device_gdd
};


                              
/*==== LOCAL FUNCS ==========================================================*/


/*
+------------------------------------------------------------------------------
| Function    : inst_num_from_dev_id
+------------------------------------------------------------------------------
| Description : get instance number from  device id
|
| Parameters  : con_data    - connection data
|
| Return      : >= 0    - Success, instance ID
                < 0     - Error (invalid device)
+------------------------------------------------------------------------------
*/
int inst_num_from_dev_id(U32 device)
{
  U32 drv_num = device & DIO_DRV_MASK;

  switch (drv_num)
  {
  case DIO_DRV_BAT:
    return GDD_INST_BAT;
  case DIO_DRV_APP:
    return GDD_INST_APP;
  case DIO_DRV_TCP:
    return GDD_INST_TCP;
  case DIO_DRV_SOCK:
    return GDD_INST_SOCK;
  case DIO_DRV_SOCKCFG:
    return GDD_INST_SOCKCFG;
  default:
    return -1;
  }
}


/*
+------------------------------------------------------------------------------
| Function    : get_instance_data
+------------------------------------------------------------------------------
| Description : Local helper function to get the instance data of a device
|
| Parameters  : gdd_dio_data    - pointer to pointer to gdd_dio_data instance
|                                 (output parameter)
|               device          - DIO device ID
|
| Return      : FALSE           - Success (gdd_dio_data has been set)
|               TRUE            - Failed (invalid device)
+------------------------------------------------------------------------------
*/
U16 get_instance_data(T_GDD_DIO_DATA ** /*out*/ gdd_dio_data, U32 device)
{
  int inst = inst_num_from_dev_id(device);
  if(inst >= 0)
  {
    *gdd_dio_data = &(gdd_dio_data_base[inst]);
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}


/*
+------------------------------------------------------------------------------
| Function    : get_connection_data
+------------------------------------------------------------------------------
| Description : Local helper function to map from DIO device ID to the
|               connection data pointer.
|
| Parameters  : device          - DIO device ID
|               gdd_dio_data    - pointer to pointer to gdd_dio_data instance
|               con_data        - pointer to connection data (output parameter)
|
| Return      : FALSE           - Success
|               TRUE            - Failed
+------------------------------------------------------------------------------
*/
U16 get_connection_data(U32 device, T_GDD_DIO_CON_DATA ** /*out*/ con_data)
{
  int i;
  T_GDD_DIO_DATA *gdd_dio_data = 0;

  if(get_instance_data(&gdd_dio_data, device))
  {
    TRACE_ERROR("[GDD] Bad DIO device number");
    return DRV_INTERNAL_ERROR;
  }

  if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_READY)
  {
    TRACE_ERROR("[GDD] State error: instance not ready");
    return DRV_INTERNAL_ERROR;
  }

  for(i=0; i < gdd_dio_data->max_con; ++i)
  {
    if(gdd_dio_data->con_arr[i].dio_device EQ device)
    {
      *con_data = &(gdd_dio_data->con_arr[i]);
      return DRV_OK;
    }
  }

  TRACE_ERROR("[GDD] Cannot get connection info for this device");
  return DRV_INTERNAL_ERROR;
}


/*
+------------------------------------------------------------------------------
| Function    : send_gdd_signal
+------------------------------------------------------------------------------
| Description : Local helper function to send a GDD signal to a GDD client.
|
| Parameters  : con_data    - connection data
|               sig_type    - type of signal to send
+------------------------------------------------------------------------------
*/

LOCAL void send_gdd_signal(T_GDD_DIO_CON_DATA * con_data, T_GDD_SIGTYPE sig_type)
{
  T_GDD_SIGNAL sig;
  char * sig_type_str;
  
  switch (sig_type)
  {
  case GDD_SIGTYPE_CONNECTION_OPENED:  sig_type_str = "GDD_SIGTYPE_CONNECTION_OPENED"; break;
  case GDD_SIGTYPE_SEND_BUF_AVAILABLE: sig_type_str = "GDD_SIGTYPE_SEND_BUF_AVAILABLE"; break;
  case GDD_SIGTYPE_CONNECTION_FAILED:  sig_type_str = "GDD_SIGTYPE_CONNECTION_FAILED"; break;
  case GDD_SIGTYPE_BUF_SENT:           sig_type_str = "GDD_SIGTYPE_BUF_SENT"; break;
  default: sig_type_str = "ununsed signal type"; break;
  }  
  
  TRACE_USER_CLASS_P2(TC_SIGNALS, "[GDD] Sending signal %s to GDD client (con_handle=0x%4x)",
                      sig_type_str, con_data->dio_device);
  
  sig.sig = sig_type;
  con_data->sig_cb(con_data->dio_device, sig);
}


/*
+------------------------------------------------------------------------------
| Function    : init_dio_driver_instance
+------------------------------------------------------------------------------
| Description : Local helper initialize a dio driver instance
+------------------------------------------------------------------------------
*/
LOCAL void init_dio_driver_instance(T_GDD_DIO_DATA * gdd_dio_data, U16 max_devices)
{
  gdd_dio_data->max_con   = (U8) max_devices;
  gdd_dio_data->ker.state = GDD_DIO_KER_INIT;
}



/*==== FUNCTIONS EXPORTED VIA dio_export() ==================================*/

/*
+------------------------------------------------------------------------------
| Function    : dio_user_ready_gdd
+------------------------------------------------------------------------------
| Description : The function allows the driver to open the channels that have 
|               channel numbers of the given range.  Each driver has its ready 
|               function. The function is not called directly by the user of 
|               the DIO interface. The DIO interface layer calls the ready 
|               functions of the DIO driver when dio_user_init() is called.
|               The driver uses the given signal_callback function for channel 
|               of the given channel number range. The driver stores the 
|               drv_handle and passes it in the T_DRV_SIGNAL structure of the 
|               Signal parameter to the calling process every time the callback
|               function is called.  This function needs to be provided by each
|               DIO driver in order to enable more than one user using the DIO 
|               interface.
|
| Parameters  : device_range_start  - First channel number of a range of 
|                                     channels which can be handled by a DIO 
|                                     user now
|               device_range_end    -	Last channel number of a range of 
|                                     channels which can be handled by a 
|                                     DIO user now
|               drv_handle          - Unique handle of the DIO user.
|               signal_callback     - This parameter points to the function 
|                                     that is called at the time an event 
|                                     occurs that is to be signaled.
|
| Return      : none
|
+------------------------------------------------------------------------------
*/
void dio_user_ready_gdd
(
  U32           device_range_start, 
  U32           device_range_end, 
  U16           drv_handle, 
  T_DRV_CB_FUNC signal_callback
)
{
  T_GDD_DIO_DATA *gdd_dio_data = 0;

  TRACE_FUNCTION("[GDD] dio_user_ready_gdd()");

  /* Only packet data is currently supported - this might change later */
  if(IS_DIO_TYPE_PKT(device_range_start) EQ FALSE)
  {
    TRACE_ERROR("[GDD] Non-packet device not supported");
    return;
  }  

  if(get_instance_data(&gdd_dio_data, device_range_start))
  {
    TRACE_ERROR("[GDD] Bad DIO device number");
    return;
  }

  if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_INIT)
  {
    TRACE_ERROR("[GDD] DIO driver not initialized");
    return;
  }

  gdd_dio_data->inst_id            = (T_GDD_INST_ID)inst_num_from_dev_id(device_range_start);
  gdd_dio_data->drv_num            = device_range_start & DIO_DRV_MASK;
  gdd_dio_data->device_range_start = device_range_start;
  gdd_dio_data->device_range_end   = device_range_end;
  gdd_dio_data->drv_handle         = drv_handle;
  gdd_dio_data->signal_callback    = signal_callback;

  gdd_dio_data->ker.state = GDD_DIO_KER_READY;
  
} /* dio_user_ready_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_user_not_ready_gdd
+------------------------------------------------------------------------------
| Description : The function is used to clear the signal_callback/channel 
|               association which was previously set with dio_user_ready_gdd().
|               That means the signal_callback function of channels of the 
|               given channel number range must not be call any more. All 
|               channels of the given channel number range need to be closed 
|               before the function can be called.  The function returns DRV_OK
|               if it was able to clear the signal_callback/channel 
|               associations successfully. In case there is no channel in the 
|               given channel number range the function also returns DRV_OK.  
|               If there is still an open channel in the given channel number 
|               range then the function returns DRV_INVALID_PARAMS. In this 
|               case the signal_callback function can still be used by the 
|               driver.
|
| Parameters  : device_range_start  - First channel number of a range of 
|                                     channels which can no longer be handled  
|                                     by a DIO user now
|               device_range_end    -	Last channel number of a range of 
|                                     channels which can no longer be handled  
|                                     by a DIO user now
|
| Return      : DRV_OK              - Initialization successful
|               DRV_INVALID_PARAMS  - User operation can not be terminated yet
|               DRV_INTERNAL_ERROR  - Internal driver error
|
+------------------------------------------------------------------------------
*/
U16 dio_user_not_ready_gdd
(
  U32 device_range_start, 
  U32 device_range_end
)
{
  T_GDD_DIO_DATA *gdd_dio_data = 0;

  TRACE_FUNCTION("[GDD] dio_user_not_ready_gdd()");

  if(get_instance_data(&gdd_dio_data, device_range_start))
  {
    TRACE_ERROR("Bad DIO device number");
    return DRV_INTERNAL_ERROR;
  }

  if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_READY)
  {
    TRACE_ERROR("[GDD] State error: instance not ready");
  }

  gdd_dio_data->ker.state = GDD_DIO_KER_INIT;

  return DRV_OK;
} /* dio_user_not_ready_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_exit_gdd
+------------------------------------------------------------------------------
| Description : This function is called when this device driver is no longer 
|               longer required.  DIO_IL calls this function after both send/
|               receive buffers in the driver are flushed and all BAT devies 
|               are closed.
|
| Parameters  : none
|
| Return      : none
|
+------------------------------------------------------------------------------
*/
void dio_exit_gdd(void)
{
  int inst;

  TRACE_FUNCTION("[GDD] dio_exit_gdd()");
  
  /* Shutdown all instances */
  for(inst=0; inst<GDD_NUM_INSTS; ++inst)
  {
    gdd_dio_data_base[inst].ker.state = GDD_DIO_KER_DEAD;
  }

} /* dio_exit_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_set_rx_buffer_gdd
+------------------------------------------------------------------------------
| Description : This function provides a receive buffer to the driver. The 
|               driver uses this buffer to store the received data of the 
|               specified device. The function should always return immediately
|               after overtaking the buffer, without waiting for data.  To 
|               avoid reception gaps more than one receive buffer should be 
|               provided to the driver via several calls of this function. 
|               The provided buffers should be used in the order they were 
|               provided.  If the driver is not able to take over the provided
|               buffer (e.g. because its internal data buffer queue is full) 
|               the function returns DRV_BUFFER_FULL.  The driver uses the 
|               DRV_SIGTYPE_READ signal when data is received.
|
| Parameters  : device              - Data device number or DLC number
|               buffer              -	Data buffer description
|
| Return      : DRV_OK              - Initialization successful
|               DRV_BUFFER_FULL     - Buffer queue is full
|               DRV_INVALID_PARAMS  - User operation can not be terminated yet
|               DRV_INTERNAL_ERROR  - Internal driver error
|               DRV_NOTCONFIGURED   - The device is not yet configured
|
+------------------------------------------------------------------------------
*/
U16 dio_set_rx_buffer_gdd
(
  U32             device, 
  T_dio_buffer *  buffer
)
{
  T_GDD_DIO_CON_DATA *con_data = 0;
  U16 pos = 0;

  TRACE_USER_CLASS(TC_FUNC_DATA_FLOW, "[GDD] dio_set_rx_buffer_gdd()");

  if(get_connection_data(device, &con_data) NEQ DRV_OK)
  {
    return DRV_INTERNAL_ERROR;
  }

  if(gdd_dio_enqueue(buffer, &con_data->rx_queue, &pos) EQ FALSE)
  {
    return DRV_BUFFER_FULL;
  }
  else
  {
    if(con_data->con_state == GDD_DIO_CON_CONNECT)
    {
      con_data->con_state = GDD_DIO_CON_READY;
      
      send_gdd_signal(con_data, GDD_SIGTYPE_CONNECTION_OPENED);
    } 
    if(con_data->wait_send_buf)
    {
      send_gdd_signal(con_data, GDD_SIGTYPE_SEND_BUF_AVAILABLE);
      
      con_data->wait_send_buf = FALSE;
    }

    return DRV_OK;
  }
} /* dio_set_rx_buffer_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_read_gdd
+------------------------------------------------------------------------------
| Description : This function returns a receive buffer and control information.
|               It should always return immediately after changing internal 
|               states, without waiting for any more data. The receive buffers
|               should be returned in the same order as provided with the 
|               dio_set_rx_buffer() calls (First-In-First-Out). The returned 
|               buffer is not in control of the driver any more.  The buffer 
|               should be returned even if it is empty. If there is no receive 
|               buffer in control of the driver any more then buffer is set to 
|               NULL. In this case only control information is delivered.
|
| Parameters  : device              - Data device number or DLC number
|               control_info        - The driver copies control information
|                                     into the provided control buffer
|               buffer              -	Data buffer description
|
| Return      : DRV_OK              - Initialization successful
|               DRV_INVALID_PARAMS  - User operation can not be terminated yet
|               DRV_INTERNAL_ERROR  - Internal driver error
|               DRV_NOTCONFIGURED   - The device is not yet configured
|
+------------------------------------------------------------------------------
*/
U16 dio_read_gdd
(
  U32             device, 
  T_DIO_CTRL *    control_info, 
  T_dio_buffer ** buffer
)
{
  T_GDD_DIO_CON_DATA *con_data = 0;

  TRACE_USER_CLASS(TC_FUNC_DATA_FLOW, "[GDD] dio_read_gdd()");

  if(get_connection_data(device, &con_data) NEQ DRV_OK)
  {
    return DRV_INTERNAL_ERROR;
  }

  /* Set control info : do nothing for now! */
  
  if(gdd_dio_dequeue(&(con_data->rx_queue), buffer) EQ FALSE)
  {
    TRACE_ERROR("[GDD] RX queue is empty");
    
    con_data->con_state = GDD_DIO_CON_READY;
    return DRV_INTERNAL_ERROR;
  }
  else
  {
    send_gdd_signal(con_data, GDD_SIGTYPE_BUF_SENT);
  }
  
  if(con_data->con_state NEQ GDD_DIO_CON_CLOSE)
  {
    con_data->con_state = GDD_DIO_CON_READY;
  }

  return DRV_OK;
} /* dio_read_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_write_gdd
+------------------------------------------------------------------------------
| Description : This function provides a send buffer to the driver which 
|               contains data to send. This function should return immediately
|               after overtaking the buffer.  To avoid transmission gaps more 
|               than one send buffer should be provided to the driver via 
|               several calls of this function. The provided send buffers 
|               should be sent in the order they were provided. If the driver 
|               is not able to take over the provided buffer (e.g. because its
|               internal buffer queue is full) the function returns 
|               DRV_BUFFER_FULL.  If buffer is set to NULL then the driver only
|               copies the provided control information.  The driver uses the 
|               DRV_SIGTYPE_WRITE signal when the data of the buffer is sent.
|
| Parameters  : device              - Data device number or DLC number
|               control_info        - The driver copies control information 
|                                     into the provided control buffer
|               buffer              -	Data buffer description
|
| Return      : DRV_OK              - Initialization successful
|               DRV_BUFFER_FULL     - Buffer queue is full
|               DRV_INVALID_PARAMS  - User operation can not be terminated yet
|               DRV_INTERNAL_ERROR  - Internal driver error
|               DRV_NOTCONFIGURED   - The device is not yet configured
|
+------------------------------------------------------------------------------
*/
U16 dio_write_gdd
(
  U32             device, 
  T_DIO_CTRL *    control_info, 
  T_dio_buffer *  buf
)
{
  T_GDD_DIO_CON_DATA *con_data = 0;
  U16 pos = 0;

  TRACE_USER_CLASS(TC_FUNC_DATA_FLOW, "[GDD] dio_write_gdd()");

  if(get_connection_data(device, &con_data) NEQ DRV_OK)
  {
    return DRV_INTERNAL_ERROR;
  }

  /* Ignore control information for now */

  if(gdd_dio_enqueue(buf, &con_data->tx_queue, &pos) EQ FALSE)
  {
    TRACE_ERROR("[GDD] TX buffer queue full");
    return DRV_BUFFER_FULL;
  }

  gdd_dio_tx_receive_buf(con_data->dio_device, buf);

  return DRV_OK;
} /* dio_write_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_get_tx_buffer_gdd
+------------------------------------------------------------------------------
| Description : This function returns a send buffer provided via dio_write(). 
|               It should always return immediately after changing internal 
|               states, without waiting for any outstanding events. The send 
|               buffers should be re-turned in the same order as provided with
|               the dio_write() calls (First-In-First-Out). The returned send 
|               buffer is not in control of the driver any more.  If there is 
|               no send buffer in control of the driver any more then buffer is
|               set to NULL.
|
| Parameters  : device              - Data device number or DLC number
|               buffer              -	Data buffer description
|
| Return      : DRV_OK              - Initialization successful
|               DRV_INVALID_PARAMS  - User operation can not be terminated yet
|               DRV_INTERNAL_ERROR  - Internal driver error
|               DRV_NOTCONFIGURED   - The device is not yet configured
|
+------------------------------------------------------------------------------
*/
U16 dio_get_tx_buffer_gdd
(
  U32             device, 
  T_dio_buffer ** buffer
)
{
  T_GDD_DIO_CON_DATA *con_data = 0;

  TRACE_USER_CLASS(TC_FUNC_DATA_FLOW, "[GDD] dio_get_tx_buffer_gdd()");

  if(get_connection_data(device, &con_data) NEQ DRV_OK)
  {
    return DRV_INTERNAL_ERROR;
  }

  if(gdd_dio_dequeue(&con_data->tx_queue, buffer) EQ FALSE)
  {
    TRACE_ERROR("[GDD] TX buffer queue empty");
    return DRV_INTERNAL_ERROR;
  }

  return DRV_OK;
} /* dio_get_tx_buffer_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_clear_gdd
+------------------------------------------------------------------------------
| Description : Clear the hardware send buffer of the device.
|               - currently not needed for GDD_DIO (BAT adapter).
|
| Parameters  : device              - Data device number or DLC number
|
| Return      : DRV_OK              - Initialization successful
|               DRV_INVALID_PARAMS  - User operation can not be terminated yet
|               DRV_INTERNAL_ERROR  - Internal driver error
|               DRV_NOTCONFIGURED   - The device is not yet configured
|               DRV_INPROCESS       - The driver is busy clearing the buffer
|
+------------------------------------------------------------------------------
*/
U16 dio_clear_gdd
(
  U32   device
)
{

  TRACE_FUNCTION("[GDD] dio_clear_gdd()");

  /* Do nothing ! */

  return DRV_OK;
} /* dio_clear_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_flush_gdd
+------------------------------------------------------------------------------
| Description : Flush the hardware send buffer of the device.
|               - currently not needed for GDD_DIO (BAT adapter).
|
| Parameters  : device              - Data device number or DLC number.
|
| Return      : DRV_OK              - Initialization successful
|               DRV_INVALID_PARAMS  - User operation can not be terminated yet
|               DRV_INTERNAL_ERROR  - Internal driver error
|               DRV_NOTCONFIGURED   - The device is not yet configured
|               DRV_INPROCESS       - The driver is busy clearing the buffer
|
+------------------------------------------------------------------------------
*/
U16 dio_flush_gdd
(
  U32   device
)
{

  TRACE_FUNCTION("[GDD] dio_flush_gdd()");

  /* Do nothing ! */

  return DRV_OK;
} /* dio_flush_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_get_capabilities_gdd
+------------------------------------------------------------------------------
| Description : This function is used to retrieve the capabilities of a device.
|               It is not allowed to change these values neither by the
|               driver nor by the protocol stack.
|
| Parameters  : device              - Data device number or DLC number.
|               capabilities        - Pointer to the device capabilities.
|
| Return      : DRV_OK              - Initialization successful
|               DRV_INVALID_PARAMS  - User operation can not be terminated yet
|               DRV_INTERNAL_ERROR  - Internal driver error
|
+------------------------------------------------------------------------------
*/
U16 dio_get_capabilities_gdd
(
  U32                 device,
  T_DIO_CAP **  capabilities
)
{
  T_GDD_DIO_CON_DATA * con_data;

  TRACE_FUNCTION("[GDD] dio_get_capabilities_gdd()");

  if(get_connection_data(device, &con_data) NEQ DRV_OK)
  {
    return DRV_INTERNAL_ERROR;
  }

  if(inst_num_from_dev_id(device) < 0)
  {
    TRACE_ERROR("[GDD] Bad device");
    send_gdd_signal(con_data, GDD_SIGTYPE_CONNECTION_FAILED);
    return DRV_INTERNAL_ERROR;
  }
  
  /* Only packet data is supported */
  if(IS_DIO_TYPE_PKT(device))
  {
    *capabilities = (T_DIO_CAP *) & con_data->dio_cap;
    return DRV_OK;
  }
  else
  {
    *capabilities = NULL;
    send_gdd_signal(con_data, GDD_SIGTYPE_CONNECTION_FAILED);
    return DRV_INVALID_PARAMS;
  }  
} /* dio_get_capability_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_set_config_gdd
+------------------------------------------------------------------------------
| Description : This function is used to configure a device (transmission rate,
|               flow control, etc). A device can be configured at any time.  
|               dcb points to a Device Control Block. The parameters that can 
|               be configured are included in the Device Control Block.  If any
|               value of the configuration is out of range, not supported or 
|               invalid in combination with any other value of the 
|               configuration, the function returns DRV_INVALID_PARAMS.  Each 
|               device needs to be configured after the reception of a 
|               DRV_SIGTYPE_CONNECT signal. Only dio_get_capabilities(), 
|               dio_set_config() and dio_close_device() can be called while the
|               device is not configured. All other device-specific functions 
|               return DRV_NOTCONFIGURED.
|
| Parameters  : device              - Data device number or DLC number
|               dcb                 - Pointer to a Device Control Block
|
| Return      : DRV_OK              - Initialization successful
|               DRV_INVALID_PARAMS  - User operation can not be terminated yet
|               DRV_INTERNAL_ERROR  - Internal driver error
|
+------------------------------------------------------------------------------
*/
U16 dio_set_config_gdd
(
  U32         device,
  T_DIO_DCB * dcb
)
{
  T_GDD_DIO_CON_DATA * con_data;

  TRACE_FUNCTION("[GDD] dio_set_config_gdd()");

  if(get_connection_data(device, &con_data) NEQ DRV_OK)
  {
    return DRV_INTERNAL_ERROR;
  }

  if (dcb EQ NULL)
  {
    TRACE_ERROR("[GDD] dcb is NULL");
    send_gdd_signal(con_data, GDD_SIGTYPE_CONNECTION_FAILED);
    return DRV_INTERNAL_ERROR;
  }

  if (DIO_TYPE_PKT != dcb->device_type)
  {
    TRACE_ERROR("[GDD] bad dcb->device_type");
    send_gdd_signal(con_data, GDD_SIGTYPE_CONNECTION_FAILED);
    return DRV_INVALID_PARAMS;
  }

  /* Ignore dbc->sleep_mode. */

  return DRV_OK;
} /* dio_set_config_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_get_config_gdd
+------------------------------------------------------------------------------
| Description : This function is used to retrieve the configuration of a 
|               device.  The driver copies the configuration into the Device 
|               Control Block provided with dcb.
|
| Parameters  : device              - Data device number or DLC number
|               dcb                 - Pointer to a Device Control Block
|
| Return      : DRV_OK              - Initialization successful
|               DRV_INVALID_PARAMS  - User operation can not be terminated yet
|               DRV_INTERNAL_ERROR  - Internal driver error
|               DRV_NOTCONFIGURED   - The device is not yet configured
|
+------------------------------------------------------------------------------
*/
U16 dio_get_config_gdd
(
  U32         device,
  T_DIO_DCB * dcb
)
{
  T_GDD_DIO_DATA * gdd_dio_data;

  TRACE_FUNCTION("[GDD] dio_get_config_gdd()");

  if(get_instance_data(&gdd_dio_data, device))
  {
    TRACE_ERROR("[GDD] Bad DIO device number");
    return DRV_INTERNAL_ERROR;
  }
  
  if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_READY)
  {
    TRACE_ERROR("[GDD] State error: instance not ready");
    return GDD_DIO_NOT_READY;
  }

  dcb->device_type = DIO_TYPE_PKT;
  dcb->sleep_mode  = DIO_SLEEP_DISABLE;

  return DRV_OK;
} /* dio_get_config_gdd */

/*
+------------------------------------------------------------------------------
| Function    : dio_close_device_gdd
+------------------------------------------------------------------------------
| Description : This function is used to close a device. The driver returns 
|               DRV_OK if it was able to close the device successfully. In case
|               the specified device does not exist the driver also returns 
|               DRV_OK.  If the driver still controls a protocol stack buffer 
|               for this device then it returns DRV_INVALID_PARAMS. In this 
|               case the device is not closed. In order to get the remaining 
|               buffers the protocol stack needs to call the functions 
|               dio_read() and dio_get_tx_buffer().
|
| Parameters  : device              - Data device number or DLC number
|
| Return      : DRV_OK              - Initialization successful
|               DRV_INVALID_PARAMS  - User operation can not be terminated yet
|               DRV_INTERNAL_ERROR  - Internal driver error
|
+------------------------------------------------------------------------------
*/
U16 dio_close_device_gdd
(
  U32   device
)
{
  T_GDD_DIO_CON_DATA * con_data;

  TRACE_FUNCTION("[GDD] dio_close_device_gdd()");

  if(get_connection_data(device, &con_data) NEQ DRV_OK)
  {
    return DRV_INTERNAL_ERROR;
  }

  /* Inform client: in case the connection was closing,
     we mark the connection as dead. If not, it means that the
     connection (which was not yet established) has failed. */
  if(con_data->con_state EQ GDD_DIO_CON_CLOSE)
  {
    gdd_dio_con_mgr_mark_dead(con_data->dio_device);
  }
  else
  {
    send_gdd_signal(con_data, GDD_SIGTYPE_CONNECTION_FAILED);
  }
  
  return DRV_OK;
} /* dio_close_device_bat */



/*==== EXPORTED FUNCTIONS ===================================================*/

/*
+------------------------------------------------------------------------------
| Function    : dio_init_bat
+------------------------------------------------------------------------------
| Description : The function initializes the BAT driver.  The 
|               function returns DRV_INITIALIZED if the driver has already been
|               initialized and is ready to be used or is already in use.  In 
|               case of initialization failure (e.g. the configuration given 
|               with drv_init can not be used) the function returns 
|               DRV_INITFAILURE. In this case the driver can not be used.
|
| Parameters  : none
|
| Return      : DRV_OK          - Initialization successful
|               DRV_INITIALIZED - Interface already initialized
|               DRV_INITFAILURE - Initialization failed
|
+------------------------------------------------------------------------------
*/
U16 dio_init_bat
(
  T_DIO_DRV * drv_init
)
{
  T_GDD_DIO_DATA * gdd_dio_data = &gdd_dio_data_base[GDD_INST_BAT];

  TRACE_FUNCTION("[GDD] dio_init_bat()");

  if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_DEAD)
  {
    TRACE_FUNCTION("Instance 'BAT' already initialized");
    return DRV_INITIALIZED;
  }

  init_dio_driver_instance(gdd_dio_data, drv_init->max_devices);

  return DRV_OK;
} /* dio_init_bat */

/*
+------------------------------------------------------------------------------
| Function    : dio_export_bat
+------------------------------------------------------------------------------
| Description : The function returns, through dio_func, a list of functions 
|               that are exported by the BAT driver.  DIO_IL calls this
|               function after dio_init_bat call return DRV_OK.
|
| Parameters  : dio_func        - Return as a pointer to the list of functions  
|                                 exported by the driver.
|
| Return      : none
|
+------------------------------------------------------------------------------
*/
void dio_export_bat
(
  T_DIO_FUNC ** dio_func
)
{ 
  TRACE_FUNCTION("[GDD] dio_export_bat");

  *dio_func = (T_DIO_FUNC*) &dio_func_bat;

} /* dio_export_bat */


/*
+------------------------------------------------------------------------------
| Function    : dio_init_app
+------------------------------------------------------------------------------
| Description : The function initializes the APP driver.  The 
|               function returns DRV_INITIALIZED if the driver has already been
|               initialized and is ready to be used or is already in use.  In 
|               case of initialization failure (e.g. the configuration given 
|               with drv_init can not be used) the function returns 
|               DRV_INITFAILURE. In this case the driver can not be used.
|
| Parameters  : none
|
| Return      : DRV_OK          - Initialization successful
|               DRV_INITIALIZED - Interface already initialized
|               DRV_INITFAILURE - Initialization failed
|
+------------------------------------------------------------------------------
*/
U16 dio_init_app
(
  T_DIO_DRV * drv_init
)
{
  T_GDD_DIO_DATA * gdd_dio_data = &gdd_dio_data_base[GDD_INST_APP];

  TRACE_FUNCTION("[GDD] dio_init_app()");

  if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_DEAD)
  {
    TRACE_FUNCTION("[GDD] Instance 'APP' already initialized");
    return DRV_INITIALIZED;
  }

  init_dio_driver_instance(gdd_dio_data, drv_init->max_devices);

  return DRV_OK;
} /* dio_init_app */


/*
+------------------------------------------------------------------------------
| Function    : dio_export_app
+------------------------------------------------------------------------------
| Description : The function returns, through dio_func, a list of functions 
|               that are exported by the APP driver.  DIO_IL calls this
|               function after dio_init_bat call return DRV_OK.
|
| Parameters  : dio_func        - Return as a pointer to the list of functions  
|                                 exported by the driver.
|
| Return      : none
|
+------------------------------------------------------------------------------
*/
void dio_export_app
(
  T_DIO_FUNC ** dio_func
)
{ 
  TRACE_FUNCTION("[GDD] dio_export_app()");

  *dio_func = (T_DIO_FUNC*) &dio_func_bat;

} /* dio_export_app */


/*
+------------------------------------------------------------------------------
| Function    : dio_init_sock
+------------------------------------------------------------------------------
| Description : The function initializes the TCP driver.  The 
|               function returns DRV_INITIALIZED if the driver has already been
|               initialized and is ready to be used or is already in use.  In 
|               case of initialization failure (e.g. the configuration given 
|               with drv_init can not be used) the function returns 
|               DRV_INITFAILURE. In this case the driver can not be used.
|
| Parameters  : none
|
| Return      : DRV_OK          - Initialization successful
|               DRV_INITIALIZED - Interface already initialized
|               DRV_INITFAILURE - Initialization failed
|
+------------------------------------------------------------------------------
*/
U16 dio_init_tcp
(
  T_DIO_DRV * drv_init
)
{
  T_GDD_DIO_DATA * gdd_dio_data = &gdd_dio_data_base[GDD_INST_TCP];

  TRACE_FUNCTION("[GDD] dio_init_tcp()");

  if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_DEAD)
  {
    TRACE_FUNCTION("[GDD] Instance 'TCP' already initialized");
    return DRV_INITIALIZED;
  }

  init_dio_driver_instance(gdd_dio_data, drv_init->max_devices);

  return DRV_OK;
} /* dio_init_tcp */


/*
+------------------------------------------------------------------------------
| Function    : dio_export_tcp
+------------------------------------------------------------------------------
| Description : The function returns, through dio_func, a list of functions 
|               that are exported by the TCP driver.  DIO_IL calls this
|               function after dio_init_bat call return DRV_OK.
|
| Parameters  : dio_func        - Return as a pointer to the list of functions  
|                                 exported by the driver.
|
| Return      : none
|
+------------------------------------------------------------------------------
*/
void dio_export_tcp
(
  T_DIO_FUNC ** dio_func
)
{ 
  TRACE_FUNCTION("[GDD] dio_export_tcp()");

  *dio_func = (T_DIO_FUNC*) &dio_func_bat;

} /* dio_export_tcp */


/*
+------------------------------------------------------------------------------
| Function    : dio_init_sock
+------------------------------------------------------------------------------
| Description : The function initializes the SOCK driver.  The 
|               function returns DRV_INITIALIZED if the driver has already been
|               initialized and is ready to be used or is already in use.  In 
|               case of initialization failure (e.g. the configuration given 
|               with drv_init can not be used) the function returns 
|               DRV_INITFAILURE. In this case the driver can not be used.
|
| Parameters  : none
|
| Return      : DRV_OK          - Initialization successful
|               DRV_INITIALIZED - Interface already initialized
|               DRV_INITFAILURE - Initialization failed
|
+------------------------------------------------------------------------------
*/
U16 dio_init_sock
(
  T_DIO_DRV * drv_init
)
{
  T_GDD_DIO_DATA * gdd_dio_data = &gdd_dio_data_base[GDD_INST_SOCK];

  TRACE_FUNCTION("[GDD] dio_init_sock()");

  if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_DEAD)
  {
    TRACE_FUNCTION("[GDD] Instance 'SOCK' already initialized");
    return DRV_INITIALIZED;
  }

  init_dio_driver_instance(gdd_dio_data, drv_init->max_devices);

  return DRV_OK;
} /* dio_init_sock */


/*
+------------------------------------------------------------------------------
| Function    : dio_export_sock
+------------------------------------------------------------------------------
| Description : The function returns, through dio_func, a list of functions 
|               that are exported by the SOCK driver.  DIO_IL calls this
|               function after dio_init_bat call return DRV_OK.
|
| Parameters  : dio_func        - Return as a pointer to the list of functions  
|                                 exported by the driver.
|
| Return      : none
|
+------------------------------------------------------------------------------
*/
void dio_export_sock
(
  T_DIO_FUNC ** dio_func
)
{ 
  TRACE_FUNCTION("[GDD] dio_export_sock()");

  *dio_func = (T_DIO_FUNC*) &dio_func_bat;

} /* dio_export_sock */


/*
+------------------------------------------------------------------------------
| Function    : dio_init_sockcfg
+------------------------------------------------------------------------------
| Description : The function initializes the SOCKCFG driver.  The 
|               function returns DRV_INITIALIZED if the driver has already been
|               initialized and is ready to be used or is already in use.  In 
|               case of initialization failure (e.g. the configuration given 
|               with drv_init can not be used) the function returns 
|               DRV_INITFAILURE. In this case the driver can not be used.
|
| Parameters  : none
|
| Return      : DRV_OK          - Initialization successful
|               DRV_INITIALIZED - Interface already initialized
|               DRV_INITFAILURE - Initialization failed
|
+------------------------------------------------------------------------------
*/
U16 dio_init_sockcfg
(
  T_DIO_DRV * drv_init
)
{
  T_GDD_DIO_DATA * gdd_dio_data = &gdd_dio_data_base[GDD_INST_SOCKCFG];

  TRACE_FUNCTION("[GDD] dio_init_sockcfg()");

  if (gdd_dio_data->ker.state NEQ GDD_DIO_KER_DEAD)
  {
    TRACE_FUNCTION("[GDD] Instance 'SOCKCFG' already initialized");
    return DRV_INITIALIZED;
  }

  init_dio_driver_instance(gdd_dio_data, drv_init->max_devices);

  return DRV_OK;
} /* dio_init_sockcfg */


/*
+------------------------------------------------------------------------------
| Function    : dio_export_sockcfg
+------------------------------------------------------------------------------
| Description : The function returns, through dio_func, a list of functions 
|               that are exported by the SOCKCFG driver.  DIO_IL calls this
|               function after dio_init_bat call return DRV_OK.
|
| Parameters  : dio_func        - Return as a pointer to the list of functions  
|                                 exported by the driver.
|
| Return      : none
|
+------------------------------------------------------------------------------
*/
void dio_export_sockcfg
(
  T_DIO_FUNC ** dio_func
)
{ 
  TRACE_FUNCTION("[GDD] dio_export_sockcfg()");

  *dio_func = (T_DIO_FUNC*) &dio_func_bat;

} /* dio_export_sockcfg */



#ifdef _SIMULATION_

/* Set the signal callback for all GDD drivers - for simulation only !!! */
GLOBAL void set_bat_driver_callback_for_simulation(T_DRV_CB_FUNC signal_callback)
{
  gdd_dio_data_base[GDD_INST_BAT].signal_callback = signal_callback;
  gdd_dio_data_base[GDD_INST_APP].signal_callback = signal_callback;
  gdd_dio_data_base[GDD_INST_TCP].signal_callback = signal_callback;
  gdd_dio_data_base[GDD_INST_SOCK].signal_callback = signal_callback;
  gdd_dio_data_base[GDD_INST_SOCKCFG].signal_callback = signal_callback;
}

#endif /* _SIMULATION_ */

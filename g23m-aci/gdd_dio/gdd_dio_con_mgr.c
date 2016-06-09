/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_con_mgr.c
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
|  Purpose  : Implements connection management functions
+-----------------------------------------------------------------------------
*/


#define ENTITY_GDD_DIO

/*==== INCLUDES =============================================================*/

#include "typedefs.h"   /* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */
#include "prim.h"       /* to get the definitions of used SAP and directions */
#include "dti.h"

#include "gdd_dio.h"       /* to get the global entity definitions */
#include "gdd_dio_queue.h"
#include "gdd_dio_con_mgr.h"

#include <string.h>


/*==== DEFINITIONS ==========================================================*/

#define	ENTER_CRITICAL_SECTION(sem) if (gdd_enter_critical_section(sem))return -1;
#define	LEAVE_CRITICAL_SECTION(sem) if (gdd_leave_critical_section(sem))return -1;


/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

static T_HANDLE   sem_GDD_DIO_CON;

/*==== PRIVATE FUNCTIONS ====================================================*/

LOCAL int get_free_connection_slot(T_GDD_DIO_DATA * gdd_dio_data,
                                    T_GDD_DIO_CON_DATA ** con_data /*output*/);
static void gdd_semaphore_err (void);
static int gdd_enter_critical_section (T_HANDLE sem);
static int gdd_leave_critical_section (T_HANDLE sem);


/*==== PUBLIC FUNCTIONS =====================================================*/


/*  Initializes the connection manager */

GLOBAL void gdd_dio_con_mgr_init (T_GDD_DIO_DATA * gdd_dio_data)
{
  int i;
  T_GDD_DIO_CON_DATA * con_data = gdd_dio_data->con_arr;
  
  TRACE_FUNCTION( "[GDD] conn_init()" );

  for(i = 0; i < gdd_dio_data->max_con; ++i, ++con_data)
  {
    con_data->con_state = GDD_DIO_CON_DEAD;
  }

  sem_GDD_DIO_CON  = vsi_s_open (VSI_CALLER "SEM_GDD_CON",1);
  if (sem_GDD_DIO_CON EQ VSI_ERROR)
    vsi_o_ttrace(VSI_CALLER TC_EVENT, "can´t open semaphore \"SEM_GDD_CON\"");
}


/* Setup a new DIO connection */

GLOBAL GDD_RESULT gdd_dio_con_mgr_new
( T_GDD_DIO_DATA * gdd_dio_data,
  T_GDD_CON_HANDLE * con_handle,
  const T_GDD_CAP * cap,
  T_GDD_RECEIVE_DATA_CB rcv_cb,
  T_GDD_SIGNAL_CB sig_cb )
{
  T_GDD_DIO_CON_DATA * con_data = 0;

  TRACE_FUNCTION("[GDD] conn_new()");

  if(get_free_connection_slot(gdd_dio_data, &con_data) NEQ 0)
  {
    TRACE_ERROR("Failed to get new connection slot");
    return GDD_NO_CONNECTION_SLOT;
  }

  gdd_dio_queue_clear(&con_data->rx_queue);
  gdd_dio_queue_clear(&con_data->tx_queue);

  con_data->wait_send_buf = FALSE;

  con_data->rcv_cb = rcv_cb;
  con_data->sig_cb = sig_cb;

  con_data->dio_cap.device_type = DIO_TYPE_PKT;
  con_data->dio_cap.device_flags = 0;
  con_data->dio_cap.mtu_control = 0;
  con_data->dio_cap.mtu_data = (U16)((T_GDD_DIO_CAP *)cap)->mtu_size;
  con_data->dio_cap.driver_name = "GDD";
  
  (*con_handle) = con_data->dio_device;


  gdd_dio_send_signal_to_dio(con_data, DRV_SIGTYPE_CONNECT);

  return GDD_OK;
}


/* Close a connection */

GLOBAL GDD_RESULT gdd_dio_con_mgr_close
( T_GDD_CON_HANDLE con_handle )
{
  T_GDD_INST_ID inst = (T_GDD_INST_ID)inst_num_from_dev_id(con_handle);
  T_GDD_DIO_CON_DATA * con_data;

  TRACE_FUNCTION( "[GDD] gdd_dio_con_mgr_close()" );

  con_data = get_con_data(&(gdd_dio_data_base[inst]), con_handle);  
  if(con_data EQ NULL)
  {
    TRACE_ERROR("Failed to get connection data");
    return GDD_INTERNAL_ERROR;
  }

  con_data->con_state = GDD_DIO_CON_CLOSE;

  gdd_dio_send_signal_to_dio(con_data, DRV_SIGTYPE_DISCONNECT);

  return GDD_OK;
}


/* Mark a connection as dead */

GDD_RESULT gdd_dio_con_mgr_mark_dead( T_GDD_CON_HANDLE con_handle )
{
  T_GDD_INST_ID inst = (T_GDD_INST_ID)inst_num_from_dev_id(con_handle);
  T_GDD_DIO_CON_DATA * con_data;

  TRACE_FUNCTION( "[GDD] gdd_dio_con_mgr_mark_dead()" );

  con_data = get_con_data(&(gdd_dio_data_base[inst]), con_handle);  
  if(con_data EQ NULL)
  {
    TRACE_ERROR("Failed to get connection data");
    return GDD_INTERNAL_ERROR;
  }

  /* Clear the connection slot
     - it's sufficient to set state and nullify handle */
  con_data->con_state = GDD_DIO_CON_DEAD;

  return GDD_OK;
}


/* Check if any of the connections is (still) open or in connecting state. */

BOOL gdd_dio_con_mgr_has_open_connection
( const T_GDD_DIO_DATA * gdd_dio_data )
{
  int i;
  T_GDD_DIO_CON_DATA * con_data = gdd_dio_data->con_arr;
  
  TRACE_FUNCTION( "[GDD] gdd_dio_con_mgr_has_open_connection()" );

  for(i = 0; i < gdd_dio_data->max_con; ++i, ++con_data)
  {
    if(con_data->con_state EQ GDD_DIO_CON_READY ||
       con_data->con_state EQ GDD_DIO_CON_SENDING ||
       con_data->con_state EQ GDD_DIO_CON_CONNECT)
    {
      return TRUE;
    }
  }
  return FALSE;
}


/* Get the connection data for a given instance & handle */

GLOBAL T_GDD_DIO_CON_DATA * get_con_data(const T_GDD_DIO_DATA * gdd_dio_data, 
                                         T_GDD_CON_HANDLE con_handle)
{
  int i;

  /* Deliberately NO tracing */

  for(i=0; i<gdd_dio_data->max_con; ++i)
  {
    if(gdd_dio_data->con_arr[i].dio_device EQ con_handle)
      return &gdd_dio_data->con_arr[i];
  }
  return NULL;
}



/* Get the connection data for a given handle only */

GLOBAL T_GDD_DIO_CON_DATA * get_con_data_from_handle(T_GDD_CON_HANDLE con_handle)
{
  T_GDD_INST_ID inst;

  /* Deliberately NO tracing */

  inst = (T_GDD_INST_ID)inst_num_from_dev_id(con_handle);

  if(inst < 0 || inst >= GDD_NUM_INSTS)
  {
    return 0;
  }

  return get_con_data(&(gdd_dio_data_base[inst]), con_handle);
}


/* Send a DIO signal via the specificed connection */

void gdd_dio_send_signal_to_dio
(T_GDD_DIO_CON_DATA * con_data, U16 sig_type)
{
  T_DRV_SIGNAL      drv_signal;
  T_GDD_DIO_DATA * inst_data;
  char * sig_type_str;

  TRACE_USER_CLASS(TC_FUNC_DATA_FLOW, "[GDD] gdd_dio_send_signal_to_dio()");

  inst_data = &gdd_dio_data_base[inst_num_from_dev_id(con_data->dio_device)];

  switch (sig_type)
  {
  case DRV_SIGTYPE_WRITE:      sig_type_str = "DRV_SIGTYPE_WRITE"; break;
  case DRV_SIGTYPE_READ:       sig_type_str = "DRV_SIGTYPE_READ"; break;
  case DRV_SIGTYPE_CONNECT:    sig_type_str = "DRV_SIGTYPE_CONNECT"; break;
  case DRV_SIGTYPE_DISCONNECT: sig_type_str = "DRV_SIGTYPE_DISCONNECT"; break;
  default: sig_type_str = "ununsed signal type"; break;
  }  

  TRACE_USER_CLASS_P2(TC_SIGNALS, "[GDD] Sending signal %s to DIO IL (con_handle=0x%4x)", sig_type_str, con_data->dio_device);

  drv_signal.SignalType = sig_type;
  drv_signal.DrvHandle  = inst_data->drv_handle;
  drv_signal.DataLength = sizeof(U32);
  drv_signal.UserData   = &(con_data->dio_device);
  (*(inst_data->signal_callback))(&drv_signal);
}


/*==== PRIVATE FUNCTIONS =====================================================*/

/*
+------------------------------------------------------------------------------
| Function    : get_free_connection_slot
+------------------------------------------------------------------------------
| Description : Find the next free connection slot for given instance.
|
| Parameters  : gdd_dio_data  - pointer to instance data
|               con_data      - pointer to pointer to connection data (output)
|
| Returns     : 0      - Success (con_data has been set)
|              -1      - Failed (no slot found)
+------------------------------------------------------------------------------
*/
LOCAL int get_free_connection_slot(T_GDD_DIO_DATA * gdd_dio_data,
                                    T_GDD_DIO_CON_DATA ** con_data /*output*/)
{
  int i;
    
  TRACE_FUNCTION( "[GDD] get_free_connection_slot()" );

  ENTER_CRITICAL_SECTION(sem_GDD_DIO_CON);

  for(i = 0; i < gdd_dio_data->max_con; ++i)
  {
    if(gdd_dio_data->device_range_start+i <= gdd_dio_data->device_range_end)
    {
      if(gdd_dio_data->con_arr[i].con_state EQ GDD_DIO_CON_DEAD)
      {
        gdd_dio_data->con_arr[i].dio_device = gdd_dio_data->device_range_start+i;
        *con_data = &(gdd_dio_data->con_arr[i]);
        (*con_data)->con_state = GDD_DIO_CON_CONNECT;

        LEAVE_CRITICAL_SECTION(sem_GDD_DIO_CON);        
        return 0;
      }
    }
  }

  LEAVE_CRITICAL_SECTION(sem_GDD_DIO_CON);

  return -1;
}


/*
+------------------------------------------------------------------------------
| Function    : gdd_semaphore_err
+------------------------------------------------------------------------------
| Description : Handle a semaphore error
+------------------------------------------------------------------------------
*/
static void gdd_semaphore_err (void)
{
  static UCHAR out = 0;
  if (!out)
  {
    out = 1;
    vsi_o_ttrace(VSI_CALLER TC_EVENT, "semaphore error");
  }
}


/*
+------------------------------------------------------------------------------
| Function    : gdd_enter_critical_section
+------------------------------------------------------------------------------
| Description : Enters a critical section.
|
| Parameters  : sem    - Semaphore handle
|
| Returns     : 0      - Success
|              -1      - Failure
+------------------------------------------------------------------------------
*/
static int gdd_enter_critical_section (T_HANDLE sem)
{
  if (vsi_s_get (VSI_CALLER sem) NEQ VSI_OK)
  {
    gdd_semaphore_err();
    return -1;
  }
  else
  {
    return 0;
  }
}


/*
+------------------------------------------------------------------------------
| Function    : gdd_leave_critical_section
+------------------------------------------------------------------------------
| Description : Leaves a critical section.
|
| Parameters  : sem    - Semaphore handle
|
| Returns     : 0      - Success
|              -1      - Failure
+------------------------------------------------------------------------------
*/
static int gdd_leave_critical_section (T_HANDLE sem)
{
  if (vsi_s_release (VSI_CALLER sem) NEQ VSI_OK)
  {
    gdd_semaphore_err();
    return -1;
  }
  else
  {
    return 0;
  }
}

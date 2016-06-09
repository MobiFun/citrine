/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_con_mgr.h
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
|  Purpose  : Connecion handling functions of the DIO BAT adapter
|             These functions form the interface of the connection manager,
|             whose responsability is to create DIO connections, assign
|             device numbers, etc.
+-----------------------------------------------------------------------------
*/

#ifndef GDD_DIO_CON_MGR_H
#define GDD_DIO_CON_MGR_H

/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_con_mgr_init
+------------------------------------------------------------------------------
| Description : Initializes the connection manager
|
| Parameters  : -
|
+------------------------------------------------------------------------------
*/
EXTERN void gdd_dio_con_mgr_init
(
  T_GDD_DIO_DATA * gdd_dio_data
);

/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_con_mgr_new
+------------------------------------------------------------------------------
| Description : Create a new DIO connection
|
| Assumptions :    gdd_dio_data->ker.state EQ GDD_DIO_KER_READY
|               && gdd_dio_init_flag[inst] EQ TRUE
|
| Parameters  : gdd_dio_data                - pointer to instance data
|               con_handle                  - output value: new conn. handle
|               cap                         - capabilities required
|               rcv_cb                      - receive callback
|               sig_cb                      - signal callback
|
| Returns     : GDD_OK                      - Connection succesfully created
|               GDD_NO_CONNECTION_SLOT      - Connection failed because we
|                                             have already the max. number of
|                                             connections.
+------------------------------------------------------------------------------
*/
EXTERN GDD_RESULT gdd_dio_con_mgr_new
(
  T_GDD_DIO_DATA * gdd_dio_data,
  T_GDD_CON_HANDLE * con_handle,
  const T_GDD_CAP * cap,
  T_GDD_RECEIVE_DATA_CB rcv_cb,
  T_GDD_SIGNAL_CB sig_cb
);


/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_con_mgr_close
+------------------------------------------------------------------------------
| Description : Close a connection
|
| Parameters  : con_handle                  - connection handle
|
| Returns     : GDD_OK                      - Connection succesfully created
|               >GDD_OK                     - Error
+------------------------------------------------------------------------------
*/
EXTERN GDD_RESULT gdd_dio_con_mgr_close( T_GDD_CON_HANDLE con_handle );


/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_con_mgr_close
+------------------------------------------------------------------------------
| Description : Mark a connection as dead
|
| Parameters  : con_handle                  - connection handle
|
| Returns     : GDD_OK                      - OK
|               >GDD_OK                     - Error
+------------------------------------------------------------------------------
*/
EXTERN GDD_RESULT gdd_dio_con_mgr_mark_dead( T_GDD_CON_HANDLE con_handle );



/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_con_mgr_has_open_connection
+------------------------------------------------------------------------------
| Description : Check if any of the connections is (still) open or in
|               connecting state.
|
| Returns     : TRUE     - At least one connection is open
|               FALSE    - None of the connections is open
+------------------------------------------------------------------------------
*/
BOOL gdd_dio_con_mgr_has_open_connection( const T_GDD_DIO_DATA * gdd_dio_data );


/*
+------------------------------------------------------------------------------
| Function    : get_con_data
+------------------------------------------------------------------------------
| Description : Get the connection data for a given instance & handle
|
| Parameters  : gdd_dio_data                - pointer to instance data
|               con_handle                  - output value: new conn. handle
|
| Returns     : NULL     - Connection not found / invalid handle
|               NOT NULL - pointer to connection
+------------------------------------------------------------------------------
*/
EXTERN T_GDD_DIO_CON_DATA * get_con_data
(
  const T_GDD_DIO_DATA * gdd_dio_data, 
  T_GDD_CON_HANDLE con_handle
);


/*
+------------------------------------------------------------------------------
| Function    : get_con_data_from_handle
+------------------------------------------------------------------------------
| Description : Get the connection data for a given andle
|
| Parameters  : con_handle                  - output value: new conn. handle
|
| Returns     : NULL     - Connection not found / invalid handle
|               NOT NULL - pointer to connection
+------------------------------------------------------------------------------
*/
EXTERN T_GDD_DIO_CON_DATA * get_con_data_from_handle(T_GDD_CON_HANDLE con_handle);


/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_send_signal_to_dio
+------------------------------------------------------------------------------
| Description : Send a DIO signal via the specificed connection
| 
| Parameters  : con_data    - pointer to the connection data
|               sig_type    - Signal type
+------------------------------------------------------------------------------
*/
EXTERN void gdd_dio_send_signal_to_dio(T_GDD_DIO_CON_DATA * con_data, U16 sig_type);


#endif /* !GDD_DIO_CON_MGR_H */


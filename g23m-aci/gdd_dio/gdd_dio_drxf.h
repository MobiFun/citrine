/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_drxf.h
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
|  Purpose  : Definitions for gdd_dio_drxf.c
+-----------------------------------------------------------------------------
*/

#ifndef GDD_DIO_DRXF_H
#define GDD_DIO_DRXF_H

/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_rx_send_buffer
+------------------------------------------------------------------------------
| Description : Signal handler for GDD_DIO_SIGNAL_RECEIVE_DATA
|               This function tries to send a buffer to the client of GDD_DIO.
|
| Parameters  : con_handle   - connection handle
|               buf          - buffer
+------------------------------------------------------------------------------
*/
EXTERN void gdd_dio_drx_sig_receive_data(T_GDD_CON_HANDLE con_handle, T_dio_buffer * buf);

/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_drx_ready_to_rcv
+------------------------------------------------------------------------------
| Description : Called when the client is signalling that she is ready to receive.
|
| Parameters  : con_handle   - connection handle
+------------------------------------------------------------------------------
*/
EXTERN void gdd_dio_drx_ready_to_rcv(T_GDD_CON_HANDLE  con_handle);


#endif /* !GDD_DIO_DRXF_H */


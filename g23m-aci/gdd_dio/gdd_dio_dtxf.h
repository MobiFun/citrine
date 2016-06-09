/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_dtxf.h
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
|  Purpose  : Definitions for gdd_dio_dtxf.c
+-----------------------------------------------------------------------------
*/

#ifndef GDD_DIO_DTXF_H
#define GDD_DIO_DTXF_H

/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_dtx_get_send_buffer
+------------------------------------------------------------------------------
| Description : Get a send buffer
+------------------------------------------------------------------------------
*/
EXTERN GDD_RESULT gdd_dio_dtx_get_send_buffer
(
  T_GDD_CON_HANDLE  con_handle,
  T_GDD_BUF **      buf,
  U16               data_size
);

/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_rx_send_buffer
+------------------------------------------------------------------------------
| Description : This function sends a buffer to DIO.
|               It is called from the GDD interface side.
+------------------------------------------------------------------------------
*/
EXTERN GDD_RESULT gdd_dio_dtx_send_buffer(T_GDD_CON_HANDLE con_handle, T_GDD_BUF * buf);

#endif /* !GDD_DIO_DTXF_H */

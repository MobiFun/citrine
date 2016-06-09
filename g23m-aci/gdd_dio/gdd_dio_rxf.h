/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_rxf.h
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
|  Purpose  : Definitions for gdd_dio_rxf.c
+-----------------------------------------------------------------------------
*/

#ifndef GDD_DIO_RXF_H
#define GDD_DIO_RXF_H

/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_rx_send_buffer
+------------------------------------------------------------------------------
| Description : Signal handler for GDD_DIO_SIGNAL_SEND_DATA
|               This function sends a buffer to DIO IL.
|
| Parameters  : con_handle   - connection handle
|               buf          - buffer
+------------------------------------------------------------------------------
*/
EXTERN void gdd_dio_rx_sig_send_data(T_GDD_CON_HANDLE con_handle, T_dio_buffer * buf);



#endif /* !GDD_DIO_RXF_H */


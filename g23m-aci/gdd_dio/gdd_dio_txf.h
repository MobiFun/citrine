/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_txf.h
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
|  Purpose  : Definitions for gdd_dio_txf.c
+-----------------------------------------------------------------------------
*/

#ifndef GDD_DIO_TXF_H
#define GDD_DIO_TXF_H

/*
+------------------------------------------------------------------------------
| Function    : gdd_dio_tx_receive_buf
+------------------------------------------------------------------------------
| Description : This functions pushes a buffer from DIO IL further UP
|               (towards the DRX service).
|
| Parameters  : con_handle               - Connection Handle
|               buf                      - The TX buffer from DIL IL.
|
| Return      : GDD_OK                   - OK
|               > GDD_OK                 - Error
|
+------------------------------------------------------------------------------
*/
EXTERN void gdd_dio_tx_receive_buf
(T_GDD_CON_HANDLE con_handle, T_dio_buffer * buf);

EXTERN void gdd_dio_tx_send_drv_sigtype_write(T_GDD_CON_HANDLE con_handle);

#endif /* !GDD_DIO_TXF_H */


/*
+-----------------------------------------------------------------------------
|  File     : dio_il_psi_stub.h
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

#ifndef DIO_IL_PSI_STUB_H
#define DIO_IL_PSI_STUB_H

/*==== DEFINITIONS ==========================================================*/

/*
 * position of Driver number - needed to retrieve driver number from device id
 */
#define DIO_DRV_POS      24


/*==== FUNCTIONS ============================================================*/


/*
+------------------------------------------------------------------------------
| Function    : psi_stub_dio_sign_callback
+------------------------------------------------------------------------------
| Description : The function psi_stub_dio_sign_callback() is the callback 
|               function of the DIO driver to indicate events of the driver.  
|               This function is called in interrupt context. It converts the  
|               given opcode to a signal.
|
| Parameters  : pointer to signal
|
+------------------------------------------------------------------------------
*/
EXTERN void psi_stub_dio_sign_callback (T_DRV_SIGNAL *SigPtr);


/*
+------------------------------------------------------------------------------
| Function    : Test helper function to switch on/off automatic provision with
|               RX buffers after read.
+------------------------------------------------------------------------------
*/
EXTERN void psi_stub_send_rx_buf_after_read(BOOL b);


/*
+------------------------------------------------------------------------------
| Function    : Test helper function to trigger the provision of an RX buffer
|               on the last used connection in the PSI STUB.
+------------------------------------------------------------------------------
*/
EXTERN void psi_stub_provide_rx_buf();


/*
+------------------------------------------------------------------------------
| Function    : Copy a DIO buffer
+------------------------------------------------------------------------------
*/
EXTERN void copy_dio_buf(const T_dio_buffer * buf_in, T_dio_buffer ** buf_out);

/*
+------------------------------------------------------------------------------
| Function    : Allocate an RX buffer for PSI simulation
+------------------------------------------------------------------------------
| Description : The allocated buffer has two segments: the first one has
|               2 bytes (for the protocol ID), and the second one has
|               GDD_DIO_MTU_SIZE for the payload.
+------------------------------------------------------------------------------
*/
EXTERN void allocate_rx_dio_buf(T_dio_buffer ** buf_out);

/*
+------------------------------------------------------------------------------
| Function    : General allocator for a DIO buffer
+------------------------------------------------------------------------------
*/
EXTERN void allocate_dio_buf(T_dio_buffer ** buf_out, const U16 seg_size[], U16 num_seg);


/*
+------------------------------------------------------------------------------
| Function    : Free a DIO buffer and all memory associated with it.
+------------------------------------------------------------------------------
*/
EXTERN void free_dio_buf(T_dio_buffer ** buf);


#endif /* !DIO_IL_PSI_STUB_H */


/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
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
|  Purpose :  Definitions for uart_dtxf.c
+-----------------------------------------------------------------------------
*/

#ifndef UART_DTXF_H
#define UART_DTXF_H


EXTERN T_ESD_STATE dtx_get_esd_state(void);

EXTERN void dtx_set_esd_state( T_ESD_STATE new_esd_state );

EXTERN void dtx_init (void);

EXTERN void dtx_free_resources (void);

EXTERN void dtx_exit (void);

EXTERN void dtx_allocate_resources (void);

EXTERN void dtx_calculate_size_multiplier (T_desc2* desc_to_send,
                                           T_DATA_FLOW_STATE data_flow);

#ifndef DTILIB
EXTERN UBYTE dtx_get_channel_id(void);
#endif /* !DTILIB */

#endif /* !UART_DTXF_H */


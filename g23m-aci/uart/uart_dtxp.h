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
|  Purpose :  Definitions for uart_dtxp.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_DTXP_H
#define UART_DTXP_H

#ifdef DTILIB
EXTERN void sig_dti_dtx_tx_buffer_ready_ind(void);
EXTERN void sig_dti_dtx_tx_buffer_full_ind(void);

#else  /* DTILIB */
EXTERN void dtx_dti_getdata_req ( T_DTI_GETDATA_REQ *dti_getdata_req );

#ifdef _SIMULATION_
EXTERN void dtx_psend_dti_data_test_ind ( T_DTI_DATA_IND *dti_data_ind );
#endif /* _SIMULATION_ */
#endif /* DTILIB */

#endif /* !UART_DTXP_H */


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
|  Purpose :  Definitions for uart_drxp.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_DRXP_H
#define UART_DRXP_H


EXTERN void drx_dti_data_test_req ( T_DTI2_DATA_TEST_REQ *dti_data_test_req );


EXTERN void drx_dti_data_req ( T_DTI2_DATA_REQ *dti_data_req );

#ifdef DTILIB
EXTERN void sig_dti_drx_data_received_ind(T_DTI2_DATA_IND *dti_data2_ind);
#endif /* DTILIB */

#endif /* !UART_DRXP_H */


/*
+-----------------------------------------------------------------------------
|  Project :  DCM and TCPIP
|  Project :  ACI
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
|  Purpose :  PSA_DCM-related definitions and declarations for the ACI
+-----------------------------------------------------------------------------
*/

#ifndef PSA_DCM_H
#define PSA_DCM_H

#include "dcm.h"
#include "socket_api.h"
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== PROTOTYPES =================================================*/
EXTERN BOOL psaDCM_open_conn_cnf(T_DCM_RET result ,
                                T_SOCK_API_INSTANCE api_instance);
EXTERN BOOL psaDCM_close_conn_cnf(T_DCM_RET result , 
                                 T_SOCK_API_INSTANCE api_instance);
EXTERN BOOL psaDCM_get_current_conn_cnf(T_DCM_RET result, 
                                        T_SOCK_API_INSTANCE api_instance,
                                        T_DCM_ENV_CTRL_BLK *dcm_evt_blk);
EXTERN BOOL psaDCM_error_ind(T_DCM_STATUS_IND_MSG* dcm_error_ind_msg, 
                             T_SOCK_API_INSTANCE api_instance );

/*==== EXPORT =====================================================*/


/*==== EOF =======================================================*/

#endif

/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI
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
|  Description :  defines the signalling functions of the protocol stack adapter for dcm control.
|            
+-----------------------------------------------------------------------------
*/

/***********************************  INCLUDES ********************************************/
#include "aci_all.h"
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"


/**************************** LOCAL VARIABLE DEFINITION  **********************/
/**************************** EXTERN VARIABLE DEFINITION  *********************/
EXTERN T_DCM_RET dcm_handle_message(T_DCM_HDR *msg_p);
/**************************** LOCAL FUCNTION DEFINITION  **********************/
/************************** EXTERN FUCNTION DEFINITION  ***********************/


/******************************************************************************/
GLOBAL void psa_dcm_open_conn_req(T_DCM_OPEN_CONN_REQ* dcm_open_conn_req)
{
  T_DCM_OPEN_CONN_REQ_MSG dcm_open_conn_req_msg;

  TRACE_FUNCTION("psa_dcm_open_conn_req()");

  memset(&dcm_open_conn_req_msg,0x00,sizeof(T_DCM_OPEN_CONN_REQ_MSG));

  dcm_open_conn_req_msg.hdr.msg_id = DCM_OPEN_CONN_REQ_MSG;
  dcm_open_conn_req_msg.conn_req.api_instance = dcm_open_conn_req->api_instance;
  dcm_open_conn_req_msg.conn_req.bearer_select= dcm_open_conn_req->bearer_select;
  dcm_open_conn_req_msg.conn_req.profile_number = dcm_open_conn_req->profile_number;
  dcm_open_conn_req_msg.conn_req.dcm_info_conn = dcm_open_conn_req->dcm_info_conn;

  dcm_handle_message((T_DCM_HDR*)&dcm_open_conn_req_msg);

  PFREE(dcm_open_conn_req);
}


/******************************************************************************/
GLOBAL void psa_dcm_close_conn_req(T_DCM_CLOSE_CONN_REQ* dcm_close_conn_req)
{
  T_DCM_CLOSE_CONN_REQ_MSG dcm_close_conn_req_msg;

  TRACE_FUNCTION("psa_dcm_close_conn_req()");

  memset(&dcm_close_conn_req_msg, 0x00, sizeof(T_DCM_CLOSE_CONN_REQ_MSG));

  dcm_close_conn_req_msg.hdr.msg_id = DCM_CLOSE_CONN_REQ_MSG;
  dcm_close_conn_req_msg.close_req.api_instance  = dcm_close_conn_req->api_instance;
  dcm_close_conn_req_msg.close_req.bearer_handle = dcm_close_conn_req->bearer_handle;

  dcm_handle_message((T_DCM_HDR*)&dcm_close_conn_req_msg);

  PFREE(dcm_close_conn_req);
}


/******************************************************************************/
GLOBAL void psa_dcm_get_current_conn_req(T_DCM_GET_CURRENT_CONN_REQ *dcm_get_current_conn_req)
{
  T_DCM_GET_CURRENT_CONN_REQ_MSG dcm_get_current_conn_req_msg;

  TRACE_FUNCTION("psa_dcm_get_current_conn_req()");

  memset(&dcm_get_current_conn_req_msg, 0x00, sizeof(T_DCM_GET_CURRENT_CONN_REQ_MSG));
     
  dcm_get_current_conn_req_msg.hdr.msg_id = DCM_GET_CURRENT_CONN_REQ_MSG;
  dcm_get_current_conn_req_msg.current_conn_req.api_instance = dcm_get_current_conn_req->api_instance;
  dcm_get_current_conn_req_msg.current_conn_req.bearer_handle = dcm_get_current_conn_req->bearer_handle;

  dcm_handle_message((T_DCM_HDR*)&dcm_get_current_conn_req_msg);

  PFREE(dcm_get_current_conn_req);
}



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
|  Description :  This file contains Dcm header file
+-----------------------------------------------------------------------------
*/


#ifndef __DCM_UTILS_H__
#define __DCM_UTILS_H__

#include "prim.h"
#include "dcm.h"

/**
 * Maximum number of IPU.
 */
#define DCM_MAX_NUMBER_IPU	 5

/*
*  Application infomation state
*/
#define ROW_FREE		0
#define ROW_ASSIGNED	1
#define ROW_CLOSING	2


/* msg_header */
typedef struct
{
    U8 msg_id;
} T_DCM_HDR;


/* Message ID */
#define DCM_OPEN_CONN_REQ_MSG                    1
#define DCM_CLOSE_CONN_REQ_MSG                   2
#define DCM_GET_CURRENT_CONN_REQ_MSG             3
#define DCM_OPEN_CONN_CNF_MSG                    4
#define DCM_CLOSE_CONN_CNF_MSG                   5
#define DCM_GET_CURRENT_CONN_CNF_MSG             6
#define DCM_ERROR_IND_MSG                        7
#define DCM_NEXT_CMD_READY_MSG                   8
#define DCM_NEXT_CMD_STOP_MSG                    9


/* Message structure*/
typedef struct
{
  T_DCM_HDR	          hdr;      /* Message header */
  T_DCM_OPEN_CONN_REQ conn_req; /* DCM open req message body */
} T_DCM_OPEN_CONN_REQ_MSG;


/* Message structure */
typedef struct
{

  T_DCM_HDR            hdr;       /* Message header */
  T_DCM_CLOSE_CONN_REQ close_req; /* DCM close req message body */
} T_DCM_CLOSE_CONN_REQ_MSG;


/* Message structure*/
typedef struct
{
  T_DCM_HDR                  hdr;              /* Message header*/
  T_DCM_GET_CURRENT_CONN_REQ current_conn_req; /* DCM get current conn req message body */
} T_DCM_GET_CURRENT_CONN_REQ_MSG;


/* Message structure */
typedef struct
{
  /* Message header */
  T_DCM_HDR   hdr;
  /* Error code. ETSI GSM 07.07  based*/
  S32 error;
  /* Result */
  T_DCM_RET result;
} T_DCM_STATUS_IND_MSG;


T_DCM_RET dcm_process_unwaited_events_state_intermediate_conn(T_DCM_HDR * msg_p);
T_DCM_RET dcm_process_open_conn_event(T_DCM_OPEN_CONN_REQ_MSG *open_conn_p);
T_DCM_RET dcm_process_close_conn_event(T_DCM_CLOSE_CONN_REQ_MSG *close_conn_p);
T_DCM_RET dcm_process_get_current_conn_event(T_DCM_GET_CURRENT_CONN_REQ_MSG *current_conn_p);
T_DCM_RET dcm_process_unknown_event_in_idle(T_DCM_HDR* msg_p);
T_DCM_RET dcm_free_row(U8 current_row);
T_DCM_RET dcm_store_ipu_info(U8 row, T_BEARER_TYPE bearer_type, char *apn,
                             char *number, char *pdp_addr, U8 cid_used,
                             char *user, char *password, U32 dns1, U32 dns2,
                             U32 gateway);
T_DCM_RET dcm_clear_ipu_info(U8 row);
T_DCM_RET dcm_process_event_error_reception(T_DCM_HDR * msg_p);
T_DCM_RET dcm_process_cgatt_ans(T_DCM_HDR * msg_p, U8 row);
T_DCM_RET dcm_process_cgact_ans(T_DCM_HDR * msg_p, U8 row);
T_DCM_RET dcm_process_cgdeact_ans(T_DCM_HDR * msg_p, U8 row);
T_DCM_RET dcm_process_sat_dn_ans(T_DCM_HDR * msg_p, U8 row);
T_DCM_RET dcm_process_sat_h_ans(T_DCM_HDR * msg_p, U8 row);

#endif /*__DCM_UTILS_H__*/

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
|  Description :  DCM handle_msg function, which is called when DCM
|                 receives a new message by application or rAT_function.
+-----------------------------------------------------------------------------
*/


/***********************************  INCLUDES ********************************************/
#include "aci_all.h"
#include "dcm.h"
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"

/**************************** LOCAL VARIABLE DEFINITION  ***********************************/
/**************************** EXTERN VARIABLE DEFINITION  **********************************/
extern T_DCM_ENV_CTRL_BLK	*dcm_env_ctrl_blk_p;

/**************************** LOCAL FUCNTION DEFINITION  ***********************************/
/************************** EXTERN FUCNTION DEFINITION  ************************************/


/******************************************************************************************
*  Function      : dcm_handle_message
*  Description   : Called every time DCM is in WAITING state or
*                  When receiving message from Application or rAT_function
*  Parameter     :	 T_DCM_HDR
*                    -Pointer on the header of the message.
*  Return 	     : T_DCM_RET DCM
*                    -DCM Result : DCM_OK, DCM_NOT_READY.
*  History       : 0001 03/09/17 CJH Created
********************************************************************************************/
T_DCM_RET dcm_handle_message(T_DCM_HDR *msg_p)
{
  T_DCM_GET_CURRENT_CONN_REQ_MSG   *current_conn_info;

  TRACE_FUNCTION("DCM: dcm_handle_message()");
  dcm_display_message(msg_p->msg_id);
  dcm_dispay_state((U8)dcm_env_ctrl_blk_p->state[0],
                   (U8)dcm_env_ctrl_blk_p->substate[0]);

  if (msg_p != NULL)
  {
    /* get current conn req is always received regardless of current state..*/
    if(msg_p->msg_id == DCM_GET_CURRENT_CONN_REQ_MSG)
    {
      TRACE_EVENT("DCM: DCM_GET_CURRENT_CONN_REQ_MSG");
      current_conn_info = (T_DCM_GET_CURRENT_CONN_REQ_MSG*)msg_p;
      dcm_process_get_current_conn_event(current_conn_info);
      return DCM_OK;
    }
    switch (dcm_env_ctrl_blk_p->state[0])
    {
      case DCM_IDLE:
        TRACE_EVENT("DCM: DCM_IDLE") ;
        /* there is no current action */
        return dcm_idle(msg_p);

      case DCM_ACTIVATING_CONN:
        TRACE_EVENT("DCM: DCM_ACTIVATING_CONN") ;
        /* An IP User have asked the opening of a connection */
        return dcm_activating_conn(msg_p);

      case DCM_CONN_ACTIVATED:
        TRACE_EVENT("DCM: DCM_CONN_ACTIVATED") ;
        /* At leat, on connection is active */
        return dcm_conn_activated(msg_p);

      case DCM_CLOSING_CONN:
        TRACE_EVENT("DCM: DCM_CLOSING_CONN") ;
        /* An IP User have asked the closing of a connection */
        return dcm_closing_conn(msg_p);

      default:
        TRACE_EVENT("DCM: unknown state") ;
        return DCM_NOT_READY;
    }
  }
  else
  {
    TRACE_ERROR("DCM: NULL message") ;
    return DCM_INVALID_PARAMETER;
  }
}

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
|  Description :  This file contains Functions corresponding to each state 
                  of DCM state machine.
+------------------------------------------------------	-----------------------
*/


/***********************************  INCLUDES ********************************/
#include "aci_all.h"
#include "dcm.h"
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"

/**************************** LOCAL VARIABLE DEFINITION  **********************/
/**************************** EXTERN VARIABLE DEFINITION  *********************/
extern T_DCM_ENV_CTRL_BLK *dcm_env_ctrl_blk_p;

/**************************** LOCAL FUCNTION DEFINITION  **********************/
/************************** EXTERN FUCNTION DEFINITION  ***********************/



/*******************************************************************************
*  Function      :  dcm_idle
*  Description   :  Called when DCM state is DCM_IDLE
*                   and DCM has received a message.
*  Parameter     :  T_DCM_HDR
*                       -Pointer on the header of the message.
*  Return        :  T_DCM_RET
*                      -DCM Result : DCM_OK, DCM_NOT_READY, DCM_UNKNOWN_EVENT
*******************************************************************************/
T_DCM_RET dcm_idle (T_DCM_HDR * msg_p)
{
  TRACE_FUNCTION("DCM: dcm_idle()");

  if(msg_p->msg_id == DCM_OPEN_CONN_REQ_MSG)
  {
     return dcm_process_open_conn_event((T_DCM_OPEN_CONN_REQ_MSG*)msg_p);
  }
  else
  {
     return dcm_process_unknown_event_in_idle(msg_p);
  }
}


/*******************************************************************************
*  Function      :  dcm_activating_conn
*  Description   :  Called when DCM state is DCM_ACTIVATING_CONN
*                   and DCM has received a message.
*  Parameter     :  T_DCM_HDR
*                   -Pointer on the header of the message.
*  Return        :  T_DCM_RET
*                   -DCM Result : DCM_OK, DCM_NOT_READY, DCM_UNKNOWN_EVENT
*******************************************************************************/
T_DCM_RET dcm_activating_conn(T_DCM_HDR * msg_p)
{
  U8 current_row = dcm_env_ctrl_blk_p->current_row;

  TRACE_FUNCTION("DCM: dcm_activating_conn()");

  switch(dcm_env_ctrl_blk_p->substate[0])
  {
    case DCM_SUB_WAIT_CGATT_CNF :
      dcm_env_ctrl_blk_p->dcm_call_back = NULL;
      if(msg_p->msg_id == DCM_NEXT_CMD_READY_MSG OR
         msg_p->msg_id == DCM_NEXT_CMD_STOP_MSG)
      {
        return dcm_process_cgatt_ans(msg_p, current_row);
      }
      else
      {
        return dcm_process_unwaited_events_state_intermediate_conn(msg_p);
      }

    case DCM_SUB_WAIT_CGACT_CNF :
      dcm_env_ctrl_blk_p->dcm_call_back = NULL;
      if(msg_p->msg_id == DCM_NEXT_CMD_READY_MSG OR
         msg_p->msg_id == DCM_NEXT_CMD_STOP_MSG)
      {
        return dcm_process_cgact_ans(msg_p, current_row);
      }
      else
      {
        return dcm_process_unwaited_events_state_intermediate_conn(msg_p);
      }

    case DCM_SUB_WAIT_SATDN_CNF:
      dcm_env_ctrl_blk_p->dcm_call_back = NULL;
      if(msg_p->msg_id == DCM_NEXT_CMD_READY_MSG OR
         msg_p->msg_id == DCM_NEXT_CMD_STOP_MSG OR msg_p->msg_id == DCM_ERROR_IND_MSG)
      {
           return dcm_process_sat_dn_ans(msg_p, current_row);
      }
      else
      {
        return dcm_process_unwaited_events_state_intermediate_conn(msg_p);
      }

    default:
      return DCM_OK;
  }

}


/*******************************************************************************
*  Function      :  dcm_conn_activated
*  Description   :  if DCM receive message from application or PS,
*                   when DCM state is DCM_CONN_ACTIVATED
*                   this function is called and process reciving message.
*  Parameter     :  T_DCM_HDR
*                   -Pointer on the header of the message.
*  Return        :  T_DCM_RET
*                   -DCM Result : DCM_OK, DCM_NOT_READY, DCM_UNKNOWN_EVENT
*******************************************************************************/

T_DCM_RET dcm_conn_activated(T_DCM_HDR * msg_p)
{

  TRACE_FUNCTION("DCM: dcm_conn_activated()");

  switch(msg_p->msg_id)
  {
    case DCM_OPEN_CONN_REQ_MSG:
      return dcm_process_open_conn_event((T_DCM_OPEN_CONN_REQ_MSG*)msg_p);

    case DCM_CLOSE_CONN_REQ_MSG:
      return dcm_process_close_conn_event((T_DCM_CLOSE_CONN_REQ_MSG *)msg_p);

    case DCM_ERROR_IND_MSG :
      return dcm_process_event_error_reception(msg_p);

    default:
      /* Ignore event - Stay in the same state. */
      return DCM_UNKNOWN_EVENT;
  }
}


/*******************************************************************************
*  Function      :  dcm_closing_conn
*  Description   :  if DCM receive message from application or PS,
*                   when DCM state is DCM_CLOSING_CONN
*                   this function is called and process reciving message.
*  Parameter     :  T_DCM_HDR
*                   -Pointer on the header of the message.
*  Return        :  T_DCM_RET
*                   -DCM Result : DCM_OK, DCM_NOT_READY, DCM_UNKNOWN_EVENT
*******************************************************************************/
T_DCM_RET dcm_closing_conn(T_DCM_HDR * msg_p )
{
  U8 current_row = dcm_env_ctrl_blk_p->current_row;

  TRACE_FUNCTION("DCM: dcm_closing_conn()");

  switch(dcm_env_ctrl_blk_p->substate[0])
  {
    case DCM_SUB_WAIT_CGDEACT_CNF:
     // dcm_env_ctrl_blk_p->dcm_call_back = NULL; //pinghua DCM_OPEN_CLOSE patch 20080429
      if(msg_p->msg_id == DCM_NEXT_CMD_READY_MSG OR
         msg_p->msg_id == DCM_NEXT_CMD_STOP_MSG)
      {
        dcm_env_ctrl_blk_p->dcm_call_back = NULL; //pinghua DCM_OPEN_CLOSE patch 20080429
        return dcm_process_cgdeact_ans(msg_p,current_row);
      }
      else
      {
        return dcm_process_unwaited_events_state_intermediate_conn(msg_p);
      }
      /*lint -e527 suppress Warning -- Unreachable */
      /* break is removed ,as case is returning before break so it is not needed */
      /*lint +e527 */
    case DCM_SUB_WAIT_SATH_CNF:
      // dcm_env_ctrl_blk_p->dcm_call_back = NULL;  //pinghua DCM_OPEN_CLOSE patch 20080429
      if(msg_p->msg_id == DCM_NEXT_CMD_READY_MSG OR
         msg_p->msg_id == DCM_NEXT_CMD_STOP_MSG)
      {
        dcm_env_ctrl_blk_p->dcm_call_back = NULL;  //pinghua DCM_OPEN_CLOSE patch 20080429
        return dcm_process_sat_h_ans(msg_p,current_row);
      }
      else
      {
        return dcm_process_unwaited_events_state_intermediate_conn(msg_p);
      }
      /*lint -e527 suppress Warning -- Unreachable */
      /* break is removed ,as case is returning before break so it is not needed */
      /*lint +e527 */
    default:
      return DCM_UNKNOWN_EVENT;
  }

}


/******************************************************************************/
LOCAL char* state_to_string(U8 state)
{
  switch(state)
  {
    case DCM_OFF:             return "DCM_OFF";
    case DCM_IDLE:            return "DCM_IDLE";
    case DCM_ACTIVATING_CONN: return "DCM_ACTIVATING_CONN";
    case DCM_CONN_ACTIVATED:  return "DCM_CONN_ACTIVATED";
    case DCM_CLOSING_CONN:    return "DCM_CLOSING_CONN";
    default:                  return "Unkown DCM-state";
  }
}

LOCAL char* substate_to_string(U8 sub_state)
{
  switch(sub_state)
  {
    case DCM_SUB_NO_ACTION:        return "DCM_SUB_NO_ACTION";
    case DCM_SUB_WAIT_CGATT_CNF:   return "DCM_SUB_WAIT_CGATT_CNF";
    case DCM_SUB_WAIT_CGACT_CNF:   return "DCM_SUB_WAIT_CGACT_CNF";
    case DCM_SUB_WAIT_SATDN_CNF:   return "DCM_SUB_WAIT_SATDN_CNF";
    case DCM_SUB_WAIT_SATH_CNF:    return "DCM_SUB_WAIT_SATH_CNF";
    case DCM_SUB_WAIT_CGDEATT_CNF: return "DCM_SUB_WAIT_CGDEATT_CNF";
    case DCM_SUB_WAIT_CGDEACT_CNF: return "DCM_SUB_WAIT_CGDEACT_CNF";
    default:                       return "Unkown DCM-sub_state";
  }
}


void dcm_dispay_state(U8 state, U8 substate)
{
  TRACE_EVENT_P2("DCM: State= %s : Substate= %s", state_to_string(state),
                 substate_to_string(substate));
}

void dcm_dispay_sub_state(U8 substate_1, U8 substate_2)
{
  TRACE_EVENT_P2("DCM: Substate_1= %s : Substate_2= %s" ,
                 substate_to_string(substate_1),substate_to_string(substate_2));
}


/**
 * Function used to change of automaton state
 *
 * @param   state and associated substate
 * @return  DCM_OK or DCM errors
 */
T_DCM_RET dcm_new_state(U8 state, U8 substate)
{
  TRACE_FUNCTION("DCM: dcm_new_state()");
  dcm_dispay_state(state, substate);

  dcm_env_ctrl_blk_p->state[0] = (T_DCM_INTERNAL_STATE)state;
  dcm_env_ctrl_blk_p->substate[0] = (T_DCM_INTERNAL_SUBSTATE)substate;

  return DCM_OK;
}


/**
 * Function used to save of automaton state
 *
 * @param   none
 * @return  DCM_OK or DCM errors
 */
T_DCM_RET dcm_save_state(void)
{
  TRACE_FUNCTION("DCM: dcm_save_state()");

  dcm_env_ctrl_blk_p->state[1] = dcm_env_ctrl_blk_p->state[0];
  dcm_env_ctrl_blk_p->substate[1] = dcm_env_ctrl_blk_p->substate[0];

  return DCM_OK;
}


/**
 * Function used to restore the automaton state (before saving)
 *
 * @param   none
 * @return  DCM_OK or DCM errors
 */
T_DCM_RET dcm_restore_state(void)
{
  TRACE_FUNCTION("DCM: dcm_restore_state()");
  dcm_env_ctrl_blk_p->state[0] = dcm_env_ctrl_blk_p->state[1];
  dcm_env_ctrl_blk_p->substate[0] = dcm_env_ctrl_blk_p->substate[1];

  return DCM_OK;
}


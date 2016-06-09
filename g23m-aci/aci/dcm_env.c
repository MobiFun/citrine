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
|  Description :  Define dcm_functions to initialize dcm
|                 Define mfw callback function to be used in mfw_cb.c
+-----------------------------------------------------------------------------
*/


/***********************  INCLUDES ********************************************/
#include "aci_all.h"
#include "dcm.h"
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"


/*************** LOCAL VARIABLE DEFINITION  ***********************************/

/*************** EXTERN VARIABLE DEFINITION  **********************************/
/*
 * Pointer on the structure gathering all the global variables
 * used by DCM instance.
 */
T_DCM_ENV_CTRL_BLK *dcm_env_ctrl_blk_p;

/*************** LOCAL FUCNTION DEFINITION  ***********************************/
static T_DCM_ENV_CTRL_BLK  dcm_env_ctrl_blk;

/************* EXTERN FUCNTION DEFINITION  ************************************/


/*******************************************************************************
*   Function     :   dcm_start
*   Parameter    :   none
*   Description  :   dcm main state change to DCM_IDLE
*******************************************************************************/
LOCAL void dcm_start(void)
{
  TRACE_FUNCTION("DCM: dcm_start()");
  dcm_new_state(DCM_IDLE, DCM_SUB_NO_ACTION);
}



/*******************************************************************************
* Function     :   dcm_init
* Parameter    :   none
* Description  :   this function initialize DCM global variable and change dcm main state
*******************************************************************************/
void dcm_init (void)
{
  TRACE_FUNCTION("DCM: dcm_init()");

  dcm_env_ctrl_blk_p = &dcm_env_ctrl_blk;

  memset(dcm_env_ctrl_blk_p, 0x00, sizeof(T_DCM_ENV_CTRL_BLK));
  dcm_env_ctrl_blk_p->dcm_call_back = NULL;

  /* initalize main state , sub state */
  dcm_new_state(DCM_OFF, DCM_SUB_NO_ACTION);

  /*
   *  dcm start : this means that dcm main state change to DCM IDLE 
   *              from now, dcm will be operated if application send request 
   *              primitive to DCM
   */
  dcm_start();
}


LOCAL char* message_id_to_string(U8 msg_id)
{
  switch(msg_id)
  {
    case DCM_OPEN_CONN_REQ_MSG:        return "DCM_OPEN_CONN_REQ_MSG";
    case DCM_CLOSE_CONN_REQ_MSG:       return "DCM_CLOSE_CONN_REQ_MSG";
    case DCM_GET_CURRENT_CONN_REQ_MSG: return "DCM_GET_CURRENT_CONN_REQ_MSG";
    case DCM_OPEN_CONN_CNF_MSG:        return "DCM_OPEN_CONN_CNF_MSG";
    case DCM_CLOSE_CONN_CNF_MSG:       return "DCM_CLOSE_CONN_CNF_MSG";
    case DCM_GET_CURRENT_CONN_CNF_MSG: return "DCM_GET_CURRENT_CONN_CNF_MSG";
    case DCM_ERROR_IND_MSG:            return "DCM_ERROR_IND_MSG";
    case DCM_NEXT_CMD_READY_MSG:       return "DCM_NEXT_CMD_READY_MSG";
    case DCM_NEXT_CMD_STOP_MSG:        return "DCM_NEXT_CMD_STOP_MSG";
    default:                           return "Unkown Msg_Id";
  }
}

void dcm_display_message(U8 msg_id)
{
  TRACE_EVENT_P1("DCM: Message= %s",message_id_to_string(msg_id));
}



/*******************************************************************************
 Function  :   dcm_send_message
 Parameter :   UBYTE
 Description : this function is used in mfw to send result about AT command to DCM
********************************************************************************/
void dcm_send_message(T_DCM_STATUS_IND_MSG msg, T_DCM_INTERNAL_SUBSTATE sub_state)
{
    TRACE_FUNCTION("DCM: dcm_send_message()");

    /* No need to check state if an error occurs */
    if(msg.hdr.msg_id == DCM_ERROR_IND_MSG AND dcm_env_ctrl_blk_p->dcm_call_back)
    {
      /* FST: Is here a special handling necessary -> 
         see function dcm_mfw_callback(...) above()  */
      (void)dcm_env_ctrl_blk_p->dcm_call_back(&msg.hdr);

    }
    else if(msg.hdr.msg_id != DCM_ERROR_IND_MSG AND 
            dcm_env_ctrl_blk_p->dcm_call_back   AND
            dcm_env_ctrl_blk_p->substate[0] == sub_state)
    {
      (void)dcm_env_ctrl_blk_p->dcm_call_back(&msg.hdr);
    }
    
    /* This else handles the case if connection has been interrupted due to user
       or loss of network before connect cnf to app has been sent */
    else if(msg.hdr.msg_id == DCM_NEXT_CMD_STOP_MSG       AND 
            dcm_env_ctrl_blk_p->dcm_call_back             AND 
            (dcm_env_ctrl_blk_p->substate[0] == sub_state AND 
             (sub_state == DCM_SUB_WAIT_SATDN_CNF OR sub_state == DCM_SUB_WAIT_CGATT_CNF )  ))  //pinghua DCM_OPEN_CLOSE patch 20080429
    {
      (void)dcm_env_ctrl_blk_p->dcm_call_back(&msg.hdr);
    }
    else if(dcm_env_ctrl_blk_p->state[0] == DCM_IDLE AND 
            dcm_env_ctrl_blk_p->dcm_call_back)
    {
      (void)dcm_env_ctrl_blk_p->dcm_call_back(&msg.hdr);
    }
    else if(dcm_env_ctrl_blk_p->substate[0] != sub_state)
    {
      TRACE_ERROR("DCM: Error: mismatch in substate");
      dcm_dispay_sub_state((U8)sub_state,(U8)dcm_env_ctrl_blk_p->substate[0]);
    }
}

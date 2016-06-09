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
|  Description :  DCM instance is a state machine with several possible states.
|                 Based on the current state, DCM handle different kind of messages/events.
|                 States are listed in the order they should normally be sequenced.
+-----------------------------------------------------------------------------
*/

#ifndef __DCMSTATE_H__
#define __DCMSTATE_H__

/**
 * DCM states.
 *
 * DCM instance is a state machine with several possible states.
 * Based on the current state, DCM handle different kind of messages/events.
 * States are listed in the order they should normally be sequenced.
 *
 * See the Readme file for a description of the state machine.
 */
typedef enum
{
	DCM_OFF,
	DCM_IDLE,
	DCM_ACTIVATING_CONN,
	DCM_CONN_ACTIVATED,
	DCM_CLOSING_CONN
} T_DCM_INTERNAL_STATE;


/**
 * DCM substates.
 *
 * DCM instance is a state machine with several possible states.
 * Substates are included in some states.
 */

typedef enum
{
  DCM_SUB_NO_ACTION = 0,                            /* ALL */
  DCM_SUB_WAIT_CGATT_CNF,                           /* DCM_ACTIVATING_CONN */
  DCM_SUB_WAIT_CGACT_CNF,                           /* DCM_ACTIVATING_CONN */
  DCM_SUB_WAIT_SATDN_CNF,                           /* DCM_ACTIVATING_CONN */
  DCM_SUB_WAIT_SATH_CNF,                            /* DCM_CLOSING_CONN    */
  DCM_SUB_WAIT_CGDEATT_CNF,                         /* DCM_CLOSING_CONN    */
  DCM_SUB_WAIT_CGDEACT_CNF                          /* DCM_CLOSING_CONN    */
} T_DCM_INTERNAL_SUBSTATE;


T_DCM_RET dcm_idle (T_DCM_HDR * msg_p);
T_DCM_RET dcm_activating_conn(T_DCM_HDR * msg_p);
T_DCM_RET dcm_conn_activated(T_DCM_HDR * msg_p);
T_DCM_RET dcm_closing_conn(T_DCM_HDR * msg_p );

T_DCM_RET dcm_new_state(U8 state, U8 substate);
T_DCM_RET dcm_save_state(void);
T_DCM_RET dcm_restore_state(void);
void dcm_dispay_state(U8 state, U8 substate);
void dcm_dispay_sub_state(U8 substate_1, U8 substate_2);


#endif

/**
 * @file	xxx_state_i.h
 * 
 * Internal definitions for XXX state machine, 
 * i.e. the states and the functions corresponding to the states.
 *
 * The messages are declared in xxx_message.h, since they can 
 * be seen from outside.
 *
 * @author	Your name here (your_email_here)
 * @version 0.1
 */

/*
 * History:
 *
 *	Date       	Author					Modification
 *	-------------------------------------------------------------------
 *	//							Create.
 *
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef __XXX_STATE_I_H_
#define __XXX_STATE_I_H_


#include "rv/rv_general.h"


/**
 * XXX states.
 *
 * XXX instance is a state machine with several possible states.
 * Based on the current state, XXX handle different kind of messages/events.
 * States are listed in the order they should normally be sequenced.
 *
 * See the Readme file for a description of the state machine.
 */
typedef enum
{
	XXX_STATE_1,
	XXX_STATE_2
} T_XXX_INTERNAL_STATE;


/*
 * Optional but	recommanded, define a function for the processing
 * in each state, like:
 *   T_XXX_RETURN xxx_state_1					(T_RV_HDR * message_p);
 *   T_XXX_RETURN xxx_state_2					(T_RV_HDR * message_p);
 */

#endif /* __XXX_STATE_I_H_ */
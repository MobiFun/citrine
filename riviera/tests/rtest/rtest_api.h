/**
 * @file	rtest_api.h
 *
 * API Definition for the RTEST SWE.
 *
 * Please not the distinction that is made from where the API
 * functions can be called:
 * - from the test task only
 * - from RTEST only
 * - from both
 *
 * @author	Vincent Oberle (v-oberle@ti.com)
 * @version 0.1
 */

/*
 * History:
 *
 * 	Date       	Modification
 *  ------------------------------------
 *	11/21/2001	Create
 *	03/04/2002	Changed name to RTEST
 *
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef __RTEST_API_H_
#define __RTEST_API_H_


#include "rvm/rvm_gen.h"		/* Generic RVM types and functions. */

#include "tests/rv/rv_test_certif.h"
#include "tests/rv/rv_test_demo.h"
#include "tests/rv/rv_test_regr.h"
#include "tests/rv/rv_test_misc.h"


/**
 * Type for the test function pointers.
 */
typedef void (*T_RTEST_FUNC) (void);


/** FROM TEST TASK ONLY **/

/**
 * Starts a test.
 *
 * This function is called by the test task (ie the task to which
 * the main gives the hand). It BLOCKS until a RTEST_TEST_RESULT message
 * is received.
 *
 * Usage:
 * To be called from the test task only, not from RTEST!
 *
 * @param	test_fct	Test function.
 * @return	Result of the test.
 */
T_RV_TEST_RET rtest_start_test(T_RTEST_FUNC test_fct);


/** FROM TEST TASK AND RNET **/

/**
 * Sets up the RTEST SWE to forward the messages that are not 
 * RTEST messages to another handle_message function.
 *
 * Usage:
 * Can be called from outside and from inside RTEST.
 *
 * @param	hm	handle_message function to forward the messages to.
 */
void rtest_set_swe_handle_message (T_RVM_RETURN (* hm) (T_RV_HDR*));

/**
 * Sets up the RTEST SWE to forward the timers to another 
 * handle_timer function.
 *
 * Usage:
 * Can be called from outside and from inside RTEST.
 *
 * @param	ht	handle_timer function to forward the timers to.
 */
void rtest_set_swe_handle_timer (T_RVM_RETURN (* ht) (T_RV_HDR*));


/**
 * Trace information on the verdict (result) of a test.
 *
 * Usage:
 * Can be called from outside and from inside RTEST.
 *
 * @param	test_verdict	Result of the test.
 */
void rtest_trace_test_verdict (T_RV_TEST_RET test_verdict);


/** FROM RNET ONLY **/

/**
 * Sends the test result back to the test task.
 *
 * This function is to be used by a test when finished. It will
 * unblock the test_start_xxx functions.
 *
 * Usage:
 * To be called from RTEST only!
 *
 * @param	result	The result of the test.
 * @return	RV_MEMORY_ERR or RV_INTERNAL_ERR if there was a problem
 *			to send the message, RV_OK otherwise.
 */
T_RV_RET rtest_send_result (T_RV_TEST_RET result);


/**
 * Returns the address ID of the RTEST SWE.
 *
 * Usage:
 * To be called from RTEST only!
 *
 * @return	Address ID of RTEST.
 */
T_RVF_ADDR_ID rtest_get_addr_id (void);

/**
 * Returns the MB ID of the RTEST SWE.
 *
 * Usage:
 * To be called from RTEST only!
 *
 * @return	Memory bank ID of RTEST
 */
T_RVF_MB_ID rtest_get_mb_id (void);


/**
 * Blocks RTEST waiting for a message.
 *
 * If the parameter msg_id is not null, the function blocks until receiving
 * a message with the same ID, DISCARDING all other messages.
 * If the parameter msg_id is null, the function returns the first message received.
 *
 * Usage:
 * To be called from RTEST only!
 *
 * @param	msg_id	Waited message ID, null for any message.
 * @return	The received message.
 */
T_RV_HDR * rtest_wait_for_message (UINT32 msg_id);


#endif /*__RTEST_API_H_*/


/**
 * @file	rtest_msg_i.h
 *
 * Data structures:
 * 1) Used to send messages to the RTEST SWE,
 * 2) RTEST can receive.
 *
 * Only for internal use.
 *
 * @author	Vincent Oberle (v-oberle@ti.com)
 * @version 0.1
 */

/*
 * History:
 *
 *	Date       	Author					Modification
 *	-------------------------------------------------------------------
 *	11/21/2001	Vincent Oberle			Create
 *	03/04/2002	Vincent Oberle			Changed name to RTEST
 *	03/15/2002	Vincent Oberle			Put as a internal file, 
 *										reduced nb of messages
 *
 * (C) Copyright 2002 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef __RTEST_MSG_I_H_
#define __RTEST_MSG_I_H_


#include "rv/rv_general.h"

#include "tests/rtest/rtest_api.h"



/**
 * Macro used for tracing RTEST messages.
 */
#define RTEST_SEND_TRACE(string, trace_level) \
	rvf_send_trace (string, (sizeof(string) - 1), NULL_PARAM, trace_level, RTEST_USE_ID)

#define RTEST_SEND_TRACE_PARAM(string, param, trace_level) \
	rvf_send_trace (string, (sizeof(string) - 1), param, trace_level, RTEST_USE_ID)



/** 
 * The message offset must differ for each SWE in order to have 
 * unique msg_id in the system.
 */
#define RTEST_MESSAGE_OFFSET	 BUILD_MESSAGE_OFFSET(RTEST_USE_ID)



/***************
 * to RTEST SWE *
 ***************/

/**
 * @name RTEST_START_TEST
 *
 * Message to indicate to the Test SWE to proceed
 * to a test.
 */
/*@{*/
/** Message ID. */
#define RTEST_START_TEST (RTEST_MESSAGE_OFFSET | 0x001)

/** Message structure. */
typedef struct 
{
	/** Message header. */
	T_RV_HDR			hdr;

	/**
	 * The pointer on the test function.
	 */
	T_RTEST_FUNC test_fct;

} T_RTEST_START_TEST;
/*@}*/



/*****************
 * from RTEST SWE *
 *****************/

/**
 * @name RTEST_TEST_RESULT
 *
 * Message issued by the RTEST SWE to indicate the result of a test.
 */
/*@{*/
/** Message ID. */
#define RTEST_TEST_RESULT (RTEST_MESSAGE_OFFSET | 0x02)

/** Message structure. */
typedef struct 
{
	/** Message header. */
	T_RV_HDR		hdr;

	/** 
	 * Test result value. Possible values are:
	 * - TEST_PASSED:	No error occured
	 * - TEST_FAILED:	An error occured but continue test suite
	 * - TEST_IRRECOVERABLY_FAILED:	An error occured, stop test suite
	 */
	T_RV_TEST_RET	result;

} T_RTEST_TEST_RESULT;
/*@}*/

#endif /* __RTEST_MSG_I_H_ */

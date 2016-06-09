/**
 * @file	rv_test.h
 *
 * Definitions used by all tests in Riviera.
 *
 * @author	Cedric Baudelet
 * @author	Vincent Oberle
 */

/*
 * History:
 *
 *	Date       	Author					Modification
 *	-------------------------------------------------------------------
 *	10/02/00	Cedric Baudelet			Create.
 *	10/03/00	Cedric Baudelet			Add UPF4 value for TEST_TYPE parameter.
 *	10/05/00	David Lamy-Charrier		Add BTCTRL value for TEST_LEVEL	parameter.
 *	10/06/00	Cedric Baudelet			Add trace_test_verdict function's definition.
 *	10/12/00	Cedric Baudelet			Add possibility to set testing
 *										parameters dynamically using LCD and
 *										keypad features.
 *	11/16/00	Cedric Baudelet			Add TRACE_RVTEST traces macros.
 *	11/21/00	Cristian Livadiotti		Add xxx_return_path parameters.
 *	11/24/00	Cedric Baudelet			Update macros.
 *	11/30/00	Cedric Baudelet			Change EOL from (0xFFFF) to (0xFF).
 *	12/08/00	Cedric Baudelet			Add XXX_STATE definitions.
 *	12/14/00	Cedric Baudelet			Update DISPLAY_TEST_VERDICT_SCREEN macro.
 *	01/11/01	Cedric Baudelet			Add Flash File System (FFS) SWE.
 *	01/12/01	Cedric Baudelet			Add T_RV_TEST_INIT definition.
 *	01/16/01	Cedric Baudelet			Include 'rv_test_filter.h' file and
 *										define redirections needed to set test
 *										filter.
 *	03/29/01	Cedric Baudelet			- Updated LCD and keypad macros.
 *										- Replaced UPF4 by UPF.
 *  07/12/01    Stephanie Gerthoux		Add Audio test parameters using LCD.
 *  10/18/01	Christophe Favergeon	R2D added.
 *  12/18/01	Vincent Oberle			Changed DISPLAY_START_SCREEN,
 *		DISPLAY_TRANSITION_SCREEN and DISPLAY_TEST_VERDICT_SCREEN macros
 *		into functions.
 *		Replaced SET_STEP_PARAM macro with set_step_param function.
 *  03/08/02	Vincent Oberle			Changed SCROLL_LCD and START_APPLI into functions.
 *	03/08/02	Vincent Oberle			Passed all remaining macros into function.
 *										Comment and formating cleaning.
 *	11/28/02	Vincent Oberle			Deleted XXX lines, generated rv_test_inc.h used instead.
 *
 * (C) Copyright 2002 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef _RV_TEST_H_
#define _RV_TEST_H_

#include "rv/rv_general.h"
#include "rvf/rvf_api.h"
#include "rvm/rvm_gen.h"



#ifdef DEVICE_A
#define DISPLAY_DEVICE_STRING ("   Device A   ")
#endif
#ifdef DEVICE_B
#define DISPLAY_DEVICE_STRING ("   Device B   ")
#endif



/**
 * !!! IMPORTANT !!!
 *
 * END OF LIST -> This flag defines the end of the list containing tests to
 * perform (NON REGRESSION only). Lists TEST_TABLE[] (defined in rv_test_cfg.h
 * file) and test_list_all[] (defined in rv_test_regr.c file) MUST contain
 * this flag as their last element.
 */
#define EOL (0xFF)


/**
 * RV TEST -> GENERAL PARAMETERS
 *
 * TEST_PARAM_NOT_DEFINED is default value for a parameter not defined yet.
 */
#define TEST_PARAM_NOT_DEFINED (0)


/**
 * RV TEST -> TEST INIT PARAMETER
 *
 * TEST_INIT parameter is set in rv_test_cfg.h file.
 */
typedef UINT8 T_RV_TEST_INIT;

#define COMP (1)
#define EXEC (2)


/**
 *
 * RV TEST -> TEST TYPE PARAMETER
 *
 * TEST_TYPE parameter is set in rv_test_cfg.h file.
 */
typedef UINT8 T_RV_TEST_TYPE;

#define REGR   (1)
#define CERTIF (2)
#define DEMO   (3)
#define MISC   (4)
#define UPF    (5)


/**
 * RV TEST -> TEST LEVEL PARAMETER
 *
 * TEST_LEVEL parameter is set in rv_test_cfg.h file.
 * Values from rv_test_inc.h are used.
 */
typedef UINT8 T_RV_TEST_LEVEL;



/**
 * RV TEST -> TEST AUDIO PARAMETER
 *
 * TEST_AUDIO parameter is set in rv_test_cfg.h file.
 */
typedef UINT8 T_RV_TEST_AUDIO;

#define LIST    (1)
#define ONESHOT (2)


/**
 * RV TEST -> TEST AUDIO SERVICES PARAMETER
 *
 * TEST_AUDIO_ SERVICES parameter is set in rv_test_cfg.h file.
 */
typedef UINT8 T_RV_TEST_AUDIO_SERVICES;

#define KBEEP   (1)
#define TRITONE (2)
#define MELO    (3)


/**
 * RV TEST -> TEST NUMBER PARAMETER
 *
 * TEST_NUMBER parameter is set in rv_test_cfg.h file.
 */
typedef UINT16 T_RV_TEST_NBR;


/**
 * RV TEST -> ERROR TYPE PARAMETER
 */
typedef enum {
	NO_ERR				=  0, /* No error occurs										*/
	NO_EVENT_RECV		= -2, /* Received no event										*/
	BAD_EVENT_RECV		= -3, /* Received event not waited								*/
	EVENT_PARAM_ERROR	= -4, /* Received event -> parameter not waited					*/
	FUNCTION_ERROR		= -5, /* A function has been called and 0 ->  returned an error	*/
	DATA_ERROR			= -9, /* An error has been found in the RX or TX data			*/
	MEMORY_ERROR		= -10 /* A memory error occurs									*/
} T_RV_TEST_ERR_TYPE;


/**
 * RV TEST -> RETURN PARAMETER
 */
typedef enum {
	TEST_PASSED					=  0, /* No error occurs						*/
	TEST_FAILED					= -2, /* An error occurs -> continu test suite	*/
	TEST_IRRECOVERABLY_FAILED	= -3  /* An error occurs -> stop test suite		*/
} T_RV_TEST_RET;


/**
 * BT DEVICES
 *
 * Notes: bd_addr are set in rv_test_cfg.h file.
 */
/*
extern T_BD_ADDR bd_addr_loc; /* bd_addr of the LOCAL  device				
extern T_BD_ADDR bd_addr_rem; /* bd_addr of the REMOTE device			
*/

/**
 * RETURN PATHS
 *
 * Notes: Return paths are used to send messages to layers. These parameters are
 * initialized at the start of each test.
 */
extern T_RV_RETURN_PATH rv_test_g_l2cap_return_path;
extern T_RV_RETURN_PATH rv_test_g_sdp_return_path;
extern T_RV_RETURN_PATH rv_test_g_rfc_return_path;








/*************
 * FUNCTIONS *
 *************/

/**
 * This task is started from Application_Initialize and
 * calls the test main function depending on test type.
 */
void rv_test (UINT32 p);


/**
 * Gets memory banks for the tests.
 */
T_RV_RET rv_test_mb_init (T_RVF_MB_ID mbIds[]);


/**
 * Trace verdict of the	performed test depending on test_verdict parameter's
 */	
void trace_test_verdict (T_RV_TEST_RET test_verdict);


/**
 * Start the specified SWE.
 * Originaly the START_APPLI macros.
 *
 * @param	swe_use_id	SWE USE ID.
 */
void rv_test_start_swe (T_RVM_USE_ID swe_use_id);





/**
 * RV_TEST_TRACE_XXX(_PARAM) macros.
 *
 * Handle TRACE_RVTEST TRACE_TYPE traces, with or without parameter.
 */
/*@{*/
#define RV_TEST_TRACE(string, level) { \
	rvf_send_trace((string), sizeof(string) - 1, NULL_PARAM, level, TRACE_RVTEST); }

#define RV_TEST_TRACE_PARAM(string, param, level) { \
	rvf_send_trace((string), sizeof(string) - 1, param, level, TRACE_RVTEST); }

#define RV_TEST_TRACE_ERROR(string) { \
	rvf_send_trace((string), sizeof(string) - 1, NULL_PARAM, RV_TRACE_LEVEL_ERROR, TRACE_RVTEST); }

#define RV_TEST_TRACE_ERROR_PARAM(string, param) { \
	rvf_send_trace((string), sizeof(string) - 1, param, RV_TRACE_LEVEL_ERROR, TRACE_RVTEST); }

#define RV_TEST_TRACE_WARNING(string) { \
	rvf_send_trace((string), sizeof (string) - 1, NULL_PARAM, RV_TRACE_LEVEL_WARNING, TRACE_RVTEST); }

#define RV_TEST_TRACE_WARNING_PARAM(string, param) { \
	rvf_send_trace((string), sizeof(string) - 1, param, RV_TRACE_LEVEL_WARNING, TRACE_RVTEST); }

#define RV_TEST_TRACE_HIGH(string) { \
	rvf_send_trace((string), sizeof(string) - 1, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_HIGH, TRACE_RVTEST); }

#define RV_TEST_TRACE_HIGH_PARAM(string, param) { \
	rvf_send_trace((string), sizeof(string) - 1, param, RV_TRACE_LEVEL_DEBUG_HIGH, TRACE_RVTEST); }

#define RV_TEST_TRACE_MEDIUM(string) { \
	rvf_send_trace((string), sizeof(string) - 1, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_MEDIUM, TRACE_RVTEST); }

#define RV_TEST_TRACE_MEDIUM_PARAM(string, param) { \
	rvf_send_trace((string), sizeof(string) - 1, param, RV_TRACE_LEVEL_DEBUG_MEDIUM, TRACE_RVTEST); }

#define RV_TEST_TRACE_LOW(string) { \
	rvf_send_trace((string), sizeof(string) - 1, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, TRACE_RVTEST); }

#define RV_TEST_TRACE_LOW_PARAM(string, param) { \
	rvf_send_trace((string), sizeof(string) - 1, param, RV_TRACE_LEVEL_DEBUG_LOW, TRACE_RVTEST); }
/*@}*/



#endif /* _RV_TEST_H_ */

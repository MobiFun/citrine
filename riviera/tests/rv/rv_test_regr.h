/**
 * @file	rv_test_regr.h
 *
 * Definitions used for non-regression tests.
 *
 * @author	Cedric Baudelet
 * @author	Vincent Oberle
 */

/*
 * History:
 *
 *	Date       	Author					Modification
 *	-------------------------------------------------------------------
 *	09/29/00	Cedric Baudelet			Create.
 *	03/15/02	Vincent Oberle			Passed all remaining macros into function.
 *										Comment and formating cleaning.
 *
 * (C) Copyright 2002 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef _RV_TEST_REGR_H_
#define _RV_TEST_REGR_H_

#include "tests/rv/rv_test.h"

#ifdef __cplusplus
extern "C"
{
#endif


/*
 * DO NOT MODIFY THIS FILE!
 * Add your test in the corresponding .c file only.
 */


/**
 * Define the MAX number of tests which it will be possible to execute during
 * the regression.
 */
#define MAX_NB_OF_TEST_TO_PASS (200)

/** Non-Regression test error type */
typedef T_RV_TEST_ERR_TYPE T_RV_REGR_ERR_TYPE;

/** Non-Regression test return value type */
typedef T_RV_TEST_RET T_RV_REGR_RET;


/** Non-Regression test configuration parameter */
typedef enum {
	TEST_LIST					=  0, /* Allows to execute chosen tests				*/
	TEST_LIST_RAND				= -2, /* Allows to execute rand chosen tests		*/ 
	TEST_ALL					= -3, /* Allows to execute all tests available		*/
	TEST_ALL_RAND				= -4  /* Allows to execute rand all tests available	*/
} T_RV_REGR_TEST_CFG;


/** Non-Regression test list contents */
typedef UINT8 T_RV_REGR_TEST_LIST_NBR;
typedef UINT8 T_RV_REGR_TEST_LIST_CFG;

#define CONFIG_A (0) /* DEVICE_A = SOURCE & DEVICE_B = TARGET	*/
#define CONFIG_B (1) /* DEVICE_A = TARGET & DEVICE_B = SOURCE	*/

typedef struct {
    T_RV_REGR_TEST_LIST_NBR	list_nbr;	/* Number of the executed test			*/
	T_RV_REGR_TEST_LIST_CFG	list_cfg;	/* Configuration of the executed test	*/
} T_RV_REGR_TEST_LIST;


/** Non-Regression test total number of tests */
typedef UINT16 T_RV_REGR_TEST_TOT;

/** Non-Regression test nb of occurence of each test */
typedef UINT8 T_RV_REGR_TEST_OCC;

/** Non-Regression test initialization parameter */
#define RV_REGR_RAND_INIT (1)


#ifdef __cplusplus
}
#endif

#endif /* _RV_TEST_REGR_ */
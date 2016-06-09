/**
 * @file	rv_test_misc.h
 *
 * Definitions used for miscellaneous tests.
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

#ifndef _RV_TEST_MISC_H_
#define _RV_TEST_MISC_H_

#include "tests/rv/rv_test.h"

#ifdef __cplusplus
extern "C"
{
#endif


/*
 * DO NOT MODIFY THIS FILE!
 * Add your test in the corresponding .c file only.
 */


/** Misc test number type */
typedef T_RV_TEST_NBR T_RV_MISC_TEST_NBR;

/** Misc test return value type */
typedef T_RV_TEST_RET T_RV_MISC_RET;

/** Misc test error type */
typedef T_RV_TEST_ERR_TYPE T_RV_MISC_ERR_TYPE;


#ifdef __cplusplus
}
#endif

#endif /* _RV_TEST_MISC_H_ */

/**
 * @file	rtest_env.h
 *
 * Declarations of the Riviera Generic Functions 
 * for the Test SW Entity of Riviera.
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

#ifndef __RTEST_ENV_H_
#define __RTEST_ENV_H_


#include "rvm/rvm_gen.h"		/* Generic RVM types and functions. */
//#include "rvf/rvf_pool_size.h"	/* Stack & Memory Bank sizes definitions */
#include "tests/rtest/rtest_pool_size.h"	/* Stack & Memory Bank sizes definitions */


/**
 * @name Mem bank
 *
 * Memory bank size and watermark.
 */
/*@{*/
#define RTEST_MB_PRIM_SIZE				RTEST_MB1_SIZE
#define RTEST_MB_PRIM_WATERMARK			(RTEST_MB_PRIM_SIZE - 1072)
/*@}*/




/**
 * @name Generic functions
 *
 * Generic functions declarations needed for a type 4 SWE
 * (Self-made SWE).
 */
/*@{*/
T_RVM_RETURN rtest_get_info (T_RVM_INFO_SWE  *info_swe);

T_RVM_RETURN rtest_set_info (T_RVF_ADDR_ID	addr_id,
							T_RV_RETURN_PATH return_path[],
							T_RVF_MB_ID		bk_id_table[],
							T_RVM_CB_FUNC	call_back_error_ft);

T_RVM_RETURN rtest_init (void);

T_RVM_RETURN rtest_core (void);

T_RVM_RETURN rtest_stop (void);
T_RVM_RETURN rtest_kill (void);
/*@}*/


#endif /*__RTEST_ENV_H_*/
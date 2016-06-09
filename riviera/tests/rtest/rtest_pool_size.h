/**
 * @file	rtest_pool_size.h
 *
 * Declarations of:
 * - the memory bank sizes and their watermark
 * - the SWE stack size
 * - the pool size needed (generally the sum of memory bank and stack sizes)
 *
 * @author	Vincent Oberle
 * @version 0.1
 */

/*
 * History:
 *
 *	Date       	Author					Modification
 *	-------------------------------------------------------------------
 *	07/08/2003	Vincent Oberle			Extracted from rvf_pool_size.h
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef __RTEST_POOL_SIZE_H_
#define __RTEST_POOL_SIZE_H_


/*
 * Values used in rtest_env.h
 */
#define RTEST_STACK_SIZE (2048) //1024)
#define RTEST_MB1_SIZE   (300000) // + 550000) //190000
#define RTEST_POOL_SIZE  (RTEST_STACK_SIZE + RTEST_MB1_SIZE)


#endif /*__RTEST_POOL_SIZE_H_*/

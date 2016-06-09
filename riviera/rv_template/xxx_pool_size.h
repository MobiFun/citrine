/**
 * @file	xxx_pool_size.h
 *
 * Declarations of:
 * - the memory bank sizes and their watermark
 * - the SWE stack size
 * - the pool size needed (generally the sum of memory bank and stack sizes)
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
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef __XXX_POOL_SIZE_H_
#define __XXX_POOL_SIZE_H_


/*
 * Values used in xxx_env.h
 */
#define XXX_STACK_SIZE  (1024)
#define XXX_MB1_SIZE    (2048)
#define XXX_POOL_SIZE  (XXX_STACK_SIZE + XXX_MB1_SIZE)


#endif /*__XXX_POOL_SIZE_H_*/

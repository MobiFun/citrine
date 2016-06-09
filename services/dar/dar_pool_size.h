/**
 * @file	dar_pool_size.h
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
 *  Date       	Author			Modification
 *  -------------------------------------------------------------------
 *  07/08/2003	Vincent Oberle		Extracted from rvf_pool_size.h
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef __DAR_POOL_SIZE_H_
#define __DAR_POOL_SIZE_H_


#define DAR_STACK_SIZE (1024)
#define DAR_MB1_SIZE   (512)

#define DAR_POOL_SIZE  (DAR_STACK_SIZE + DAR_MB1_SIZE)


#endif /*__DAR_POOL_SIZE_H_*/

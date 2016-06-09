/**
 * @file	ti_prf_pool_size.h
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

#ifndef __TI_PRF_POOL_SIZE_H_
#define __TI_PRF_POOL_SIZE_H_


#ifndef _WINDOWS
  #include "debug.cfg"
#endif


/*
 * Values used in ti_profiler_env.h
 */
#define TI_PRF_STACK_SIZE (1000)

#if (TI_NUC_MONITOR == 1)
  #define TI_PRF_MB1_SIZE (1080000)
#else
  #define TI_PRF_MB1_SIZE (400)
#endif

#define TI_PRF_POOL_SIZE  (TI_PRF_STACK_SIZE + TI_PRF_MB1_SIZE)


#endif /*__TI_PRF_POOL_SIZE_H_*/

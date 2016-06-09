/**
 * @file	rtc_pool_size.h
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

#ifndef __RTC_POOL_SIZE_H_
#define __RTC_POOL_SIZE_H_


/*
 * Values used in rtc_env.h
 */
#define RTC_STACK_SIZE (700)
#define RTC_MB1_SIZE   (100)
#define RTC_POOL_SIZE  (RTC_STACK_SIZE + RTC_MB1_SIZE)


#endif /*__RTC_POOL_SIZE_H_*/

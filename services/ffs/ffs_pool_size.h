/**
 * @file	ffs_pool_size.h
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
 *  09/03/2004  Tommy Jensen            Split pool size into several defines
 *
 * This code is in the public domain per the order of the
 * Supreme Revolutionary Command.
 */

#ifndef __FFS_POOL_SIZE_H_
#define __FFS_POOL_SIZE_H_

#include "../../include/config.h"
#include "../../riviera/rv/rv_defined_swe.h"

/*
 * Values used in ffs_env.h
 */
#define FFS_STACK_SIZE   (1024)
#define FFS_MAILBUF_SIZE (1024)      // Default: Max 20 pending FFS mails.
#define FFS_TESTBUF_SIZE (0)         // Must be set to zero

#if (!GSMLITE)
#ifdef RVM_MSFE_SWE
   #define FFS_STREAMBUF_SIZE (40960)
#else
   #define FFS_STREAMBUF_SIZE (8192)
#endif // RVM_MSFE_SWE
#else 
   #define FFS_STREAMBUF_SIZE (4096)
#endif // GSMLITE

#define FFS_MB1_SIZE (FFS_STREAMBUF_SIZE + FFS_MAILBUF_SIZE + FFS_TESTBUF_SIZE)
#define FFS_POOL_SIZE  (FFS_STACK_SIZE + FFS_MB1_SIZE)


#endif /*__FFS_POOL_SIZE_H_*/

/**
 * @file	rvm_pool_size.h
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
 *	17/09/2003	Gary Totney			    Calibrated MB3 for 30 tasks
 *
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef __RVM_POOL_SIZE_H_
#define __RVM_POOL_SIZE_H_


#if 0 //#ifndef _WINDOWS
  #include "rv.cfg"	     // TEST
  #include "debug.cfg" 	 // TI_PROFILER, TI_NUC_MONITOR
  #include "board.cfg"    // BOARD
  #include "swconfig.cfg" // GSMLITE
#endif

/*
 * RVM SWE; This SWE is of course always enabled.
 *
 * Values used in rvm_i.h
 */

#define RVF_RVM_STACK_SIZE (1500)

#if (defined BLUETOOTH) || (TEST==1) || (TI_PROFILER == 1) || (TI_NUC_MONITOR == 1)
  #define RVF_RVM_MB1_SIZE (13000)
#else
   #if (GSMLITE)
      #define RVF_RVM_MB1_SIZE  (1200)
   #else
      #define RVF_RVM_MB1_SIZE  (6000)
   #endif
#endif
#define RVF_RVM_MB2_SIZE    (700)

#if (GSMLITE)
   #define RVF_RVM_MB3_SIZE	(3200)
   #define RVF_RVM_MB4_SIZE	(0)
   #define RVF_RVM_MB5_SIZE	(0)
   #define RVF_RVM_COMPENSATOR_POOL_SIZE	(1000)
#else
   #define RVF_RVM_MB3_SIZE	(9000)
   #define RVF_RVM_MB4_SIZE	(3000)
   #define RVF_RVM_MB5_SIZE	(3000)
   #define RVF_RVM_COMPENSATOR_POOL_SIZE	(12500+3000)
#endif


#define RVF_RVM_POOL_SIZE  (RVF_RVM_STACK_SIZE + RVF_RVM_MB1_SIZE + \
                            RVF_RVM_MB2_SIZE + RVF_RVM_MB3_SIZE + \
                            RVF_RVM_MB4_SIZE + RVF_RVM_MB5_SIZE +\
                            RVF_RVM_COMPENSATOR_POOL_SIZE )



#endif /*__RVM_POOL_SIZE_H_*/

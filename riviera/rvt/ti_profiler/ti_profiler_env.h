/*******************************************************************************
 *
 * ti_profiler_env.c
 *
 * This module interfaces the environment and contains all start/init/stop...
 * functions of the TI profiler.
 *
 * (C) Texas Instruments, all rights reserved
 *
 * Version number	: 0.1
 *
 * History			: 0.1 (7/30/2001) - Created
 *
 * Date             : 7/30/2001
 *
 * Author           : Philippe Martinez, 
 *
 ******************************************************************************/

#ifndef __TI_PROFILER_ENV_H__
#define __TI_PROFILER_ENV_H__

#include "rvm/rvm_gen.h"

#include "ti_prf_pool_size.h"	/* Stack & Memory Bank sizes definitions */

#define TI_PROFILER_TASK_STACK_SIZE TI_PRF_STACK_SIZE

#define TI_PROFILER_MB_SIZE         TI_PRF_MB1_SIZE
#define TI_PROFILER_MB_WATERMARK    TI_PROFILER_MB_SIZE

#define TI_PROFILER_MAILBOX         RVF_TASK_MBOX_0
#define TI_PROFILER_TASK_PRIORITY   255

T_RVM_RETURN
ti_prf_get_info (T_RVM_INFO_SWE *p_info_swe);


T_RVM_RETURN
ti_prf_set_info(T_RVF_ADDR_ID addr_id,
				T_RV_RETURN_PATH	return_path[],
				T_RVF_MB_ID bk_id[],
				T_RVM_RETURN (*rvm_error_ft)(T_RVM_NAME swe_name,
								 T_RVM_RETURN error_cause,
								 T_RVM_ERROR_TYPE error_type,
								 T_RVM_STRING error_msg));

T_RVM_RETURN
ti_prf_env_init (void);

T_RVM_RETURN
ti_prf_env_stop (void);

T_RVM_RETURN
ti_prf_kill (void);


#endif /* __TI_PROFILER_ENV_H__ */
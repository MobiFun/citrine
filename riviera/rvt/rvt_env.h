/*******************************************************************************
 *
 * rvt_env.c
 *
 * This module interfaces the environment and contains all start/init/stop...
 * functions of the trace module.
 *
 * (C) Texas Instruments, all rights reserved
 *
 * Version number	: 0.1
 *
 * History			: 0.1 (7/4/2000) - Created
 *
 * Date             : 7/4/2000
 *
 * Author           : Cristian Livadiotti, c-livadiotti@ti.com
 *
 ******************************************************************************/

#ifndef __RVT_ENV_H__
#define __RVT_ENV_H__

#include "../rvm/rvm_gen.h"


typedef enum { RVT_NOT_STARTED,	RVT_STARTED	} T_RVT_STATE;

extern T_RVF_MB_ID		rvt_mb_id;

extern T_RVT_STATE		rvt_module_state;

T_RVM_RETURN
rvt_get_info (T_RVM_INFO_SWE *p_info_swe);

T_RVM_RETURN
rvt_set_info (T_RVF_ADDR_ID		addr_id,
			  T_RV_RETURN_PATH	return_path[],
			  T_RVF_MB_ID		bk_id[],
			  T_RVM_CB_FUNC		rvm_error_ft);

T_RVM_RETURN
rvt_init (void);

T_RVM_RETURN
rvt_task_core (void);

T_RVM_RETURN
rvt_stop (void);

T_RVM_RETURN
rvt_kill (void);


#endif /* __RVT_ENV_H__ */

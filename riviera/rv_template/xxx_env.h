/**
 * @file	xxx_env.h
 *
 * Declarations of the Riviera Generic Functions 
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
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef __XXX_ENV_H_
#define __XXX_ENV_H_


#include "rvm/rvm_gen.h"		/* Generic RVM types and functions. */
#include "rvf/rvf_pool_size.h"	/* Stack & Memory Bank sizes definitions */
#include "xxx/xxx_pool_size.h"	/* Stack & Memory Bank sizes definitions */

/** 
 * Wished priority of the host task for the SWE.
 *
 * During development, put the hardcoded value here.
 * After integration, the value should be in rvm_priorities.h
 */
#ifdef RVM_XXX_TASK_PRIORITY
#define XXX_TASK_PRIORITY               RVM_XXX_TASK_PRIORITY	
#else
#define XXX_TASK_PRIORITY               80	
#endif


/**
 * @name Generic functions
 *
 * Generic functions declarations needed for a Type 2 SWE
 * (Group Member SWE).
 */
/*@{*/
T_RVM_RETURN xxx_get_info (T_RVM_INFO_SWE  *info_swe);

T_RVM_RETURN xxx_set_info ( T_RVF_ADDR_ID	addr_id,
							T_RV_RETURN_PATH return_path[],
							T_RVF_MB_ID		bk_id_table[],
							T_RVM_CB_FUNC	call_back_error_ft);

T_RVM_RETURN xxx_init (void);

T_RVM_RETURN xxx_kill (void);

/* Type 1 specific generic functions */
T_RVM_RETURN xxx_start (void);
/* End of specific */
/* Type 2 specific generic functions */
T_RVM_RETURN xxx_start (void);
T_RVM_RETURN xxx_stop (T_RV_HDR * msg);
T_RVM_RETURN xxx_handle_message (T_RV_HDR * msg);
T_RVM_RETURN xxx_handle_timer (T_RV_HDR * msg);
/* End of specific */
/* Type 3 specific generic functions */
T_RVM_RETURN xxx_start (void);
T_RVM_RETURN xxx_stop (T_RV_HDR * msg);
T_RVM_RETURN xxx_handle_message (T_RV_HDR * msg);
T_RVM_RETURN xxx_handle_timer (T_RV_HDR * msg);
/* End of specific */
/* Type 4 specific generic functions */
T_RVM_RETURN xxx_core (void);
/* End of specific */

/*@}*/

#endif /*__XXX_ENV_H_*/

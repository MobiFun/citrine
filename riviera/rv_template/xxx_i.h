/**
 * @file	xxx_i.h
 *
 * Internal definitions for XXX.
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

#ifndef __XXX_INST_I_H_
#define __XXX_INST_I_H_


#include "rv/rv_general.h"
#include "rvf/rvf_api.h"
#include "rvm/rvm_gen.h"
#include "rvm/rvm_use_id_list.h"
#include "rvm/rvm_ext_use_id_list.h"

#include "xxx/xxx_cfg.h"
#include "xxx/xxx_api.h"
#include "xxx/xxx_message.h"

#include "xxx/xxx_state_i.h"


/** Macro used for tracing XXX messages. */
#define XXX_SEND_TRACE(string, trace_level) \
	rvf_send_trace (string, (sizeof(string) - 1), NULL_PARAM, trace_level, XXX_USE_ID)



/**
 * The Control Block buffer of XXX, which gathers all 'Global variables'
 * used by XXX instance.
 *
 * A structure should gathers all the 'global variables' of XXX instance.
 * Indeed, global variable must not be defined in order to avoid using static memory.
 * A T_XXX_ENV_CTRL_BLK buffer is allocated when creating XXX instance and is 
 * then always refered by XXX instance when access to 'global variable' 
 * is necessary.
 */
typedef struct
{
	/** Store the current state of the XXX instance */
	T_XXX_INTERNAL_STATE state;

	/** Pointer to the error function */
	T_RVM_RETURN (*error_ft)(T_RVM_NAME swe_name,	
							 T_RVM_RETURN error_cause,
							 T_RVM_ERROR_TYPE error_type,
							 T_RVM_STRING error_msg);
	/** Mem bank id. */
	T_RVF_MB_ID prim_mb_id;

	T_RVF_ADDR_ID	addr_id;

} T_XXX_ENV_CTRL_BLK;


/** External ref "global variables" structure. */
extern T_XXX_ENV_CTRL_BLK	*xxx_env_ctrl_blk_p;


#endif /* __XXX_INST_I_H_ */
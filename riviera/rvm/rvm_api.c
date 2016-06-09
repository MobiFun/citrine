/**
 *                                                                          
 *  @file	rvm_api.c                                              
 *                                                                          
 *  This file contains interface functions.
 *
 * @author	Cristian Livadiotti (c-livadiotti@ti.com)
 * @version	0.2
 *
 */

/*
 * Revision History:																			
 *
 * 06/04/2000	Cristian Livadiotti		Create.
 * 10/22/2001	David Lamy-Charrier		Update for new Riviera 1.6.
 *																			
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */


#include "../../include/config.h"

#include "../rvf/rvf_env.h"

#include "rvm_gen.h"
#include "rvm_api.h"
#include "rvm_i.h"
#include "rvm_use_id_list.h"

#include <string.h>

extern T_RVM_KNOWN_SWE * rvm_swe_array;


/*******************************************************************************
** Function         rvm_start_environment
**
** Description      Start the RV manager. 
**					
*******************************************************************************/
T_RVM_RETURN	rvm_start_environment()
{	T_RVF_BUFFER * rvm_stack;

	/* initialize the RVM */
	if ( rvm_init() == FALSE)
	{	return RVM_INTERNAL_ERR;
	}

	/* start the RV manager task */

	if ( rvf_get_buf( rvm_stack_mem_bank, RVM_STACK_SIZE, &rvm_stack) == RVF_RED )
	{	return RVM_MEMORY_ERR;
	}

	if ( rvf_create_task( rvm_task, RVM_TASK_ID, "RVM", rvm_stack, RVM_STACK_SIZE, RVM_PRIORITY, RVM_TASK, DEFAULT_TIME_SLICING, RUNNING) 
		!= RVF_OK )
	{	return RVM_INTERNAL_ERR;
	}

	return RVM_OK;
}


/*******************************************************************************
**
** Function         rvm_start_swe
**
** Description      Called by an application to start the specified SWE
**
** Parameters:		USE_ID of the SWE to start.
**					return path for asynchronous response
**
** Returns          T_RVM_RETURN:	RVM_OK if everything is ok,
**									RVM_INVALID_PARAMETER if the SWE USE_ID is unknown
**									RVM_NOT_READY if the get_info function has not been specified in the database
**												  or the SWE has been already started.
**									RVM_MEMORY_ERR if there is not enough memory in the RVM memory bank.
**									RVM_INTERNAL_ERR if the RVM task has not been created.
**
*******************************************************************************/
T_RVM_RETURN	rvm_start_swe (T_RVM_USE_ID swe_use_id, T_RV_RETURN_PATH return_path)
{	
	T_RVM_MSG		*msg;
	T_RVM_RETURN	rvm_status;
	UINT8			num_swe;
	
	/* Check Application is "startable"	*/
	if ((rvm_status = rvm_check_application (swe_use_id, &num_swe, RVM_START_APPLI)) != RVM_OK)
	{
		rvf_send_trace("RVM_task: rvm_start_swe() this appli cannot be started", 54, (UINT32)rvm_status, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );
		return rvm_status;
	}
	
	/* build a msg  */
	if (rvf_get_buf( rvm_mem_bank, sizeof(T_RVM_MSG), (void **)&msg) == RVF_RED )
	{
		rvf_send_trace("RVM_task: No memory", 19, NULL_PARAM, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );
		return RVM_MEMORY_ERR;
	}

	msg->header.msg_id			= RVM_START_APPLI;
	msg->header.src_addr_id		= return_path.addr_id; 
//	msg->header.callback_func	= return_path.callback_func; 
	msg->rp.callback_func		= return_path.callback_func;
	msg->swe_num				= num_swe;
	
	rvf_send_trace("RVM: SWE START REQUEST", 22, rvm_swe_array[num_swe].swe_use_id, RV_TRACE_LEVEL_DEBUG_LOW, RVM_USE_ID );

	/* and send it to the RVM task to be treated by the RVM task */
	/* Note: task_id is used as the destination addr_id. This is only */
	/* true in the case of mailbox zero. Then task_id==addr_id        */
	if ( rvf_send_msg( RVM_TASK_ID, msg) != RVF_OK)
	{	rvf_free_buf( msg);
		return RVM_INTERNAL_ERR;
	}

	return RVM_OK;
}

/*******************************************************************************
**
** Function         rvm_stop_swe
**
** Description      Called by an application to stop the specified SWE.
**
** Parameters:		USE_ID of the SWE to start.
**					return path for asynchronous response
**
** Returns          T_RVM_RETURN:	RVM_OK if everything is ok,
**									RVM_INVALID_PARAMETER if the USE_ID is unknown
**									RVM_NOT_READY if the get_info function has not been specified in the database
**												  or the SWE is not running.
**									RVM_MEMORY_ERR if there is not enough memory in the RVM memory bank.
**									RVM_INTERNAL_ERR if the RVM task has not been created.
**
** RV2 ADDITIONAL NOTES: This now creates a RVM_STOP_MSG instead of a RVM_STOP_APPLI message.
** both are supported in the RVM-FSM, hence the legacy concept may be reverted to easily
*******************************************************************************/
T_RVM_RETURN	rvm_stop_swe (T_RVM_USE_ID swe_use_id, T_RV_RETURN_PATH return_path)
{	
	T_RVM_STOP_MSG	*msg;
	T_RVM_RETURN	rvm_status;
	UINT8			num_swe;

	/*
	** Check Application is "stopable"
	*/
	if ((rvm_status = rvm_check_application (swe_use_id, &num_swe, RVM_STOP_APPLI)) != RVM_OK)
		return rvm_status;
	
	/* build a msg  */
	if (rvf_get_buf( rvm_mem_bank, sizeof(T_RVM_STOP_MSG), (void **)&msg) == RVF_RED )
	{	return RVM_MEMORY_ERR;
	}

	msg->header.msg_id			= RVM_STOP_MSG;
	msg->header.src_addr_id		= return_path.addr_id; 
//	msg->header.callback_func	= return_path.callback_func; 
	msg->rp.callback_func		= return_path.callback_func; 
	msg->swe_num				= num_swe;
	msg->status					= SWE_RUNNING;
	
	/* and send it to the rve mailbox to be treated by the RVM task */
	if ( rvf_send_msg( RVM_TASK_ID, msg) != RVF_OK)
	{	rvf_free_buf( msg);
		return RVM_INTERNAL_ERR;
	}

	return RVM_OK;
}

// NOTE: this may be used to enable the terminator to uncondionally 
// kill the Entity. Mainly, spoofing RVM.
T_RVM_RETURN	rvm_kill_immediate (T_RVM_USE_ID swe_use_id, T_RV_RETURN_PATH return_path){	
	T_RVM_STOP_MSG	*msg;
	T_RVM_RETURN	rvm_status;
	UINT8			num_swe;

	/*
	** Check Application is "stopable"
	*/
	if ((rvm_status = rvm_check_application (swe_use_id, &num_swe, RVM_STOP_APPLI)) != RVM_OK)
		return rvm_status;
	
	/* build a msg  */
	if (rvf_get_buf( rvm_mem_bank, sizeof(T_RVM_STOP_MSG), (void **)&msg) == RVF_RED )
	{	return RVM_MEMORY_ERR;
	}

	msg->header.msg_id			= RVM_STOP_MSG;
	msg->header.src_addr_id		= return_path.addr_id; 
//	msg->header.callback_func	= return_path.callback_func; 
	msg->rp.callback_func		= return_path.callback_func; 
	msg->swe_num				= num_swe;
	msg->status					= SWE_STOPPING;
	
	/* and send it to the rve mailbox to be treated by the RVM task */
	if ( rvf_send_msg( RVM_TASK_ID, msg) != RVF_OK)
	{	rvf_free_buf( msg);
		return RVM_INTERNAL_ERR;
	}

	return RVM_OK;
}

T_RVM_RETURN rvm_swe_stopped(T_RV_HDR* p_msg) {
	T_RVM_STOP_MSG	*msg=(T_RVM_STOP_MSG*)p_msg;

	msg->status					= SWE_STOPPING;
	
	if ( rvf_send_msg( RVM_TASK_ID, msg) != RVF_OK)	{
		rvf_free_buf( msg);
		return RVM_INTERNAL_ERR;
	}
	return RVM_OK;
}

/*******************************************************************************
**
** Function         rvm_snd_msg_to_upper
**
** Description      Called during processing to report to MMI the result of an
**                  action.
**
** Parameters:		name of the application to start.
**					return path for asynchronous response
**
** Returns          T_RVM_RETURN:	RVM_OK if everything is ok,
**									RVM_INVALID_PARAMETER if the appli name is unknown
**									RVM_NOT_READY if the get_info function has not been specified in the database
**												  or the application has been already started.
**									RVM_MEMORY_ERR if there is not enough memory in the RVM memory bank.
**									RVM_INTERNAL_ERR if the RVM task has not been created.
**
*******************************************************************************/
T_RVM_RETURN	rvm_snd_msg_to_upper	(T_RVM_APPLI_ACTION action,
										 T_RVM_RETURN result,
										 UINT8 swe_num,
										 T_RV_RETURN_PATH return_path)
{	
	T_RVM_APPLI_RESULT		*msg;


	/* build a msg  */
	if (rvf_get_buf( rvm_mem_bank, sizeof(T_RVM_APPLI_RESULT), (void **)&msg) == RVF_RED )
	{	return RVM_MEMORY_ERR;
	}

	msg->header.msg_id			= RVM_EVT_TO_APPLI;
	msg->header.src_addr_id		= RVM_TASK_ID;
//	msg->header.callback_func	= return_path.callback_func; 
	msg->rp.callback_func		= return_path.callback_func; 
	msg->result					= result;
	msg->action					= action;
	msg->swe_index				= swe_num;

	memcpy(msg->swe_name, rvm_swe_array[swe_num].swe_name, RVM_NAME_MAX_LEN);

	/* and send it to the rve mailbox to be treated by the RVM task */
	if (return_path.callback_func)
	{
		return_path.callback_func ((void*) msg);
	}
	else
	{
		if ( rvf_send_msg(return_path.addr_id, (void*)msg) != RVF_OK)
		{	rvf_free_buf( msg);
			return RVM_INTERNAL_ERR;
		}
	}

	return RVM_OK;
}



/*******************************************************************************
**
** Function         rvm_get_swe_information
**
** Description      Called by a SWE to know information about another SWE.
**
** Parameters In:		name of the swe we want to get information about.
**
** Parameters Out:		State of the SWE.
**
** Returns          T_RVM_RETURN:	RVM_OK if everything is ok,
**									RVM_INVALID_PARAMETER if the SWE use_id is unknown
**									RVM_NOT_READY if the 
**									RVM_MEMORY_ERR if there is not enough memory in the RVM memory bank.
**									RVM_INTERNAL_ERR if the RVM task has not been created.
**
*******************************************************************************/
T_RVM_RETURN	rvm_get_swe_information	(T_RVM_USE_ID swe_id, 
										 T_RV_RETURN_PATH * return_path)
{
	UINT8 swe_index;

	if (rvm_get_swe_index(&swe_index, swe_id) != RVM_OK)
	{

		RVM_TRACE_WARNING_PARAM("RVM API: Get SWE Information of an unknown SWE, use_id:", swe_id);
		return RVM_INVALID_PARAMETER;
	}

	if (return_path != NULL)
	{
		(*return_path).callback_func	= rvm_swe_array[swe_index].swe_return_path.callback_func;
		(*return_path).addr_id			= rvm_swe_array[swe_index].swe_addr_id;
	}

	return RVM_OK;
}

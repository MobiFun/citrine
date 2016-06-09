/**
 *                                                                          
 *  @file	rvm_swe_db.c                                              
 *                                                                          
 *  This file contains the functions related to the SW Entities
 *	database management.
 *
 * @author	David Lamy-Charrier (d-lamy@ti.com)	
 * @version	0.2
 *
 */

/*
 * Revision History:																			
 *
 * 01/19/2000	David Lamy-Charrier		Create.
 * 10/22/2001	David Lamy-Charrier		Update for new Riviera 1.6.
 *																			
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */

#include "../rvf/rvf_env.h"

#include "rvm_gen.h"
#include "rvm_api.h"
#include "rvm_i.h"
#include "rvm_use_id_list.h"

#include <string.h>

extern T_RVM_CONST_SWE_INFO RVM_SWE_GET_INFO_ARRAY[];

extern const T_RVM_USE_ID * RVM_TYPE2_SWE_GROUPS[];


/* id of the main rvm memory bank */
T_RVF_MB_ID rvm_mem_bank=RVF_INVALID_MB_ID;
T_RVF_MB_ID rvm_sys_mem_bank=RVF_INVALID_MB_ID;
T_RVF_MB_ID rvm_timer_mem_bank=RVF_INVALID_MB_ID;
T_RVF_MB_ID rvm_tm_notify_mem_bank=RVF_INVALID_MB_ID;
T_RVF_MB_ID rvm_stack_mem_bank=RVF_INVALID_MB_ID;


/* database of all SW Entities, their name, their get_info function and their links */
T_RVM_KNOWN_SWE	* rvm_swe_array;

T_RVM_GROUP_INFO * rvm_group_array;

/*
** Used task id array
** This array allows the RVM to allocate dynamically task ids to SWEs
** The task ids are allocated during creation.
*/
/*BOOLEAN rvm_allocated_task_id [MAX_RVF_TASKS];  */

/*
** Number of SW Entities known on the system
*/
UINT8 rvm_swe_number = 0;


/*******************************************************************************
** Function         rvm_init_swe_db
**
** Description      Internal function called once at the beginning which 
**					initializes the SWE database from information gathered 
**					in a const array (defined in rvm_swe.c).
**
*******************************************************************************/
T_RVM_RETURN	rvm_init_swe_db ()
{
	UINT8 swe_cpt=0, i=0, j=0, group_cpt = 0;
	T_RVM_INFO_SWE swe_info;
	char rvm_trace_name[RVM_NAME_MAX_LEN+20] = "SWE Name: ";
	
	/* Initialize the allocated task id array */
/*	for (i=0; i<MAX_RVF_TASKS; i++)
	{
		rvm_allocated_task_id[i] = FALSE;
	}
*/
	/* None task ID allocated at startup except:
	 *   - RVM_TASK_ID,
	 *   - RVTEST_MENU_TASK_ID,
	 *   - DUMMY_TASK_ID.
	 *
	 * Refer to rv_general.h.
	 *
	 * TO DO: Remove the last two IDs as soon as managed as SWEs. */
/*	rvm_allocated_task_id[RVM_TASK_ID] = TRUE;   
	
	if ( RVTEST_MENU_TASK_ID < MAX_RVF_TASKS) 
	{	rvm_allocated_task_id[RVTEST_MENU_TASK_ID] = TRUE;
	}
	
	if ( IDLE_TASK_ID < MAX_RVF_TASKS) 
	{	rvm_allocated_task_id[IDLE_TASK_ID] = TRUE;
	}
*/
	
	/* Get the number of known SWEs */
	while ( ( RVM_SWE_GET_INFO_ARRAY[swe_cpt].get_info_func) != NULL)
	{
		swe_cpt++;
	}
	rvm_swe_number = swe_cpt;
	RVM_TRACE_DEBUG_HIGH_PARAM("RVM: number of registered SWE in the system: ", rvm_swe_number);

	/* Get memory for the SWEs array	*/
	if ( rvf_get_buf( rvm_mem_bank, (swe_cpt+1)*sizeof(T_RVM_KNOWN_SWE), (T_RVF_BUFFER**)&rvm_swe_array ) == RVF_RED )
	{	
		RVM_TRACE_WARNING("RVM_init_swe_db: not enough memory in the RVM main memory bank for init of array");
		return RVM_MEMORY_ERR;
	}
	memset(rvm_swe_array, 0, (swe_cpt+1)*sizeof(T_RVM_KNOWN_SWE));

	/* call all get_info functions to gather information about the SWEs */
	for (i=0; i<swe_cpt; i++)
	{
		/* Call get info fct */
		(RVM_SWE_GET_INFO_ARRAY[i].get_info_func)(&swe_info);
		
		/* Init global structure */
		rvm_swe_array[i].swe_get_info	= RVM_SWE_GET_INFO_ARRAY[i].get_info_func;
		rvm_swe_array[i].swe_use_id		= RVM_SWE_GET_INFO_ARRAY[i].use_id;
		rvm_swe_array[i].swe_addr_id	= RVF_INVALID_ADDR_ID;
		rvm_swe_array[i].group_index	= RVM_OWN_GROUP;
		rvm_swe_array[i].swe_type		= swe_info.swe_type;

		switch(swe_info.swe_type)
		{
		case(RVM_SWE_TYPE_1):
			{	strcpy (rvm_swe_array[i].swe_name, swe_info.type_info.type1.swe_name);
				rvm_swe_array[i].swe_return_path.callback_func	= swe_info.type_info.type1.return_path.callback_func;
				rvm_swe_array[i].swe_use_id = swe_info.type_info.type1.swe_use_id;
				rvm_swe_array[i].swe_stack_size = 0;
				rvm_swe_array[i].swe_priority = 0;
				break;
			}

		case(RVM_SWE_TYPE_2):
			{	strcpy (rvm_swe_array[i].swe_name, swe_info.type_info.type2.swe_name);
				rvm_swe_array[i].swe_return_path.callback_func	= swe_info.type_info.type2.return_path.callback_func;
				rvm_swe_array[i].swe_use_id = swe_info.type_info.type2.swe_use_id;
				rvm_swe_array[i].swe_stack_size = swe_info.type_info.type2.stack_size;
				rvm_swe_array[i].swe_priority = swe_info.type_info.type2.priority;
				break;
			}

		case(RVM_SWE_TYPE_3):
			{	strcpy (rvm_swe_array[i].swe_name, swe_info.type_info.type3.swe_name);
				rvm_swe_array[i].swe_return_path.callback_func	= swe_info.type_info.type3.return_path.callback_func;
				rvm_swe_array[i].swe_use_id = swe_info.type_info.type3.swe_use_id;
				rvm_swe_array[i].swe_stack_size = swe_info.type_info.type3.stack_size;
				rvm_swe_array[i].swe_priority = swe_info.type_info.type3.priority;
				break;
			}

		case(RVM_SWE_TYPE_4):
			{	strcpy (rvm_swe_array[i].swe_name, swe_info.type_info.type4.swe_name);
				rvm_swe_array[i].swe_return_path.callback_func	= swe_info.type_info.type4.return_path.callback_func;
				rvm_swe_array[i].swe_use_id = swe_info.type_info.type4.swe_use_id;
				rvm_swe_array[i].swe_stack_size = swe_info.type_info.type4.stack_size;
				rvm_swe_array[i].swe_priority = swe_info.type_info.type4.priority;
				break;
			}
		}

		
		rvm_swe_array[i].swe_state = SWE_NOT_STARTED;
		rvm_swe_array[i].stack_ptr = NULL;
		rvm_swe_array[i].nb_using_appli = 0;

		for (j=0; j<RVM_MAX_SWE_USING; j++)
		{
			rvm_swe_array[i].using_appli[j] = RVM_INVALID_SWE_INDEX;
		}

		rvm_swe_array[i].swe_return_path.addr_id		= RVF_INVALID_ADDR_ID;

		rvm_swe_array[i].mmi_return_path.callback_func	= NULL;
		rvm_swe_array[i].mmi_return_path.addr_id		= RVF_INVALID_ADDR_ID;
	}

	RVM_TRACE_DEBUG_HIGH("RVM init: Known SWE database built");

	/* display the list of known SWEs with their name, use_id and index in the array. */
	for (i=0; i<swe_cpt; i++)
	{
		strcpy(rvm_trace_name + 10, rvm_swe_array[i].swe_name);
		rvf_send_trace(rvm_trace_name , (UINT8)strlen(rvm_trace_name), NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, RVM_USE_ID );
		RVM_TRACE_DEBUG_LOW_PARAM("SWE number:", i);
		RVM_TRACE_DEBUG_LOW_PARAM("SWE use_id:", rvm_swe_array[i].swe_use_id);
	}

	/* initialize SWEs group array */
	
	/* Get the number of groups 
	group_cpt = 0;
	while ( (RVM_TYPE2_SWE_GROUPS[group_cpt]) != NULL)
	{
		i++;
	}

	// Get memory for the groups array	
	if ( rvf_get_buf( rvm_mem_bank, (group_cpt+1)*sizeof(T_RVM_GROUP_INFO), (T_RVF_BUFFER**)&rvm_group_array ) == RVF_RED )
	{	
		RVM_TRACE_WARNING("RVM_build_swe_list: not enough memory in the RVM main memory bank for init of group array");
		return RVM_MEMORY_ERR;
	}
	memset(rvm_group_array, 0, (group_cpt+1)*sizeof(T_RVM_GROUP_INFO));

	// update the group_index of each type 2 SWE. 
	for( i = 0; i < group_cpt; i++)
	{	rvm_group_array[i].host_state	= SWE_NOT_STARTED;
		rvm_group_array[i].task_id		= RVF_INVALID_TASK;	
		rvm_group_array[i].task_priority= 255;
		
		if( RVM_TYPE2_SWE_GROUPS[i] != NULL)
		{	UINT8 index;
			j = 0;
			while( RVM_TYPE2_SWE_GROUPS[i][j] != RVM_INVALID_USE_ID)
			{	if( rvm_get_swe_index( &index, RVM_TYPE2_SWE_GROUPS[i][j]) == RVM_OK)
				{	// store the host group in the SWE 
					rvm_swe_array[index].group_index = i;
					
					// computes the highest stack size 
					if( rvm_swe_array[index].swe_stack_size > rvm_group_array[i].stack_size)
					{	rvm_group_array[i].stack_size = rvm_swe_array[index].swe_stack_size;
					}

					// computes the smallest priority 
					if( rvm_swe_array[index].swe_priority < rvm_group_array[i].task_priority)
					{	rvm_group_array[i].task_priority = rvm_swe_array[index].swe_priority;
					}
				}
				j++;
			}
		}
	}
	*/
	return RVM_OK;
}


/*******************************************************************************
** Function         rvm_get_swe_index
**
** Description      Internal function which returns the index of the swe in the
**					database of known SWEs.
**					Returns RVM_OK if it exists, else RVM_INVALID_PARAMETER.
*******************************************************************************/
T_RVM_RETURN rvm_get_swe_index( UINT8 * index, T_RVM_USE_ID swe_use_id)
{	for ( (*index) = 0; ( (*index) < rvm_swe_number) && (swe_use_id != rvm_swe_array[*index].swe_use_id) ; (*index)++);
	if ( *index == rvm_swe_number ) /* swe use id not found */
	{
		*index = RVM_INVALID_SWE_INDEX;
		return RVM_INVALID_PARAMETER;
	}
	return RVM_OK;
}


/*******************************************************************************
** Function         rvm_check_application
**
** Description      Internal function which checks if a SWE can be started
**                  or stopped, depending on appli_action parameter.
**                  If it is the case, it returns the application number.
*******************************************************************************/
T_RVM_RETURN	rvm_check_application	(T_RVM_USE_ID swe_use_id, UINT8* num_swe, T_RVM_APPLI_ACTION appli_action)
{	
	/* check if the SWE use_id exists */
	if (rvm_get_swe_index(num_swe, swe_use_id) != RVM_OK)
	{	
		return RVM_INVALID_PARAMETER;
	}
	
	/* check if the get_info function is known */
	if (rvm_swe_array[*num_swe].swe_get_info == NULL )
	{	
		*num_swe = 0;
		return RVM_NOT_READY;
	}

	/* check if the appli can be started or stopped */	
/*	if ((appli_action == RVM_START_APPLI) && ( rvm_swe_array[*num_swe].swe_state != SWE_NOT_STARTED))
	{	
		*num_swe = 0;
		return RVM_NOT_READY;
	}
*/
	/* If more than 1 SWE is using the application, we cannot stop it */
/*	if (appli_action == RVM_STOP_APPLI)
	{
		if ( (rvm_swe_array[*num_swe].swe_state != SWE_RUNNING) || \
			 (rvm_swe_array[*num_swe].nb_using_appli != 1) || \
			 (rvm_swe_array[rvm_swe_array[*num_swe].using_appli[0]].swe_use_id != rvm_swe_array[*num_swe].swe_use_id) )
		{
			*num_swe = 0;
			return RVM_NOT_READY;
		}
	}*/

	return RVM_OK;
}


/*******************************************************************************
** Function         rvm_unlock_swe
**
** Description      Internal function that sets back SWE state to NOT_STARTED
**                  in case an error occurs or the stop process is finished
*******************************************************************************/
T_RVM_RETURN	rvm_unlock_swe	( T_RVM_PROCESSING_SWE * appli)
{
	T_RVM_PROCESSING_SWE * cur_elem = appli;

	while (cur_elem)
	{
		UINT8 swe_index = cur_elem->swe_id;

		if (rvm_swe_array[swe_index].swe_state != SWE_RUNNING)
		{	
			cur_elem = cur_elem->next_swe;
			continue;
		}

		rvm_swe_array[swe_index].swe_state = SWE_NOT_STARTED;

		cur_elem = cur_elem->next_swe;
	}

	return RVM_OK;
}


/*******************************************************************************
**
** Function         rvm_build_swe_list
**
** Description      Build the list of SWEs required to launch the specified SWE.
**
** Parameters:		T_RVM_PROCESSING_SWE ** list:	list of required SWE.
**					UINT8 swe_num:				index of the SWE to start in the array of known SWEs.
**
** Returns          T_RVM_RETURN
**
*******************************************************************************/
T_RVM_RETURN rvm_build_swe_list(T_RVM_PROCESSING_SWE ** list, UINT8 swe_num, UINT8 mode)
{
	T_RVM_PROCESSING_SWE * cur_elem = *list;
	T_RVM_USE_ID loc_linked_swe_id[RVM_MAX_NB_LINKED_SWE];
	UINT8* rvm_swe_to_call;
	UINT8 rvm_current_swe_to_call = 0, rvm_last_swe_to_call = 1;
	UINT8	swe_cpt;
	UINT8	mb_cpt;
	UINT8	nb_linked_swe = 0;
	volatile T_RVM_RETURN	rvm_ret_value = RVM_OK;
	UINT8 isRunning=0;

	/* allocate a matrix to store temporarily the dependencies between SWEs */
	if (rvf_get_buf( rvm_mem_bank, rvm_swe_number*rvm_swe_number, (void**)&rvm_swe_to_call) == RVF_RED)
	{	
		rvf_send_trace("RVM_build_swe_list: not enough memory in the RVM memory bank for build_swe_list process", 87, NULL_PARAM, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );
		return RVM_MEMORY_ERR;
	}

	rvm_swe_to_call[rvm_current_swe_to_call] = swe_num;

	do
		/* While some get_info functions has to be called. */
	{
		T_RVM_INFO_SWE swe_info;
		UINT8 swe_num_i = rvm_swe_to_call[rvm_current_swe_to_call];

		/* Check if SWE is running or not */
		if (rvm_swe_array[swe_num_i].swe_state == SWE_RUNNING && mode==0) {
			rvf_send_trace("RVM_build_swe_list: SWE already running, nb: ", 45, swe_num_i, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID );
			rvm_current_swe_to_call++;
			continue;
		}

		if ( rvm_swe_array[swe_num_i].swe_state == SWE_STOPPING && mode==0) {
			rvf_send_trace("RVM_build_swe_list: SWE stopped awaiting kill, nb: ", 45, swe_num_i, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID );
			rvm_current_swe_to_call++;
			continue;
		}



		/* Check if SWE is already queued or not. */
		cur_elem = *list;
		while ( (cur_elem != NULL) && ( rvm_swe_array[cur_elem->swe_id].swe_use_id != rvm_swe_array[swe_num_i].swe_use_id) )
		{	
			cur_elem = cur_elem->next_swe;
		}

		if (cur_elem != NULL)
			/* SWE was found =>	- update SWE variables and pointers
								- go to next step of the loop. */
		{
			rvm_current_swe_to_call++;
			continue;
		}


		/* cur_elem = NULL => SWE was not found => create its entry in the list.	*/

		/* create it and call its get_info function */
		if ( rvf_get_buf( rvm_mem_bank, sizeof(T_RVM_PROCESSING_SWE), (void**)&cur_elem ) == RVF_RED )
		{	
			rvf_send_trace("RVM_build_swe_list: not enough memory in the RVM main memory bank for the SWE entry", 83, NULL_PARAM, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );
			rvm_ret_value = RVM_MEMORY_ERR;
			break;
		}

		/*
		** initialize the new entry
		*/
		memset(cur_elem, 0, sizeof(T_RVM_PROCESSING_SWE) );

		/*
		** Set the SWE id: index in known SWE array
		*/
		cur_elem->swe_id = swe_num_i;

		/* call its get_info() function */
		if ( rvm_swe_array[swe_num_i].swe_get_info(&swe_info) != RVM_OK )
		{	
			/*	Here an error occured in its get_info function.
				free memory allocated for this entry and return an error. */
			rvf_free_buf( cur_elem );
			rvf_send_trace("RVM_build_swe_list: get_info function returns an error", 54, NULL_PARAM, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );
			rvm_ret_value = RVM_INTERNAL_ERR;
			break;
		}

		cur_elem->swe_type						= swe_info.swe_type;
		cur_elem->rvm_functions.core			= rvm_generic_swe_core;

		switch( swe_info.swe_type)
		{	
		case ( RVM_SWE_TYPE_1):
			{	cur_elem->rvm_functions.set_info		= swe_info.type_info.type1.set_info;
				cur_elem->rvm_functions.init			= swe_info.type_info.type1.init;
				cur_elem->rvm_functions.start			= swe_info.type_info.type1.start;
				cur_elem->rvm_functions.stop1			= swe_info.type_info.type1.stop;
				cur_elem->rvm_functions.kill			= swe_info.type_info.type1.kill;
		
				nb_linked_swe							= swe_info.type_info.type1.nb_linked_swe;
				for( mb_cpt = 0; mb_cpt < nb_linked_swe; mb_cpt++ )
				{	loc_linked_swe_id[mb_cpt] = swe_info.type_info.type1.linked_swe_id[mb_cpt];
				}

				/* memory bank information */
				cur_elem->nb_requested_mb				= swe_info.type_info.type1.nb_mem_bank;
				
				for( mb_cpt = 0; mb_cpt < cur_elem->nb_requested_mb; mb_cpt++ )
				{	
					memcpy (cur_elem->swe_mem_bank[mb_cpt].mb_name, \
							swe_info.type_info.type1.mem_bank[mb_cpt].bank_name, \
							RVF_MAX_MB_LEN);
					cur_elem->swe_mem_bank[mb_cpt].mb_initial_param.size \
								= swe_info.type_info.type1.mem_bank[mb_cpt].initial_params.size;
					cur_elem->swe_mem_bank[mb_cpt].mb_initial_param.watermark \
								= swe_info.type_info.type1.mem_bank[mb_cpt].initial_params.watermark;
				}

				break;
			}
		case ( RVM_SWE_TYPE_2):
			{	cur_elem->rvm_functions.set_info		= swe_info.type_info.type2.set_info;
				cur_elem->rvm_functions.init			= swe_info.type_info.type2.init;
				cur_elem->rvm_functions.start			= swe_info.type_info.type2.start;
				cur_elem->rvm_functions.stop			= swe_info.type_info.type2.stop;
				cur_elem->rvm_functions.kill			= swe_info.type_info.type2.kill;
				cur_elem->rvm_functions.handle_message	= swe_info.type_info.type2.handle_message;
				cur_elem->rvm_functions.handle_timer	= swe_info.type_info.type2.handle_timer;

				cur_elem->priority						= swe_info.type_info.type2.priority;
				cur_elem->stack_size					= swe_info.type_info.type2.stack_size;
				nb_linked_swe							= swe_info.type_info.type2.nb_linked_swe;
				for( mb_cpt = 0; mb_cpt < nb_linked_swe; mb_cpt++ )
				{	loc_linked_swe_id[mb_cpt] = swe_info.type_info.type2.linked_swe_id[mb_cpt];
				}

				/* memory bank information */
				cur_elem->nb_requested_mb				= swe_info.type_info.type2.nb_mem_bank;
				
				for( mb_cpt = 0; mb_cpt < cur_elem->nb_requested_mb; mb_cpt++ )
				{	
					memcpy (cur_elem->swe_mem_bank[mb_cpt].mb_name, \
							swe_info.type_info.type2.mem_bank[mb_cpt].bank_name, \
							RVF_MAX_MB_LEN);
					cur_elem->swe_mem_bank[mb_cpt].mb_initial_param.size \
								= swe_info.type_info.type2.mem_bank[mb_cpt].initial_params.size;
					cur_elem->swe_mem_bank[mb_cpt].mb_initial_param.watermark \
								= swe_info.type_info.type2.mem_bank[mb_cpt].initial_params.watermark;
				}

				break;
			}
		case ( RVM_SWE_TYPE_3):
			{	cur_elem->rvm_functions.set_info		= swe_info.type_info.type3.set_info;
				cur_elem->rvm_functions.init			= swe_info.type_info.type3.init;
				cur_elem->rvm_functions.start			= swe_info.type_info.type3.start;
				cur_elem->rvm_functions.stop			= swe_info.type_info.type3.stop;
				cur_elem->rvm_functions.kill			= swe_info.type_info.type3.kill;
				cur_elem->rvm_functions.handle_message	= swe_info.type_info.type3.handle_message;
				cur_elem->rvm_functions.handle_timer	= swe_info.type_info.type3.handle_timer;

				cur_elem->priority						= swe_info.type_info.type3.priority;
				cur_elem->stack_size					= swe_info.type_info.type3.stack_size;
				nb_linked_swe							= swe_info.type_info.type3.nb_linked_swe;
				for( mb_cpt = 0; mb_cpt < nb_linked_swe; mb_cpt++ )
				{	loc_linked_swe_id[mb_cpt] = swe_info.type_info.type3.linked_swe_id[mb_cpt];
				}

				/* memory bank information */
				cur_elem->nb_requested_mb				= swe_info.type_info.type3.nb_mem_bank;
				
				for( mb_cpt = 0; mb_cpt < cur_elem->nb_requested_mb; mb_cpt++ )
				{	
					memcpy (cur_elem->swe_mem_bank[mb_cpt].mb_name, \
							swe_info.type_info.type3.mem_bank[mb_cpt].bank_name, \
							RVF_MAX_MB_LEN);
					cur_elem->swe_mem_bank[mb_cpt].mb_initial_param.size \
								= swe_info.type_info.type3.mem_bank[mb_cpt].initial_params.size;
					cur_elem->swe_mem_bank[mb_cpt].mb_initial_param.watermark \
								= swe_info.type_info.type3.mem_bank[mb_cpt].initial_params.watermark;
				}

				break;
			}
		case ( RVM_SWE_TYPE_4):
			{	cur_elem->rvm_functions.set_info		= swe_info.type_info.type4.set_info;
				cur_elem->rvm_functions.init			= swe_info.type_info.type4.init;
				cur_elem->rvm_functions.stop1			= swe_info.type_info.type4.stop;
				cur_elem->rvm_functions.kill			= swe_info.type_info.type4.kill;
				cur_elem->rvm_functions.core			= swe_info.type_info.type4.core;

				cur_elem->priority						= swe_info.type_info.type4.priority;
				cur_elem->stack_size					= swe_info.type_info.type4.stack_size;
				nb_linked_swe							= swe_info.type_info.type4.nb_linked_swe;
				for( mb_cpt = 0; mb_cpt < nb_linked_swe; mb_cpt++ )
				{	loc_linked_swe_id[mb_cpt] = swe_info.type_info.type4.linked_swe_id[mb_cpt];
				}

				/* memory bank information */
				cur_elem->nb_requested_mb				= swe_info.type_info.type4.nb_mem_bank;
				
				for( mb_cpt = 0; mb_cpt < cur_elem->nb_requested_mb; mb_cpt++ )
				{	
					memcpy (cur_elem->swe_mem_bank[mb_cpt].mb_name, \
							swe_info.type_info.type4.mem_bank[mb_cpt].bank_name, \
							RVF_MAX_MB_LEN);
					cur_elem->swe_mem_bank[mb_cpt].mb_initial_param.size \
								= swe_info.type_info.type4.mem_bank[mb_cpt].initial_params.size;
					cur_elem->swe_mem_bank[mb_cpt].mb_initial_param.watermark \
								= swe_info.type_info.type4.mem_bank[mb_cpt].initial_params.watermark;
				}

				break;
			}
		}

		/* allocate a task id, if necessary. */
/*		if ( (swe_info.swe_type == RVM_SWE_TYPE_3) 
			||(swe_info.swe_type == RVM_SWE_TYPE_4) 
			|| ( (swe_info.swe_type == RVM_SWE_TYPE_2) && ( rvm_swe_array[swe_num_i].group_index == RVM_OWN_GROUP) )
			|| ( (swe_info.swe_type == RVM_SWE_TYPE_2) && ( rvm_group_array[rvm_swe_array[swe_num_i].group_index].task_id == RVF_INVALID_TASK) ) )
		{	
			T_RVM_TASK_ID loc_task_id;
			if(swe_info.swe_type==RVM_SWE_TYPE_2) loc_task_id = rvm_allocate_task_id(0);
			else loc_task_id = rvm_allocate_task_id(1);

			if (loc_task_id == RVF_INVALID_TASK)
			{
				rvm_ret_value = RVM_INTERNAL_ERR;
				break;
			}
			if ( (swe_info.swe_type == RVM_SWE_TYPE_3) 
				||(swe_info.swe_type == RVM_SWE_TYPE_4) 
				|| ( (swe_info.swe_type == RVM_SWE_TYPE_2) && ( rvm_swe_array[swe_num_i].group_index == RVM_OWN_GROUP) ) )
			{	rvm_swe_array[swe_num_i].swe_addr_id = loc_task_id;
			}
			else
			{	if ( (swe_info.swe_type == RVM_SWE_TYPE_2) && ( rvm_group_array[rvm_swe_array[swe_num_i].group_index].task_id == RVF_INVALID_TASK) )
				{	rvm_group_array[rvm_swe_array[swe_num_i].group_index].task_id = loc_task_id;
				}
			}

		}
*/
		/* Insert the element in the head of the list. */
		if (*list == NULL)	/* the list is empty */
		{
			/* This element is the first of the list */
			*list = cur_elem;
			cur_elem->next_swe = NULL;
		}
		else
		{	cur_elem->next_swe = *list;
			*list = cur_elem;
		}

		/* Get the list of linked swe num, and put it in the array */
		if (nb_linked_swe != 0)	{
				for (swe_cpt = 0; swe_cpt < nb_linked_swe; swe_cpt++) {
					if (rvm_get_swe_index(&(rvm_swe_to_call[rvm_last_swe_to_call + swe_cpt]), loc_linked_swe_id[swe_cpt] ) != RVM_OK) {
					rvm_ret_value = RVM_INVALID_PARAMETER;
					rvf_send_trace("rvm_swe_db: Task allocation error!",35, NULL_PARAM, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );	
					break;
				}
			}
				if (rvm_ret_value != RVM_OK) break;

				rvm_last_swe_to_call += nb_linked_swe;
		}
		/* Once everything is done for current swe, increment rvm_current_swe_to_call */
		rvm_current_swe_to_call ++;
		
	}
	while (rvm_current_swe_to_call != rvm_last_swe_to_call);


	if (rvm_ret_value != RVM_OK)
		/* Something went wrong => undo everything	*/
	{
		rvm_delete_used_memory (*list);
	}

	rvf_free_buf (rvm_swe_to_call);
	return (rvm_ret_value);
}


/*******************************************************************************
**
** Function         rvm_clean_env
**
** Description      This function will clean the environment.
**                  Its main tasks are:
**                    - Update the using_appli pointer of the SWE array
**                    - Update states to NOT_STARTED
**                    - Release local memory
**
** Parameters:		T_RVM_PROCESSING_SWE * appli: list of required SWEs with their parameters.
**
** Returns          T_RVM_OK if all allocation are successful,
**                  else T_RVM_INTERNAL_ERR (then some SWE are not killed.)
**
*******************************************************************************/
T_RVM_RETURN rvm_clean_env( T_RVM_PROCESSING_SWE * appli)
{	
	T_RVM_PROCESSING_SWE * cur_swe = appli;
	UINT8 appli_nb1, appli_nb2;
	volatile T_RVM_RETURN rvm_ret_value = RVM_OK;
	
	/* for each SWE in the list */
	while (cur_swe != NULL )
	{
		UINT8 swe_index = cur_swe->swe_id;

		/* Update using_appli array	*/
		if (rvm_swe_array[swe_index].nb_using_appli > 1)
		{
			for (appli_nb1 = 0; appli_nb1 < (rvm_swe_array[swe_index].nb_using_appli - 1); appli_nb1++)
			{
				if (rvm_swe_array[swe_index].using_appli[appli_nb1] == appli->swe_id)
					/* appli was found in the using_appli array -> remove it and shift down the others */
				{
					for (appli_nb2 = appli_nb1; appli_nb2 < rvm_swe_array[swe_index].nb_using_appli; appli_nb2++)
					{
						rvm_swe_array[swe_index].using_appli[appli_nb2] = rvm_swe_array[swe_index].using_appli[appli_nb2 + 1];
					}
					/* Once using_appli pointer has been eliminated, we can exit the loop */
					break;
				}

			}
		}

		/* Decrement nb of using applications */
		rvm_swe_array[swe_index].nb_using_appli--;

		/* Force to NULL last appli pointer	*/
		rvm_swe_array[swe_index].using_appli[rvm_swe_array[swe_index].nb_using_appli] = RVM_INVALID_SWE_INDEX;

		/* If last appli was deleted, put state back to init */
		if (rvm_swe_array[swe_index].nb_using_appli == 0)
		{
			rvm_swe_array[swe_index].swe_state = SWE_NOT_STARTED;
		}

		/* Proceed to the next SWE	*/
		cur_swe = cur_swe->next_swe;
	}

	/* Once Everything is back in stand-by, release used memory	*/
	rvm_delete_used_memory (appli);

	return rvm_ret_value;
}


/*******************************************************************************
**
** Function         rvm_error
**
** Description      Called by a SWE in case of unrecoverable error
**
** Parameters:		T_RVM_NAME swe_name:
**					T_RVM_RETURN error_cause: 
**					T_RVM_ERROR_TYPE error_type:
**					T_RVM_STRING error_msg:
**
** Returns          T_RVM_RETURN: RVM_OK if successful, else a negative value.
**
*******************************************************************************/
T_RVM_RETURN	rvm_error ( T_RVM_NAME swe_name, T_RVM_RETURN error_cause, 
							T_RVM_ERROR_TYPE error_type, T_RVM_STRING error_msg)
{	
	char swe_name_string[40];
	UINT8 i=0;

	memcpy(swe_name_string,"RVM: coming from: ",18);
	memcpy((void*)((char*)swe_name_string+18),swe_name,RVM_NAME_MAX_LEN);

	RVM_TRACE_WARNING("RVM: unrecoverable error indication");
	rvf_send_trace(swe_name_string, 18 + RVM_NAME_MAX_LEN, NULL_PARAM, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );
	RVM_TRACE_WARNING_PARAM("RVM: Error Cause: ", error_cause);
	RVM_TRACE_WARNING_PARAM("RVM: Error Type: ", error_type);
	/* check the message to limit its length to RVM_ERROR_MSG_MAX_LENGTH characters. */
	while (error_msg[i])
	{
		i++;
		if (i>RVM_ERROR_MSG_MAX_LENGTH)
			break;
	}

	rvf_send_trace(error_msg, i, NULL_PARAM, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );

	return RVM_OK;
}

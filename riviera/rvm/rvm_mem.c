/**
 *                                                                          
 *  @file	rvm_mem.c                                              
 *                                                                          
 *  This file contains the functions related to memory management within RVM.
 *
 * @author	David Lamy-Charrier (d-lamy@ti.com)	
 * @version	0.1
 *
 */

/*
 * Revision History:																			
 *
 * 10/26/2001	David Lamy-Charrier		Create for Riviera 1.6.
 *																			
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */

#include "rvm_i.h"
#include "rvm_gen.h"
#include "rvm_api.h"
#include "rvm_use_id_list.h"
#include "../rvf/rvf_env.h"

extern T_RVM_GET_INFO_FUNC RVM_SWE_GET_INFO_ARRAY[];

extern T_RVM_KNOWN_SWE	* rvm_swe_array;

/*******************************************************************************
** Function         rvm_delete_used_memory
**
** Description      Internal function which deletes used local mem if an error
**                  was received or at the end of the start/stop process.
**					
*******************************************************************************/
T_RVM_RETURN rvm_delete_used_memory ( T_RVM_PROCESSING_SWE * appli)
{
	T_RVF_BUFFER_Q			buffer_to_free_q = {0, 0, 0};
	T_RVM_PROCESSING_SWE		* cur_elem = appli;

	while (cur_elem != NULL)
	{
		rvf_enqueue (&buffer_to_free_q, cur_elem);
		cur_elem = cur_elem->next_swe;
	}

	while (buffer_to_free_q.p_first)
	{
		rvf_free_buf (rvf_dequeue (&buffer_to_free_q));
	}

	return RVM_OK;
}


/*******************************************************************************
** Function         rvm_delete_created_mb
**
** Description      Internal function which deletes all created MB if an error
**                  was received, or in case application has to be stopped.
**
*******************************************************************************/
T_RVM_RETURN rvm_delete_created_mb (T_RVM_PROCESSING_SWE * appli)
{
	UINT8 mb_index;
	T_RVM_PROCESSING_SWE * cur_elem = appli;
	volatile T_RV_RET ret_value = RV_OK;
	UINT8 mb_to_delete;

	while (cur_elem != NULL )
	{
		UINT8 swe_index = cur_elem->swe_id;

		/*	If more than one appli is using this SWE, cannot delete MB
			Process to the next SWE. */
		if (rvm_swe_array[swe_index].nb_using_appli > 1)
		{	
			cur_elem = cur_elem->next_swe; /* process the next SWE */ 
			continue;
		}

		/*	If the state is running, it means that this SWE has not
			to be stopped. */
		if (rvm_swe_array[swe_index].swe_state == SWE_RUNNING)
		{
			cur_elem = cur_elem->next_swe; /* process the next SWE */
			continue;
		}

		/* We're here:
			- either because swe_state == SWE_NOT_STARTED => error in starting prcess
			- either because swe_state == SWE_STOPPING => regular stopping process */

		if (cur_elem->nb_created_mb == 0)
		{
			cur_elem = cur_elem->next_swe; /* process the next SWE */
			continue;
		}

		mb_to_delete = cur_elem->nb_created_mb;

		for( mb_index = 0; mb_index < mb_to_delete; mb_index++)
		{
			ret_value = rvf_delete_mb(cur_elem->swe_mem_bank[mb_index].mb_name);
			if (ret_value != RV_OK)
			{
				rvf_send_trace("RVM: Error in deletion of memory bank: ", 39, NULL_PARAM, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );
				rvf_send_trace(cur_elem->swe_mem_bank[mb_index].mb_name, RVF_MAX_MB_LEN, NULL_PARAM, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );
			}
			else
			{
				(cur_elem->nb_created_mb)--;
			}

			ret_value = RVM_OK;
		}

		cur_elem = cur_elem->next_swe; /* process the next SWE */ 
	}

	return ret_value;
}

/*******************************************************************************
**
** Function         rvm_check_memory_requirement
**
** Description      This function checks if there is enough memory 
**					to start a SWE(and all the linked SWEs)
**
** Parameters:		T_RVM_PROCESSING_SWE * appli: list of required SWEs with their parameters.
**
** Returns          T_RVM_RETURN: RVM_OK if there is enough memory, else RVM_MEMORY_ERR.
**
*******************************************************************************/
T_RVM_RETURN rvm_verify_memory_requirement( T_RVM_PROCESSING_SWE * appli,
										    T_RVM_GROUP_DIRECTIVE* gd, 
											UINT8 cnt) {	
	T_RVM_PROCESSING_SWE * cur_swe = appli;
	UINT32 required_mem = 0;
	UINT32 total_mem = 0;
	UINT32 used_mem = 0;
	UINT8 mb_index;
	UINT8 i=0;
	UINT16 host_task_mem=0;


	/* get available memory from the rvf */
	if ( rvf_get_available_mem( &total_mem, &used_mem) != RVF_OK )
	{	return RVM_MEMORY_ERR;
	}

	/* count required memory */
	while (cur_swe !=NULL ) /* for each SWE */
	{
		UINT8 swe_index = cur_swe->swe_id;

		/*
		** If SWE is already running => MB already created => do nothing
		*/
		if (rvm_swe_array[swe_index].swe_state != SWE_RUNNING )
		{
			for( mb_index = 0; mb_index < cur_swe->nb_requested_mb; mb_index++) /* for each mb */
			{	
				required_mem += cur_swe->swe_mem_bank[mb_index].mb_initial_param.size;
			}

			/* add the necessary stack sizes */
			/* TO DO: add the stack size for host groups not yet started */
			if( (cur_swe->swe_type == RVM_SWE_TYPE_3)
				|| (cur_swe->swe_type == RVM_SWE_TYPE_4) )
/*				|| ((cur_swe->swe_type == RVM_SWE_TYPE_2) && (rvm_swe_array[swe_index].group_index == RVM_OWN_GROUP) )) */
			{	
				required_mem += rvm_swe_array[swe_index].swe_stack_size;
				required_mem += SYSTEM_TASK_MEM;  /* only for type 3 & 4. A-M-E-N-D-E-D!    */
			}

		}
		cur_swe = cur_swe->next_swe;
	}

	/* type 2 group host system and stack mem. is catered for here								*/
	for(i=0; i<cnt; i++) host_task_mem+=gd[i].stack_size;			/* A-M-E-N-D-E-D!  */
	host_task_mem+=(cnt*SYSTEM_TASK_MEM);
	
	/* compare available memory and required memory (eventually, use a percentage to improve performances) */
	if ((required_mem+host_task_mem) + used_mem > total_mem ) { 	/* A-M-E-N-D-E-D!  */
		RVM_TRACE_WARNING_PARAM("RVM: Memory required (incl. used): ", (UINT32)(required_mem+host_task_mem+ used_mem) );
		RVM_TRACE_WARNING_PARAM("RVM: Total Memory available      : ", (UINT32)total_mem);
		return RVM_MEMORY_ERR;
	} else{
		return RVM_OK;
	}
}


/*******************************************************************************
**
** Function         rvm_allocate_mem
**
** Description      This function creates all the required memory banks or
**					increases their size if needed, to start a SWE. 
**
** Parameters:		T_RVM_PROCESSING_SWE * appli: list of required SWEs with their parameters.
**
** Returns          T_RVM_RETURN: RVM_OK if all allocations are successful, 
**					else RVM_MEMORY_ERR and it releases all the allocated memory.
**
*******************************************************************************/
T_RVM_RETURN rvm_allocate_mb( T_RVM_PROCESSING_SWE * appli)
{	T_RVM_PROCESSING_SWE * cur_swe = appli;
	UINT8 mb_index;
	volatile T_RVM_RETURN rvm_ret_value = RVM_OK;
	
	/* for each SWE in the list */
	while ((cur_swe != NULL ) && (rvm_ret_value == RVM_OK))
	{
		UINT8 swe_index = cur_swe->swe_id;

		if (rvm_swe_array[swe_index].swe_state == SWE_NOT_STARTED) 
			/* If the state is not SWE_RUNNING, then the MBs have to be created	*/
		{
			for( mb_index = 0; mb_index < cur_swe->nb_requested_mb; mb_index++) /* for each mb */
			{	T_RVF_MB_PARAM mb_param;
				/* add the initial size */
				mb_param.size = cur_swe->swe_mem_bank[mb_index].mb_initial_param.size;
				mb_param.watermark = cur_swe->swe_mem_bank[mb_index].mb_initial_param.watermark;

				/* create the mb */
				if ( mb_param.size != 0)
				{	
					/* create the mb */
					if ( rvf_create_mb( cur_swe->swe_mem_bank[mb_index].mb_name, mb_param, &(cur_swe->bk_id_table[mb_index]) ) != RVF_OK)
					{
						/* if an error occurs */
						rvm_ret_value = RVM_MEMORY_ERR;
						break;
					}
					else
					{
						cur_swe->nb_created_mb++;
					}
				}
			}
		}

		cur_swe = cur_swe->next_swe; /* process the next SWE */ 
	}

	if (rvm_ret_value != RVM_OK)
		/* Something went wrong, should release all used memory */
	{
		rvf_send_trace("RVM: Problem in memory bank creation !!!", 40, NULL_PARAM, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );
		if (rvm_delete_created_mb (appli) != RVM_OK)
		{
			rvf_send_trace("RVM: MB deleting error!!!", 25, NULL_PARAM, RV_TRACE_LEVEL_WARNING, RVM_USE_ID );
		}

	}
	return rvm_ret_value;
}


/*******************************************************************************
**
** Function         rvm_allocate_stack_buffer
**
** Description      This function allocates a buffer for the stack of a new 
**					task created by RVM.
**
** Parameters:		UINT32 stack_size: size of stack.
**					T_RVF_BUFFER** stack_ptr: pointer to the allocated buffer.
**
** Returns          T_RVM_RETURN: RVM_OK if all allocation is successful, 
**					else RVM_MEMORY_ERR.
**
*******************************************************************************/
T_RVM_RETURN rvm_allocate_stack_buffer( UINT32 stack_size, T_RVF_BUFFER** stack_ptr )
{	T_RVF_MB_PARAM mb_params;

	/* increase the size of the stack MB before allocating the new buffer */
	if( rvf_get_mb_param( RVM_STACK_MB, &mb_params) != RV_OK)
	{	return RVM_MEMORY_ERR;
	}
	
	mb_params.size		+= stack_size;
	mb_params.watermark	+= stack_size;

	if( rvf_set_mb_param( RVM_STACK_MB, &mb_params) != RV_OK)
	{	return RVM_MEMORY_ERR;
	}

	if( rvf_get_buf( rvm_stack_mem_bank, stack_size, stack_ptr) == RVF_RED)
	{	return RVM_MEMORY_ERR;
	}
	return RVM_OK;

}

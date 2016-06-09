/**
 *                                                                          
 *  @file	rvm_swe_hdlr.c                                              
 *                                                                          
 *  This file contains the functions related to SWEs management within RVM.
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
#include <stdio.h>
#include "../rvf/rvf_env.h"
#include "rvm_gen.h"
#include "rvm_api.h"
#include "rvm_i.h"

#include "../rvf/rvf_i.h"		/* ET2 rvf private invocation API */
#include "../rvf/rvf_api.h"		/* A-M-E-N-D-E-D! */

/* temporary inclusion for HCI pb on WINDOWS */
/* TO DO: remove it. */
#include "rvm_use_id_list.h"

#include <string.h>

extern T_RVM_CONST_SWE_INFO RVM_SWE_GET_INFO_ARRAY[];

extern T_RVM_USE_ID * RVM_TYPE2_SWE_GROUPS[];

extern BOOLEAN rvm_allocated_task_id [MAX_RVF_TASKS];

extern T_RVM_KNOWN_SWE	* rvm_swe_array;


/* private      */
T_RVM_RETURN _fatal(T_RVM_PROCESSING_SWE* appli, UINT8 rm);

/***********************************************************************
* Function         _resolve_t2_grouping   (private)
*
* Description      resolves number of group directives & ret. group count
*************************************************************************/
UINT8 _resolve_t2_grouping(T_RVM_PROCESSING_SWE* appli, T_RVM_GROUP_DIRECTIVE* gd) {
	T_RVM_INFO_SWE			swe_info;
	T_RVM_PROCESSING_SWE*	cur_swe = appli;
	UINT8 i=0, j=0, k=0;

	for(; cur_swe != NULL;)	{
		UINT8 swe_index = cur_swe->swe_id;

		rvm_swe_array[swe_index].swe_get_info(&swe_info);

		if (rvm_swe_array[swe_index].swe_state !=SWE_RUNNING && //== SWE_NOT_STARTED && 
			swe_info.swe_type==RVM_SWE_TYPE_2) {

			for(i=0; i<MAX_GRPS; i++) {
				if(swe_info.type_info.type2.swe_group_directive == gd[i].group_directive) {
					for(k=0; gd[i].hosted_swe_db_index[k]!=0; k++);
					if(k<MAX_COMPOSITES) {
						gd[i].hosted_swe_db_index[k]=swe_index;
					} else {
						/* TO DO ... ERROR !!! */
					}
//					RVM_TRACE_WARNING_PARAM("rvm.SweHndlr.resolve_t2_grouping(), appended to grp entry , nb=",\
//											 (UINT32)swe_index);
//printf("rvm.SweHndlr.resolve_t2_grouping(): appended %d to group: %d\n",gd[i].hosted_swe_db_index[k], gd[i].host_task_addr); 

					break;
				} else if(	swe_info.type_info.type2.swe_group_directive != gd[i].group_directive &&
							gd[i].host_task_addr==0 ) {

					/* Constraint! Expects all group priorites and stack sz to be equal 
					 * Additional method must be used to set highest entity	pri. or resolve			*/
					gd[i].host_task_addr=RVF_INVALID_ADDR_ID; //rvm_allocate_task_id(1);
					gd[i].group_directive=swe_info.type_info.type2.swe_group_directive;	
					gd[i].task_priority=swe_info.type_info.type2.priority;
					gd[i].stack_size=swe_info.type_info.type2.stack_size;

					gd[i].hosted_swe_db_index[0]=swe_index;
					j++;
//					RVM_TRACE_WARNING_PARAM("rvm.SweHndlr.resolve_t2_grouping(), created grp entry , nb=",\
//											 (UINT32)swe_index);
//printf("rvm.SweHndlr.resolve_t2_grouping(): created host group: %d AND append %d\n",gd[i].host_task_addr, gd[i].hosted_swe_db_index[0]); 
					break;	
				} 			
			}

		} else RVM_TRACE_WARNING_PARAM("rvm.SweHndlr.resolve_t2_grouping(), SWE Not type 2: ", rvm_swe_array[swe_index].swe_use_id); 
		cur_swe = cur_swe->next_swe; /* process next SWE */
	}
//printf("rvm.SweHndlr.resolve_t2_grouping(): total group count: %d\n", j);

//for(i=0; i<j; i++) /* de'bugger only!! */
//	for(k=0; k<MAX_COMPOSITES && gd[i].hosted_swe_db_index[k]!=0; k++) 
//		printf("host addr: %d, T2 swe_db_index %d\n",
//		gd[i].host_task_addr, gd[i].hosted_swe_db_index[k]);

	return j;
}

/*******************************************************************************
** Function         rvm_allocate_task_id
**
** Description      Internal function which allocate the first available
**					task id to a SWE in creation
*******************************************************************************/
T_RVM_TASK_ID rvm_allocate_task_id(UINT8 isRealTask) {
/*	UINT8 i=0;	*/

	/*	Find the 1st free task id
		If we reach the max: all task ids are allocated => not possible to start SWE.*/
	/* while (rvm_allocated_task_id[i] == TRUE)
	{
		i++;
		if (i == MAX_RVF_TASKS)
			return RVF_INVALID_TASK;
	}*/

	/* Lock task id and return its value. */
	/* rvm_allocated_task_id[i] = TRUE; */
	/* return ((T_RVM_TASK_ID) i); */
	return (T_RVM_TASK_ID) rvf_allocate_task_id(isRealTask); /* A-M-E-N-D-E-D! */
}


/*******************************************************************************
**
** Function         rvm_set_swe_info
**
** Description      This function call the set_info function of each SWEs required 
**					to start a specified SWE.
**
** Parameters:		T_RVM_PROCESSING_SWE * appli: list of required SWEs with their parameters.
**
** Returns          T_RVM_RETURN: RVM_OK if successful.
**
*******************************************************************************/
T_RVM_RETURN	rvm_set_swe_info(T_RVM_PROCESSING_SWE * appli)
{	
	T_RVM_PROCESSING_SWE * cur_swe = appli;
	UINT8 i;
	T_RVF_MB_ID	_bk_id_table[RVM_MAX_NB_MEM_BK];

	/* for each SWE in the list */
	while( cur_swe != NULL )
	{
		UINT8 swe_index = cur_swe->swe_id;

		if (rvm_swe_array[swe_index].swe_state != SWE_RUNNING)
			/* Call the set_info function for only those for which MB were just created	*/
		{
			/* First build return path	*/
			T_RVM_INFO_SWE		swe_info;
			T_RV_RETURN_PATH	return_path[RVM_MAX_NB_LINKED_SWE];
			T_RVM_USE_ID		linked_swe_use_id[RVM_MAX_NB_LINKED_SWE];
			UINT8				nb_linked_swe = 0;

			rvm_swe_array[swe_index].swe_get_info(&swe_info);

			switch( swe_info.swe_type)
			{
			case(RVM_SWE_TYPE_1):
				{	nb_linked_swe = swe_info.type_info.type1.nb_linked_swe;
					memcpy( linked_swe_use_id, swe_info.type_info.type1.linked_swe_id, RVM_MAX_NB_LINKED_SWE * sizeof(T_RVM_USE_ID) );
					if(rvm_swe_array[swe_index].swe_state != SWE_NOT_STARTED) {
						for(i=0;i<swe_info.type_info.type1.nb_mem_bank; i++) {
							rvf_get_mb_id((char*)&swe_info.type_info.type1.mem_bank[i],
							&_bk_id_table[i]);
						}
					}
					break;
				}
			case(RVM_SWE_TYPE_2):
				{	nb_linked_swe = swe_info.type_info.type2.nb_linked_swe;
					memcpy( linked_swe_use_id, swe_info.type_info.type2.linked_swe_id, RVM_MAX_NB_LINKED_SWE * sizeof(T_RVM_USE_ID) );
					if((rvm_swe_array[cur_swe->swe_id].swe_addr_id = rvm_allocate_task_id(0))==RVF_INVALID_ADDR_ID) {
						return RVM_INTERNAL_ERR;
					}
					if(rvm_swe_array[swe_index].swe_state != SWE_NOT_STARTED) {
						for(i=0;i<swe_info.type_info.type2.nb_mem_bank; i++) {
							rvf_get_mb_id((char*)&swe_info.type_info.type2.mem_bank[i],
							&_bk_id_table[i]);
						}
					}
					break;
				}
			case(RVM_SWE_TYPE_3):
				{	nb_linked_swe = swe_info.type_info.type3.nb_linked_swe;
					memcpy( linked_swe_use_id, swe_info.type_info.type3.linked_swe_id, RVM_MAX_NB_LINKED_SWE * sizeof(T_RVM_USE_ID) );
					if((rvm_swe_array[cur_swe->swe_id].swe_addr_id = rvm_allocate_task_id(1))==RVF_INVALID_ADDR_ID) {
						return RVM_INTERNAL_ERR;
					}
					if(rvm_swe_array[swe_index].swe_state != SWE_NOT_STARTED) {
						for(i=0;i<swe_info.type_info.type3.nb_mem_bank; i++) {
							rvf_get_mb_id((char*)&swe_info.type_info.type3.mem_bank[i],
							&_bk_id_table[i]);
						}					
					}
					break;
				}
			case(RVM_SWE_TYPE_4):
				{	nb_linked_swe = swe_info.type_info.type4.nb_linked_swe;
					memcpy( linked_swe_use_id, swe_info.type_info.type4.linked_swe_id, RVM_MAX_NB_LINKED_SWE * sizeof(T_RVM_USE_ID) );
					if((rvm_swe_array[cur_swe->swe_id].swe_addr_id = rvm_allocate_task_id(1))==RVF_INVALID_ADDR_ID) {
						return RVM_INTERNAL_ERR;
					}
					if(rvm_swe_array[swe_index].swe_state != SWE_NOT_STARTED) {
						for(i=0;i<swe_info.type_info.type4.nb_mem_bank; i++) {
							rvf_get_mb_id((char*)&swe_info.type_info.type4.mem_bank[i],
							&_bk_id_table[i]);
						}					
					}
					break;
				}
			}
			rvm_swe_array[cur_swe->swe_id].swe_return_path.addr_id=rvm_swe_array[cur_swe->swe_id].swe_addr_id;

			for (i=0; i < nb_linked_swe; i++)
			{
				UINT8 linked_swe_index;
				if (rvm_get_swe_index(&linked_swe_index, linked_swe_use_id[i]) != RVM_OK)
				{
					return RVM_INTERNAL_ERR;
				}
				return_path[i].callback_func	= rvm_swe_array[linked_swe_index].swe_return_path.callback_func;
				/* TO DO: manage addr_id for GROUP_MEMBER SWEs */
				return_path[i].addr_id			= rvm_swe_array[linked_swe_index].swe_addr_id;
			}


			if (cur_swe->rvm_functions.set_info != NULL ) {
				if(rvm_swe_array[swe_index].swe_state == SWE_NOT_STARTED) {
				cur_swe->rvm_functions.set_info(rvm_swe_array[cur_swe->swe_id].swe_addr_id, \
												return_path, \
												cur_swe->bk_id_table, \
												rvm_error);
				} else {
					cur_swe->rvm_functions.set_info(rvm_swe_array[cur_swe->swe_id].swe_addr_id, \
													return_path, \
													_bk_id_table, \
													rvm_error);
				}
			}
		}

		cur_swe = cur_swe->next_swe; /* process next SWE */
	}
	return RVM_OK;
}


/*******************************************************************************
**
** Function         rvm_initialize_swe
**
** Description      This function initialize all the required SWEs which are not running.
**					It also creates the tasks in a suspend state.
**					Then it resumes the tasks and call the start function of each SWE.
**
** Parameters:		T_RVM_PROCESSING_SWE * appli: list of required SWEs with their parameters.
**
** Returns          T_RVM_RETURN: RVM_OK if successful.
**
*******************************************************************************/
T_RVM_RETURN		rvm_initialize_swe( T_RVM_PROCESSING_SWE * appli,
										T_RVM_GROUP_DIRECTIVE* gd, 
										UINT8 t2cnt) {	
	T_RVM_PROCESSING_SWE * cur_swe = appli;
	UINT8 i=0, j=0;
	UINT16 tuid=0;
	T_RVF_BUFFER* stack_ptr=NULL;
	T_RVM_INFO_SWE swe_info;

#ifdef _WINDOWS
	BOOLEAN hci_started = FALSE;
#endif

	/* for each SWE in the list, initialize it */
	while( cur_swe != NULL )
	{	
		UINT8 swe_index = cur_swe->swe_id;

		if ( rvm_swe_array[swe_index].swe_state != SWE_RUNNING)
		{
			/* call its init function */
			if (cur_swe->rvm_functions.init)
			{
				if (cur_swe->rvm_functions.init() != RVM_OK)
				{
					rvf_send_trace("RVM: Error Calling init function of swe nb ", 43, \
								(UINT32)swe_index, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
				}
			}
		}
		cur_swe = cur_swe->next_swe;
	}


	/* for each SWE in the list, create the task if necessary. */
	cur_swe = appli;
	while( cur_swe != NULL )
	{
		UINT8 swe_index = cur_swe->swe_id;

		if ( rvm_swe_array[swe_index].swe_state != SWE_RUNNING)		{
			/* start the task if necessary in SUSPEND mode */

			if ( cur_swe->swe_type == RVM_SWE_TYPE_4) {
				/* allocate a buffer for the stack */
				if ( rvm_allocate_stack_buffer( cur_swe->stack_size,
					 &rvm_swe_array[swe_index].stack_ptr) != RVM_OK) {

					rvf_send_trace("RVM: Error allocating stack nb:", 28, (UINT32)rvm_swe_array[swe_index].swe_addr_id, RV_TRACE_LEVEL_ERROR, RVM_USE_ID);

					/* TO DO: manage the error case */
					return RVF_MEMORY_ERR;
				}

				/* start the task in suspend mode */
				if (rvf_create_task((TASKPTR) cur_swe->rvm_functions.core, \
									(UINT8)rvm_swe_array[swe_index].swe_addr_id,\
									rvm_swe_array[swe_index].swe_name, \
									rvm_swe_array[swe_index].stack_ptr, \
									cur_swe->stack_size, \
									cur_swe->priority, \
									ET4_TASK,\
									DEFAULT_TIME_SLICING, \
									SUSPEND ) != RV_OK) {

					rvf_send_trace("RVM: Error Creating Task nb:", 28, (UINT32)rvm_swe_array[swe_index].swe_addr_id, RV_TRACE_LEVEL_ERROR, RVM_USE_ID);
				}

				rvf_setRtAddrSweIndex(rvm_swe_array[swe_index].swe_addr_id, 
									  swe_index);

				rvf_send_trace("RVM: Created task nb ", 21, (UINT32)rvm_swe_array[swe_index].swe_addr_id, RV_TRACE_LEVEL_DEBUG_LOW, RVM_USE_ID);

			} else if (cur_swe->swe_type == RVM_SWE_TYPE_3) {
				/* allocate a buffer for the stack */
				if ( rvm_allocate_stack_buffer( cur_swe->stack_size,
					 &rvm_swe_array[swe_index].stack_ptr) != RVM_OK) {

					rvf_send_trace("RVM: Error allocating stack nb:", 28, (UINT32)rvm_swe_array[swe_index].swe_addr_id, RV_TRACE_LEVEL_ERROR, RVM_USE_ID);
					/* TO DO: manage the error case */
					return RVF_MEMORY_ERR;
				}

				/* start the task in suspend mode */
				if (rvf_create_task((TASKPTR)rvm_t3_proxy, \
									(UINT8)rvm_swe_array[swe_index].swe_addr_id,\
									rvm_swe_array[swe_index].swe_name, \
									rvm_swe_array[swe_index].stack_ptr, \
									cur_swe->stack_size, \
									cur_swe->priority, \
									ET3_TASK,\
									DEFAULT_TIME_SLICING, \
									SUSPEND ) != RV_OK) {

					rvf_send_trace("RVM: Error Creating E3 Task nb:", 28, (UINT32)rvm_swe_array[swe_index].swe_addr_id, RV_TRACE_LEVEL_ERROR, RVM_USE_ID);
				}
			
				rvf_register_t3_handlers(rvm_swe_array[swe_index].swe_addr_id,
										 cur_swe->rvm_functions.handle_message, /* traverse list hence: cur_swe->rvm_functions */
										 cur_swe->rvm_functions.handle_timer );

				rvf_setRtAddrSweIndex(rvm_swe_array[swe_index].swe_addr_id, 
									  swe_index);

				rvf_send_trace("RVM: Created task nb ", 21, (UINT32)rvm_swe_array[swe_index].swe_addr_id, RV_TRACE_LEVEL_DEBUG_LOW, RVM_USE_ID);
	
			
			}
		}
		cur_swe = cur_swe->next_swe; /* process next SWE */
	} 
	/* resolve T2 grouping			*/
	for(i=0; i<t2cnt; i++) {
		gd[i].host_task_addr=rvf_resolveHostingAddrId(gd[i]);
		if( gd[i].host_task_addr==RVF_INVALID_ADDR_ID) {

		if ( rvm_allocate_stack_buffer( gd[i].stack_size, &stack_ptr) != RVM_OK){
			/* TO DO: manage the error case - ABORT & Clean-up if one or more linked Ent. fail */
			//break;
			return RVF_MEMORY_ERR;
		} 

			gd[i].host_task_addr=rvm_allocate_task_id(1);
		rvf_create_task((TASKPTR)rvm_t2_proxy,
						gd[i].host_task_addr, //
						"hosting_task",
						stack_ptr,
						gd[i].stack_size,
						gd[i].task_priority,
						ET2_HOST_TASK,		
						DEFAULT_TIME_SLICING,
						SUSPEND);

			rvf_associateGrpToHost(gd[i].host_task_addr, gd[i].group_directive);
		}  

		for(j=0; j<MAX_COMPOSITES && gd[i].hosted_swe_db_index[j]!=0; j++) {
			/* create virtual task for each "hosted_swe_db_index[]" */
			rvm_swe_array[gd[i].hosted_swe_db_index[j]].swe_get_info(&swe_info);

			rvf_create_virtual_task(swe_info.type_info.type2.handle_message,
									swe_info.type_info.type2.handle_timer,
									rvm_swe_array[gd[i].hosted_swe_db_index[j]].swe_addr_id,
									gd[i].host_task_addr,
									rvm_swe_array[gd[i].hosted_swe_db_index[j]].swe_name,
									rvm_swe_array[gd[i].hosted_swe_db_index[j]].swe_priority,
									ET2_VTASK);
			rvf_setRtAddrSweIndex(rvm_swe_array[gd[i].hosted_swe_db_index[j]].swe_addr_id, 
								  gd[i].hosted_swe_db_index[j]);

			/* register	each with associate host						     */
			rvf_registerToHost( gd[i].host_task_addr, 
								rvm_swe_array[gd[i].hosted_swe_db_index[j]].swe_addr_id);
		}
	
	}

	/* resume all hosting tasks...		*/
	for(i=0; i<t2cnt; i++) rvf_resume_task((UINT8)gd[i].host_task_addr); 

	/* start composites or virtual tasks   */
	for(i=0; i<t2cnt; i++) {
		rvm_start_group_req((UINT8)gd[i].host_task_addr, 
		gd[i].hosted_swe_db_index);
	}

	/* for each SWE in the list, start it if necessary. */
	for(cur_swe = appli; cur_swe != NULL; ) {	
		UINT8 swe_index = cur_swe->swe_id;

		if ( rvm_swe_array[swe_index].swe_state != SWE_RUNNING)	{
			/* if the SWE is a task, resume it */
			if ( (cur_swe->swe_type == RVM_SWE_TYPE_3)
				|| (cur_swe->swe_type == RVM_SWE_TYPE_4) ) {

				/* TO DO: check the return value */
				if(rvf_resume_task((UINT8)rvm_swe_array[swe_index].swe_addr_id )!=RVF_OK) {
					RVM_TRACE_WARNING("RVM: ERROR! UNABLE TO RESUME SWE");
					return RVF_INTERNAL_ERR;
				}
				rvf_send_trace("RVM: Resumed task nb ", 21, (UINT32)rvm_swe_array[swe_index].swe_addr_id, RV_TRACE_LEVEL_DEBUG_LOW, RVM_USE_ID);
				rvf_send_trace("RVM: Resumed SWE ", 17, (UINT32)rvm_swe_array[swe_index].swe_use_id, RV_TRACE_LEVEL_DEBUG_LOW, RVM_USE_ID);

#ifdef _WINDOWS
				if (rvm_swe_array[swe_index].swe_use_id ==  HCI_USE_ID ) {
						hci_started = TRUE;
				}
#endif

			} else if(cur_swe->swe_type==RVM_SWE_TYPE_1) {	/* A-M-E-N-D-E-D! */
			
				/* call its init function */
				if (cur_swe->rvm_functions.start) {
					if (cur_swe->rvm_functions.start() != RVM_OK) {
						rvf_send_trace("RVM: Error Calling start function of swe nb ", 44, \
									(UINT32)swe_index, RV_TRACE_LEVEL_ERROR, RVM_USE_ID);
					}
				}
			}
		}

		/* increment the number of using swe and points to the using appli */
		/* DOES NOT DEPEND ON THE STATE */
		/*rvm_swe_array[swe_index].swe_get_info(&swe_info);
		switch( swe_info.swe_type) {
			case RVM_SWE_TYPE_1: 
				if(!swe_info.type_info.type1.nb_linked_swe) rvm_swe_array[swe_index].nb_using_appli=0;
			break;
			case RVM_SWE_TYPE_2:
				if(!swe_info.type_info.type2.nb_linked_swe) rvm_swe_array[swe_index].nb_using_appli=0;
			break;
			case RVM_SWE_TYPE_3:
				if(!swe_info.type_info.type3.nb_linked_swe) rvm_swe_array[swe_index].nb_using_appli=0;
			break;
			case RVM_SWE_TYPE_4:
				if(!swe_info.type_info.type4.nb_linked_swe) rvm_swe_array[swe_index].nb_using_appli=0;
			break;
			default: rvm_swe_array[swe_index].nb_using_appli=0;
		}*/

//		if(rvm_swe_array[swe_index].nb_using_appli) {
//		rvm_swe_array[swe_index].using_appli[rvm_swe_array[swe_index].nb_using_appli++] = appli->swe_id;
//
//		}

		if(rvm_swe_array[appli->swe_id].nb_using_appli<RVM_MAX_SWE_USING ) {
			rvm_swe_array[appli->swe_id].using_appli[rvm_swe_array[appli->swe_id].nb_using_appli++]=swe_index;
		} else {
			RVM_TRACE_WARNING_PARAM("RVM: Unable to track 'Using Appli' list is full nb=", appli->swe_id);
		}
		
		cur_swe = cur_swe->next_swe; /* process next SWE */
	}

	for(cur_swe=appli; cur_swe!=NULL; ) {
		rvm_swe_array[cur_swe->swe_id].swe_state = SWE_RUNNING;
		cur_swe = cur_swe->next_swe; 
	}

#ifdef _WINDOWS
	if (hci_started == TRUE) {
		rvf_delay(RVF_MS_TO_TICKS(1000));
	}
#endif

	return RVM_OK;
}


/*******************************************************************************
**
** Function         rvm_stop_swe_list
**
** Description      This function will call the stop functions when possible.
**
** Parameters:		T_RVM_PROCESSING_SWE * appli: list of required SWEs with their parameters.
**
** Returns          T_RVM_OK if all allocation are successful,
**                  else T_RVM_INTERNAL_ERR (then some SWE are not stopped.
**
*******************************************************************************/
T_RVM_RETURN rvm_stop_swe_list( T_RVM_PROCESSING_SWE * appli, T_RV_HDR* hdr)
{	
	T_RVM_PROCESSING_SWE * cur_swe = appli;
	T_RVM_INFO_SWE swe_info;
	volatile T_RVM_RETURN rvm_ret_value = RVM_OK;
	T_RVM_STOP_MSG* p_msg=NULL;
	UINT8 i=0;
	

	/* for each SWE in the list */
	while (cur_swe != NULL )
	{
		UINT8 swe_index = cur_swe->swe_id;

		/* If nb_using_appli > 1, SWE cannot be stopped	*/
/*		if (rvm_swe_array[swe_index].nb_using_appli > 1) {
			cur_swe = cur_swe->next_swe;
			continue;
		}
		// If nb_using_appli == 1 but using_appli != appli, SWE cannot be stopped 
		if ((rvm_swe_array[swe_index].nb_using_appli == 1) && \
			(rvm_swe_array[swe_index].using_appli[0] != appli->swe_id))	{
			cur_swe = cur_swe->next_swe;
			continue;
		}
*/
		if (cur_swe->swe_type==RVM_SWE_TYPE_1) { //cater for de-init of lib
			if(cur_swe->rvm_functions.stop1)cur_swe->rvm_functions.stop1();
			if(cur_swe->rvm_functions.kill)cur_swe->rvm_functions.kill();
			cur_swe = cur_swe->next_swe;
			continue;
		}
		if 	(cur_swe->swe_type==RVM_SWE_TYPE_4) { // etype 4 restriction
			RVM_TRACE_WARNING_PARAM("RVM: Stop & Kill is not applicable to Type 4 entities, nb=", (UINT32)swe_index);
			for (rvm_swe_array[swe_index].nb_using_appli=0,i=0; i<RVM_MAX_SWE_USING; i++) {  //reset using appli - workaround! 
				rvm_swe_array[swe_index].using_appli[i] = RVM_INVALID_SWE_INDEX;
			}
			cur_swe = cur_swe->next_swe;
			continue;
		}
		/* Retrieve stop function with a get_info */
		if (rvm_swe_array[swe_index].swe_get_info == NULL)
		{
			RVM_TRACE_WARNING_PARAM("RVM: SWE with no get info, cannot be stopped, nb=", (UINT32)swe_index);
			cur_swe = cur_swe->next_swe;
			rvm_ret_value = RVM_INTERNAL_ERR;
			continue;
		}
		rvm_swe_array[swe_index].swe_get_info( &swe_info);

		if (cur_swe->rvm_functions.stop == NULL) {
			RVM_TRACE_WARNING_PARAM("RVM: SWE with no stop function, cannot be stopped, nb=", (UINT32)swe_index);
			cur_swe = cur_swe->next_swe;
			continue;
		}

		if (rvf_get_buf( rvm_mem_bank, sizeof(T_RVM_STOP_MSG), (void **)&p_msg) == RVF_RED ) {
			RVM_TRACE_WARNING_PARAM("RVM: Unable to create STOP msg, nb=", (UINT32)swe_index);
			cur_swe = cur_swe->next_swe;
			continue;
		}

		p_msg->header.msg_id			= RVM_STOP_MSG;
		p_msg->header.src_addr_id		= hdr->src_addr_id; 
		p_msg->header.dest_addr_id		= hdr->dest_addr_id;
//		p_msg->header.callback_func		= hdr->callback_func; 
		p_msg->rp.callback_func			= ((T_RVM_STOP_MSG*)hdr)->rp.callback_func;
		p_msg->status					= SWE_STOPPING;
		p_msg->swe_num					= swe_index; //((T_RVM_STOP_MSG*)hdr)->swe_num;

		if ( rvf_send_msg( rvm_swe_array[swe_index].swe_addr_id, p_msg) != RVF_OK)	{
			rvm_ret_value = RVM_INTERNAL_ERR;
			cur_swe = cur_swe->next_swe;
			continue;
		}

		rvm_swe_array[swe_index].swe_state=SWE_STOPPING;

/*printf("SHUTDOWN: SWE %s nb %d USING APPLI= %d\n",rvm_swe_array[swe_index].swe_name, swe_index, rvm_swe_array[swe_index].nb_using_appli);
for(i=0; i<rvm_swe_array[swe_index].nb_using_appli; i++)printf(" %d, ", rvm_swe_array[swe_index].using_appli[i]);
printf("\n");*/

		for (rvm_swe_array[swe_index].nb_using_appli=0,i=0; i<RVM_MAX_SWE_USING; i++) {  //reset using appli - workaround! 
			rvm_swe_array[swe_index].using_appli[i] = RVM_INVALID_SWE_INDEX;
		}

/*printf("SHUTDOWN: SWE %s nb %d USING APPLI= %d\n",rvm_swe_array[swe_index].swe_name, swe_index, rvm_swe_array[swe_index].nb_using_appli);
for(i=0; i<rvm_swe_array[swe_index].nb_using_appli; i++)printf(" %d, ", rvm_swe_array[swe_index].using_appli[i]);
printf("\n");*/

		/* Stop SWE - amended to ASYNC	*/
		/* TO DO: for type 2 and 3 SWEs, send a message to the host to call the stop function */
		//cur_swe->rvm_functions.stop(NULL);

		/* Proceed to the next SWE	*/
		cur_swe = cur_swe->next_swe;
	}

	return rvm_ret_value;
}


/*******************************************************************************
**
** Function         rvm_suspend_swe_tasks
**
** Description      This function will suspend all SWE that are tasks.
**
** Parameters:		T_RVM_PROCESSING_SWE * appli: list of required SWEs with their parameters.
**
** Returns          T_RVM_OK if all allocation are successful,
**                  else T_RVM_INTERNAL_ERR (then some SWE are not stopped.
**
*******************************************************************************/
T_RVM_RETURN rvm_suspend_swe_tasks( T_RVM_PROCESSING_SWE * appli)
{	
	T_RVM_PROCESSING_SWE * cur_swe = appli;
	T_RVM_INFO_SWE swe_info;
	volatile T_RVM_RETURN rvm_ret_value = RVM_OK;
	
	/* for each SWE in the list */
	while (cur_swe != NULL )
	{
		UINT8 swe_index = cur_swe->swe_id;

		/* If nb_using_appli > 1, SWE cannot be stopped	*/
		if (rvm_swe_array[swe_index].nb_using_appli > 1)
		{
			cur_swe = cur_swe->next_swe;
			continue;
		}
		/* If nb_using_appli == 1 but using_appli != appli, SWE cannot be stopped */
		if ((rvm_swe_array[swe_index].nb_using_appli == 1) && \
			(rvm_swe_array[swe_index].using_appli[0] != appli->swe_id))
		{
			cur_swe = cur_swe->next_swe;
			continue;
		}

		/* Retrieve task info with a get_info */
		if (rvm_swe_array[swe_index].swe_get_info == NULL)
		{
			RVM_TRACE_WARNING_PARAM("RVM: SWE with no get info, cannot be stopped, nb=", (UINT32)swe_index);
			cur_swe = cur_swe->next_swe;
			rvm_ret_value = RVM_INTERNAL_ERR;
			continue;
		}
		rvm_swe_array[swe_index].swe_get_info( &swe_info);

		/* If SWE is not a task, continue */
		/* TO DO: manage group member SWEs */
		if ( (swe_info.swe_type == RVM_SWE_TYPE_1) ||
			 (swe_info.swe_type == RVM_SWE_TYPE_2) )
		{
			cur_swe = cur_swe->next_swe;
			continue;
		}

		/* Suspend SWE task	*/
		rvf_suspend_task( (UINT8)rvm_swe_array[swe_index].swe_return_path.addr_id);
		RVM_TRACE_DEBUG_LOW_PARAM("RVM: Suspended task nb ", (UINT32) (rvm_swe_array[swe_index].swe_return_path.addr_id & 0x000000FF) );

		/* Proceed to the next SWE */
		cur_swe = cur_swe->next_swe;
	}

	return rvm_ret_value;
}


/*******************************************************************************
**
** Function         rvm_kill_swe_list
**
** Description      This function will call the kill functions when possible.
**                  It will also delete the task, the stack and the used MBs.
**
** Parameters:		T_RVM_PROCESSING_SWE * appli: list of required SWEs with their parameters.
**
** Returns          T_RVM_OK if everything is successful,
**                  else T_RVM_INTERNAL_ERR (then some SWE are not killed).
**
*******************************************************************************/
T_RVM_RETURN rvm_kill_swe_list( T_RVM_PROCESSING_SWE * appli)
{	
	T_RVM_PROCESSING_SWE * cur_swe = appli;
	T_RVM_INFO_SWE swe_info;
	volatile T_RVM_RETURN rvm_ret_value = RVM_OK;
	
	/* for each SWE in the list */
	while (cur_swe != NULL )
	{
		UINT8 swe_index = cur_swe->swe_id;

		/* If nb_using_appli > 1, SWE cannot be killed */
		if (rvm_swe_array[swe_index].nb_using_appli > 1)
		{
			cur_swe = cur_swe->next_swe;
			continue;
		}

		/* If nb_using_appli == 1 but using_appli != appli, SWE cannot be killed */
		if ((rvm_swe_array[swe_index].nb_using_appli == 1) && \
			(rvm_swe_array[swe_index].using_appli[0] != appli->swe_id))
		{
			cur_swe = cur_swe->next_swe;
			continue;
		}

		/* Retrieve kill function with a get_info */
		if (rvm_swe_array[swe_index].swe_get_info == NULL)
		{
			RVM_TRACE_WARNING_PARAM("RVM: SWE with no get info, cannot be killed, nb=", (UINT32)swe_index);
			cur_swe = cur_swe->next_swe;
			rvm_ret_value = RVM_INTERNAL_ERR;
			continue;
		}
		rvm_swe_array[swe_index].swe_get_info( &swe_info);

		if (cur_swe->rvm_functions.kill == NULL)
		{
			RVM_TRACE_WARNING_PARAM("RVM: SWE with no kill function, cannot be killed, nb=", (UINT32)swe_index);
			cur_swe = cur_swe->next_swe;
			rvm_ret_value = RVM_INTERNAL_ERR;
			continue;
		}

		/* Kill SWE	*/
		cur_swe->rvm_functions.kill();
		
		/* TO DO: manage group member SWEs */
		/* If the SWE is a task, the task should be deleted, as well as its stack */
		if ( (swe_info.swe_type == RVM_SWE_TYPE_3) ||
			 (swe_info.swe_type == RVM_SWE_TYPE_4) )
		{
			rvf_exit_task((UINT8)(rvm_swe_array[swe_index].swe_return_path.addr_id));
 			rvf_free_buf(rvm_swe_array[swe_index].stack_ptr);
			RVM_TRACE_DEBUG_LOW_PARAM("RVM: Deleted task nb ", (UINT32)(rvm_swe_array[swe_index].swe_return_path.addr_id & 0x000000FF));
			rvf_free_sys_resources(rvm_swe_array[swe_index].swe_addr_id, 2);

		} else if(swe_info.swe_type == RVM_SWE_TYPE_2) {
			rvf_free_sys_resources(rvm_swe_array[swe_index].swe_addr_id, 0);
		}

		/* Proceed to the next SWE	*/
		cur_swe = cur_swe->next_swe;
	}

	return rvm_ret_value;
}


/*******************************************************************************
**
** Function         rvm_launch_appli
**
** Description      Called by the main RVM task to start a specified known application
**
** Parameters:		T_RVM_MSG msg: containing the return path and the index of the 
**					application to start in the array of known SWEs.
**
** Returns          None
**
*******************************************************************************/
void rvm_launch_appli( T_RVM_MSG * msg_Ptr) {
	T_RVM_GROUP_DIRECTIVE GroupDirectives[MAX_GRPS]; 
	UINT8 gdCount=0;
	T_RVM_PROCESSING_SWE	* appli = NULL; /* pointer to the first element of the list */
	T_RV_RETURN_PATH		appli_return_path;
	UINT8	i,j=0;

	for(i=0; i<MAX_GRPS; i++) {
		GroupDirectives[i].group_directive=0;
		GroupDirectives[i].host_task_addr=0;
		GroupDirectives[i].stack_size=0;
		memset(&GroupDirectives[i].hosted_swe_db_index, 0, (sizeof(UINT8)*MAX_COMPOSITES));
	}

	/* store the return path of the caller */
	appli_return_path.callback_func	= msg_Ptr->rp.callback_func;
	appli_return_path.addr_id		= msg_Ptr->header.src_addr_id;
	
	/* recursively call all get_info functions and build the list of running swe */	
	if ( rvm_build_swe_list( &appli, msg_Ptr->swe_num, 0) != RVM_OK )
	{	
		/* Display error message
		   error case: use the return_path to inform the caller that an error occurs*/
		rvm_snd_msg_to_upper(RVM_START_APPLI, RVM_INVALID_PARAMETER, msg_Ptr->swe_num, appli_return_path);
		RVM_TRACE_ERROR("RVM: SWE list built error");
		return;
	}

	gdCount=_resolve_t2_grouping(appli, GroupDirectives);

	if(!appli) {
		// error case: use return_path to inform the caller about memory lack 
		// Unlock state of SWE and free memory	
		RVM_TRACE_WARNING_PARAM("RVM: ABORTED, Stand-alone ENTITY start request!", (UINT32)msg_Ptr->swe_num);
		rvm_snd_msg_to_upper(RVM_START_APPLI, RVM_NOT_READY, msg_Ptr->swe_num, appli_return_path);
		rvm_delete_used_memory (appli);
		return;
	}

	RVM_TRACE_DEBUG_HIGH("RVM: SWE list built success");
	RVM_TRACE_DEBUG_HIGH_PARAM("RVM: trying to launch SWE", rvm_swe_array[appli->swe_id].swe_use_id);
	
	/* check if there is enough available memory */
	if ( rvm_verify_memory_requirement( appli, GroupDirectives, gdCount) != RVM_OK)
	{
		/* error case: use return_path to inform the caller about memory lack */
		/* Unlock state of SWE and free memory	*/
		RVM_TRACE_WARNING_PARAM("RVM: SWE not enough memory: unable to launch Appli nb", (UINT32)appli->swe_id);
		rvm_snd_msg_to_upper(RVM_START_APPLI, RVM_MEMORY_ERR, msg_Ptr->swe_num, appli_return_path);
		rvm_delete_used_memory (appli);
		return;
	}

	/* allocates memory banks */
	 if ( rvm_allocate_mb( appli) != RVM_OK )
	 {	/* error case: use return_path to inform the caller about memory lack */
		rvm_delete_used_memory (appli);
		rvm_snd_msg_to_upper(RVM_START_APPLI, RVM_MEMORY_ERR, msg_Ptr->swe_num, appli_return_path);
		RVM_TRACE_WARNING("RVM: SWE memory bank allocation error - launch aborted!");
		return;
	 }
	 RVM_TRACE_DEBUG_LOW("RVM: SWE memory bank allocation success");

	/* call set_info function for each SWE */
	 if ( rvm_set_swe_info( appli) != RVM_OK)
	 {	/* error case: use return_path to inform the caller that an error occurs */
		RVM_TRACE_WARNING("RVM: SWE set info functions error");
		_fatal(appli, 0);
		rvm_delete_created_mb(appli);
		rvm_delete_used_memory (appli);
		rvm_snd_msg_to_upper(RVM_START_APPLI, RVM_INTERNAL_ERR, msg_Ptr->swe_num, appli_return_path);
		return;
	 }
	 RVM_TRACE_DEBUG_LOW("RVM: SWE set info functions called");


	 /* call the init and start functions */
	 if ( rvm_initialize_swe( appli, GroupDirectives, gdCount) != RVM_OK)
	 {	/* error case: use return_path to inform the caller that an error occurs */
		RVM_TRACE_WARNING("RVM: SWE initialization error");
		rvm_snd_msg_to_upper(RVM_START_APPLI, RVM_INTERNAL_ERR, msg_Ptr->swe_num, appli_return_path);
		_fatal(appli, 2);
		rvm_delete_created_mb(appli);
		rvm_delete_used_memory (appli);
		return;
	 }
	 RVM_TRACE_DEBUG_LOW("RVM: SWE initialization success");

	 /* build a message and send the response to the caller */
	 /* send a result using the return_path */
	 rvm_snd_msg_to_upper(RVM_START_APPLI, RVM_OK, msg_Ptr->swe_num, appli_return_path);
	

	 /* and store the return_path */
	 rvm_swe_array[ msg_Ptr->swe_num ].mmi_return_path.callback_func = msg_Ptr->rp.callback_func;
	 rvm_swe_array[ msg_Ptr->swe_num ].mmi_return_path.addr_id = msg_Ptr->header.src_addr_id;
	 	
	 /* Once Everything is back in stand-by, release used memory */
	 rvm_delete_used_memory (appli);
}


/*******************************************************************************
**
** Function         rvm_shut_down_appli
**
** Description      Called by the main RVM task to stop a specified known application
**
** Parameters:		T_RVM_MSG msg: containing the return path and the index of the 
**					application to stop in the array of known SWEs.
**
** Returns          None
**
*******************************************************************************/
void rvm_stop_appli( T_RVM_STOP_MSG* msg_Ptr) {
	T_RVM_PROCESSING_SWE	* appli = NULL; /* pointer to the first element of the list */
	T_RVM_RETURN		ret_value;
	UINT8				swe_idx = 200;
	T_RV_RETURN_PATH	appli_return_path;

	appli_return_path.callback_func = msg_Ptr->rp.callback_func;
	appli_return_path.addr_id		= msg_Ptr->header.src_addr_id;
	

	RVM_TRACE_DEBUG_HIGH_PARAM("RVM: trying to stop Appli nb ", (UINT32)swe_idx);
	
	if (rvm_swe_array[msg_Ptr->swe_num].nb_using_appli > 1) {
		RVM_TRACE_WARNING_PARAM("RVM: SWE has dependencies, nb=", (UINT32)msg_Ptr->swe_num);
		return;
	}
	// ??? If nb_using_appli == 1 but using_appli != appli, SWE cannot be stopped 
	if ((rvm_swe_array[msg_Ptr->swe_num].nb_using_appli == 1) && \
		(rvm_swe_array[msg_Ptr->swe_num].using_appli[0] != msg_Ptr->swe_num))	{
		RVM_TRACE_WARNING_PARAM("RVM: SWE has dependencies, nb=", (UINT32)msg_Ptr->swe_num);
		return;
	}


	/* TO DO : REBUILD SWE LIST !!!! */
	if ( rvm_build_swe_list( &appli, msg_Ptr->swe_num, 1) != RVM_OK )
	{	
		/* Display error message
		   error case: use the return_path to inform the caller that an error occurs*/
		rvm_snd_msg_to_upper(RVM_START_APPLI, RVM_INVALID_PARAMETER, msg_Ptr->swe_num, appli_return_path);
		RVM_TRACE_ERROR("RVM: SWE list built error");
		return;
	}

	/* Stop all swe in the list that are used only once	*/
	if ((ret_value = rvm_stop_swe_list(appli, (T_RV_HDR*)msg_Ptr)) != RVM_OK )
	{
		/* Display error message
		   TO DO: error case: use the return_path to inform the caller that an error occurs	*/
		RVM_TRACE_WARNING_PARAM("RVM: Error in SWE stop", (UINT32)ret_value);
		return;
	}

	rvm_delete_used_memory (appli);

	RVM_TRACE_DEBUG_LOW("RVM: SWE stop broadcast!");

}

// NOTE: presently no timeout exists, if the ENT. fails to reply 
// to stop with rvm_swe_stopped() RVM doesn't kill it.
void rvm_swe_has_stopped(T_RVM_STOP_MSG* msg) {
	T_RVM_STOP_MSG* p_msg=(T_RVM_STOP_MSG*)msg;
	T_RV_RETURN_PATH	appli_return_path;

	appli_return_path.callback_func = msg->rp.callback_func;
	appli_return_path.addr_id		= msg->header.src_addr_id;

	if(msg->status!=SWE_STOPPING) {
		// inform upper of problem
		rvm_snd_msg_to_upper(RVM_STOP_APPLI, RVM_INVALID_PARAMETER, msg->swe_num, appli_return_path);
		RVM_TRACE_ERROR("RVM: Entity declines STOP REQ");
		rvf_free_msg((T_RV_HDR*)msg);
		return;
	}

	// cont. with shutdown - MUST DO ERROR CASE !
	rvm_shutdown_swe(p_msg->swe_num);

	// set stopped status
	rvm_swe_array[p_msg->swe_num].swe_state=SWE_KILLED; //SWE_STOPPING;

	/* build a message and send the response to the caller */
	/* send a result using the return_path */
	if(rvm_get_mb_level(p_msg->swe_num) ){
		rvm_snd_msg_to_upper(RVM_STOP_APPLI, RV_MEMORY_REMAINING, msg->swe_num, appli_return_path);
	} else {
		rvm_snd_msg_to_upper(RVM_STOP_APPLI, RVM_OK, msg->swe_num, appli_return_path);
	}
	
	/* and store the return_path */
	rvm_swe_array[ msg->swe_num ].mmi_return_path.callback_func = msg->rp.callback_func;
	rvm_swe_array[ msg->swe_num ].mmi_return_path.addr_id		= msg->header.src_addr_id;

}

void rvm_shutdown_swe(UINT8 index) { //should ret. ok or fail
	rvm_suspend_swe(index);
	rvm_kill_swe(index);
}

void rvm_suspend_swe(UINT8 swe_index) {
		volatile T_RVM_RETURN rvm_ret_value = RVM_OK;
		T_RVM_INFO_SWE swe_info;

		/* ??? If nb_using_appli > 1, SWE cannot be stopped	
		if (rvm_swe_array[swe_index].nb_using_appli > 1) {
			RVM_TRACE_WARNING_PARAM("RVM-SUSPEND: SWE has dependencies, nb=", (UINT32)swe_index);
		}
		// ??? If nb_using_appli == 1 but using_appli != appli, SWE cannot be stopped 
		if ((rvm_swe_array[swe_index].nb_using_appli == 1) && \
			(rvm_swe_array[swe_index].using_appli[0] != swe_index))	{
			RVM_TRACE_WARNING_PARAM("RVM-SUSPEND: SWE has dependencies, nb=", (UINT32)swe_index);
		}*/

		/* Retrieve task info with a get_info */
		if (rvm_swe_array[swe_index].swe_get_info == NULL) {
			RVM_TRACE_WARNING_PARAM("RVM: SWE with no get info, cannot be stopped, nb=", (UINT32)swe_index);
			rvm_ret_value = RVM_INTERNAL_ERR;
			return;
		}
		rvm_swe_array[swe_index].swe_get_info( &swe_info);

		/* If SWE is not a task, continue */
		/* TO DO: manage group member SWEs */
		if ( (swe_info.swe_type == RVM_SWE_TYPE_1) ||
			 (swe_info.swe_type == RVM_SWE_TYPE_2) ) {
			return;
		}
	
		/* Suspend SWE task	*/
		rvf_suspend_task( (UINT8)rvm_swe_array[swe_index].swe_return_path.addr_id);
		RVM_TRACE_DEBUG_LOW_PARAM("RVM: Suspended task nb ", (UINT32) (rvm_swe_array[swe_index].swe_return_path.addr_id & 0x000000FF) );
}

T_RVM_RETURN rvm_kill_swe(UINT8 swe_index) {
		T_RVM_INFO_SWE swe_info;
		volatile T_RVM_RETURN rvm_ret_value = RVM_OK;
		UINT8 isVirtual=0;
		T_RVF_G_ADDR_ID	gid=RVF_INVALID_ADDR_ID;
		UINT8 isIdle=0;
		UINT8 i=0;

		/* If nb_using_appli > 1, SWE cannot be killed 
		if (rvm_swe_array[swe_index].nb_using_appli > 1) return rvm_ret_value;

		// If nb_using_appli == 1 but using_appli != appli, SWE cannot be killed 
		if ((rvm_swe_array[swe_index].nb_using_appli == 1) && \
			(rvm_swe_array[swe_index].using_appli[0] != swe_index)) {
			RVM_TRACE_WARNING_PARAM("RVM-KILL: SWE has dependencies, nb=", (UINT32)swe_index);
			return rvm_ret_value;
		}*/

		/* Retrieve kill function with a get_info */
		if (rvm_swe_array[swe_index].swe_get_info == NULL){
			RVM_TRACE_WARNING_PARAM("RVM-KILL: SWE has no kill function defined, nb=", (UINT32)swe_index);
			rvm_ret_value = RVM_INTERNAL_ERR;
		}

		rvm_swe_array[swe_index].swe_get_info(&swe_info);
		switch( swe_info.swe_type) {
			case RVM_SWE_TYPE_1: 
//				if(swe_info.type_info.type1.kill) swe_info.type_info.type1.kill() ;
				isVirtual=1;
			break;
			case RVM_SWE_TYPE_2:
				gid=resolveHostAddrId(rvm_swe_array[swe_index].swe_addr_id);
				rvf_unregisterFromHost(gid, rvm_swe_array[swe_index].swe_addr_id);
				rvf_isHostingTaskIdle(gid, &isIdle);
				if(isIdle) { 	// Defered suspend of hosting task: 
					rvf_suspend_task(gid);
					rvf_exit_task(gid);
					rvf_free_sys_resources(gid, 2);
				}
				if(swe_info.type_info.type2.kill) swe_info.type_info.type2.kill();
				isVirtual=1;
			break;
			case RVM_SWE_TYPE_3:
				if(swe_info.type_info.type3.kill) swe_info.type_info.type3.kill();
			break;
			case RVM_SWE_TYPE_4:
				if(swe_info.type_info.type4.kill) swe_info.type_info.type4.kill();
			break;
			default: 
				RVM_TRACE_WARNING_PARAM("RVM: SWE with no kill function, cannot be killed, nb=", (UINT32)swe_index);
		}

		if(!isVirtual) {
			rvf_exit_task((UINT8)(rvm_swe_array[swe_index].swe_return_path.addr_id));
			rvf_free_buf(rvm_swe_array[swe_index].stack_ptr);
			RVM_TRACE_DEBUG_LOW_PARAM("RVM: Deleted task nb ", (UINT32)(rvm_swe_array[swe_index].swe_return_path.addr_id & 0x000000FF));
			rvf_free_sys_resources(rvm_swe_array[swe_index].swe_addr_id, 2);
		} else {
			rvf_free_sys_resources(rvm_swe_array[swe_index].swe_addr_id, 0);
		}

	return rvm_ret_value;
}

UINT8 rvm_get_mb_level(UINT8 swe_index) {
	T_RVM_INFO_SWE swe_info;
	INT8 i=0;
	UINT8 isUsed=0;
	
	rvm_swe_array[swe_index].swe_get_info(&swe_info);
		switch( swe_info.swe_type) {
			case RVM_SWE_TYPE_1: 
				if(swe_info.type_info.type1.nb_mem_bank!=0)
					for(i=0; i<swe_info.type_info.type1.nb_mem_bank; i++) {
						rvf_mb_is_used(swe_info.type_info.type1.mem_bank[i].bank_name, &isUsed);
						if(isUsed)	return isUsed;
					}
			return isUsed;
			case RVM_SWE_TYPE_2:
				if(swe_info.type_info.type2.nb_mem_bank!=0)
					for(i=0; i<swe_info.type_info.type2.nb_mem_bank; i++) {
						rvf_mb_is_used(swe_info.type_info.type2.mem_bank[i].bank_name, &isUsed);
						if(isUsed)	return isUsed;
					}
					return isUsed;
			case RVM_SWE_TYPE_3:
				if(swe_info.type_info.type3.nb_mem_bank!=0)
					for(i=0; i<swe_info.type_info.type3.nb_mem_bank; i++) {
						rvf_mb_is_used(swe_info.type_info.type3.mem_bank[i].bank_name, &isUsed);
						if(isUsed)	return isUsed;
					}
			return isUsed;
			case RVM_SWE_TYPE_4:
				if(swe_info.type_info.type4.nb_mem_bank!=0)
					for(i=0; i<swe_info.type_info.type4.nb_mem_bank; i++) {
						rvf_mb_is_used(swe_info.type_info.type4.mem_bank[i].bank_name, &isUsed);
						if(isUsed)	return isUsed;
					}
			return isUsed;
			default: RVM_TRACE_DEBUG_LOW("RVM: Error rvm_get_mb_level()");
			return isUsed;
		}
}

void rvm_shut_down_appli( T_RVM_MSG * msg_Ptr) {
	T_RVM_PROCESSING_SWE	* appli = NULL; /* pointer to the first element of the list */
	T_RVM_RETURN		ret_value;
	UINT8				swe_idx = 200;
	T_RV_RETURN_PATH	appli_return_path;

	
	appli_return_path.callback_func = msg_Ptr->rp.callback_func;
	appli_return_path.addr_id		= msg_Ptr->header.src_addr_id;


	RVM_TRACE_DEBUG_HIGH_PARAM("RVM: trying to stop Appli nb ", (UINT32)swe_idx);
	
	/* TO DO : REBUILD SWE LIST !!!! */
	if ( rvm_build_swe_list( &appli, msg_Ptr->swe_num, 1) != RVM_OK )
	{	
		/* Display error message
		   error case: use the return_path to inform the caller that an error occurs*/
		rvm_snd_msg_to_upper(RVM_START_APPLI, RVM_INVALID_PARAMETER, msg_Ptr->swe_num, appli_return_path);
		RVM_TRACE_ERROR("RVM: SWE list built error");
		return;
	}

	/* Stop all swe in the list that are used only once	*/
	if ((ret_value = rvm_stop_swe_list(appli, (T_RV_HDR*)msg_Ptr)) != RVM_OK )
	{
		/* Display error message
		   TO DO: error case: use the return_path to inform the caller that an error occurs	*/
		RVM_TRACE_WARNING_PARAM("RVM: Error in SWE stop", (UINT32)ret_value);
		return;
	}
	RVM_TRACE_DEBUG_LOW("RVM: SWE stop success");


	/* Suspend all swe that are tasks */
	if ((ret_value = rvm_suspend_swe_tasks(appli)) != RVM_OK )
	{
		/* Display error message
		   TO DO: error case: use the return_path to inform the caller that an error occurs	*/
		RVM_TRACE_WARNING_PARAM("RVM: Error in tasks suspension", (UINT32)ret_value);
		return;
	}
	RVM_TRACE_DEBUG_LOW("RVM: SWE task supsended");

	/* Kill all SWEs */
	if ((ret_value = rvm_kill_swe_list(appli)) != RVM_OK)
	{
		/* Display error message
		   TO DO: error case: use the return_path to inform the caller that an error occurs	*/
		RVM_TRACE_WARNING_PARAM("RVM: Error in SWE killing", (UINT32)ret_value);
		return;
	}
	RVM_TRACE_DEBUG_LOW("RVM: SWE kill success");

	
	/* Delete the swe Memory Banks */
	rvm_delete_created_mb(appli);

	/* Delete memory used and restore NOT_STARTED states */
	if ((ret_value = rvm_clean_env(appli)) != RVM_OK)
	{
		/* Display error message
		   TO DO: error case: use the return_path to inform the caller that an error occurs */
		RVM_TRACE_WARNING_PARAM("RVM: Error in Memory cleaning", (UINT32)ret_value);
		return;
	}
	RVM_TRACE_DEBUG_LOW("RVM: Memory cleaning success");


	/* build a message and send the response to the caller */
	/* send a result using the return_path */
	rvm_snd_msg_to_upper(RVM_STOP_APPLI, RVM_OK, msg_Ptr->swe_num, appli_return_path);

	
	/* and store the return_path */
	rvm_swe_array[ msg_Ptr->swe_num ].mmi_return_path.callback_func = msg_Ptr->rp.callback_func;
	rvm_swe_array[ msg_Ptr->swe_num ].mmi_return_path.addr_id		= msg_Ptr->header.src_addr_id;
}

T_RVM_RETURN _fatal( T_RVM_PROCESSING_SWE * appli, UINT8 rm) {
	T_RVM_PROCESSING_SWE * cur_swe =  NULL;
//	T_RVM_INFO_SWE swe_info;
	
	RVM_TRACE_DEBUG_LOW("RVM: Fatality handler: reclaiming system resources!");
	/* free all appli's system resources */
	for (cur_swe = appli; cur_swe!=NULL; ) {
		if(rvm_swe_array[cur_swe->swe_id].swe_state!=SWE_RUNNING)
			rvf_free_sys_resources(rvm_swe_array[cur_swe->swe_id].swe_addr_id, rm);
	}
	

	return RVM_OK;
}

/*******************************************************************************
**
** Function         rvm_generic_swe_core
**
** Description      This is the main task core used for GROUP_MEMBER SWEs hosting
**					and for SINGLE SWEs.
**
** Parameters:		useless, may be for future evolutions if Nucleus really 
**					supports it.
**
** Returns          None
**
*******************************************************************************/
T_RVM_RETURN rvm_generic_swe_core(void)
{
   return RVM_OK;
}

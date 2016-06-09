/****************************************************************************/
/*                                                                          */
/*  File Name:	ffs_env.c						    */
/*                                                                          */
/*  Purpose:	This file contains definitions for RV manager related	    */
/*		functions used to get info, start and stop the FFS SWE.	    */
/*                                                                          */
/*  Version	0.1							    */
/*									    */
/*  Date       	Modification						    */
/*  ------------------------------------				    */
/*  10/24/2000	Create							    */
/*									    */
/*  Author	Pascal Puel						    */
/*									    */
/* (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../include/config.h"
#include "ffs.h"
#include "ffs_env.h" 
#include "../../riviera/rvm/rvm_gen.h"
#include "../../riviera/rvm/rvm_priorities.h"
#include "../../riviera/rvm/rvm_use_id_list.h"
#include "task.h" 
#include <string.h>

extern void ffs_task_init(T_RVF_MB_ID mbid, T_RVF_ADDR_ID  addr_id);
T_FFS_TASK_INFO ffs_task_info;

/* global pointer to the error function */
static T_RVM_RETURN (*ffs_error_ft)(T_RVM_NAME swe_name,
				    T_RVM_RETURN error_cause,
				    T_RVM_ERROR_TYPE error_type,
				    T_RVM_STRING error_msg);

/******************************************************************************
* Function	  : ffs_get_info
*
* Description : This function is called by the RV manager to learn 
*				driver requirements in terms of memory, SWEs...
*
* Parameters  : T_RVM_INFO_SWE  * swe_info: pointer to the structure to fill
*				containing infos related to the driver SWE.
*
* Return      :  T_RVM_RETURN
* 
* History	  : 0.1 (20-August-2000)
*									
*
******************************************************************************/
T_RVM_RETURN ffs_get_info(T_RVM_INFO_SWE  * infoSWE)
{

	/* SWE info */
	infoSWE->swe_type = RVM_SWE_TYPE_4;

	infoSWE->type_info.type4.swe_use_id	= FFS_USE_ID;
	memcpy ( (UINT8 *) infoSWE->type_info.type4.swe_name, "FFS", sizeof("FFS") );
	infoSWE->type_info.type4.version    = 0;

	infoSWE->type_info.type4.stack_size = FFS_STACK_SIZE;
	infoSWE->type_info.type4.priority   = RVM_FFS_TASK_PRIORITY;
	
	/* memory bank info */
	infoSWE->type_info.type4.nb_mem_bank = 1;
	
	memcpy ((UINT8 *) infoSWE->type_info.type4.mem_bank[0].bank_name, "FFS_PRIM", RVM_NAME_MAX_LEN);
	infoSWE->type_info.type4.mem_bank[0].initial_params.size          = FFS_MB_PRIM_SIZE;
	infoSWE->type_info.type4.mem_bank[0].initial_params.watermark     = FFS_MB_PRIM_WATERMARK;

	/* linked SWE info */
	infoSWE->type_info.type4.nb_linked_swe = 0;

	/* generic functions */
	infoSWE->type_info.type4.set_info = ffs_set_info;
	infoSWE->type_info.type4.init     = ffs_init;
	infoSWE->type_info.type4.core     = ffs_start;
	infoSWE->type_info.type4.stop     = ffs_stop;
	infoSWE->type_info.type4.kill     = ffs_kill;

	/* Set return_path */
	infoSWE->type_info.type4.return_path.callback_func = NULL;
	infoSWE->type_info.type4.return_path.addr_id       = 0;

	return RV_OK;
}


/******************************************************************************
* Function	  : ffs_set_info
*
* Description : This function is called by the RV manager to inform  
*		the driver SWE about task_id, mb_id and error function.
*
* Parameters  : - T_RVF_ADDR_ID addr_id: unique path to the SWE.
*		- T_RV_RETURN ReturnPath[], array of return path for linked SWE
*		- T_RVF_MB_ID mbId[]: array of memory bank ids.
*		- callback function to call in case of unrecoverable error.
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1 (20-August-2000)
*									
*
******************************************************************************/
T_RVM_RETURN ffs_set_info(T_RVF_ADDR_ID  addr_id,
			  T_RV_RETURN		ReturnPath[],
			  T_RVF_MB_ID mbId[],
			  T_RVM_RETURN (*callBackFct)(T_RVM_NAME SWEntName,
						    T_RVM_RETURN errorCause,
						    T_RVM_ERROR_TYPE errorType,
						    T_RVM_STRING errorMsg))
{
	/* store the pointer to the error function */
	ffs_error_ft = callBackFct ;

	ffs_task_init(mbId[0], addr_id);

	ffs_task_info.addr_id = addr_id;
	ffs_task_info.mbid = mbId[0];

	return RV_OK;
}


/******************************************************************************
* Function	  : ffs_init
*
* Description : This function is called by the RV manager to initialize the 
*		ffs SWE before creating the task and calling ffs_start. 
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1 (20-August-2000)
*									
*
******************************************************************************/
T_RVM_RETURN ffs_init(void)
{
	return RV_OK;
}


/******************************************************************************
* Function	  : ffs_start
*
* Description : This function is called by the RV manager to start the ffs
*		SWE, it is the body of the task.
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1 (20-August-2000)
*									
*
******************************************************************************/
T_RVM_RETURN ffs_start(void)
{
    ffs_task();
    return RV_OK;
}


/******************************************************************************
* Function	  : ffs_stop
*
* Description : This function is called by the RV manager to stop the ffs SWE.
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1 (20-August-2000)
*									
*
******************************************************************************/
T_RVM_RETURN ffs_stop(void)
{
	/* other SWEs have not been killed yet, ffs can send messages to other SWEs */

	return RV_OK;
}


/******************************************************************************
* Function	  : ffs_kill
*
* Description : This function is called by the RV manager to kill the ffs 
*		SWE, after the ffs_stop function has been called.
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1 (20-August-2000)
*									
*
******************************************************************************/
T_RVM_RETURN ffs_kill (void)
{
	/* free all memory buffer previously allocated */
	return RV_OK;
}

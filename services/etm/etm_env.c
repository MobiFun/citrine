/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	etm_env.c 
 *
 * @author	Kim T. Peteren (ktp@ti.com)
 * @version 0.1
 *

 *
 * History:
 *
 * 	Date       	Modification
 *  ------------------------------------
 *  16/06/2003	Creation
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/

#include "etm_config.h"
#include "etm_env.h"

#include "../../riviera/rvm/rvm_priorities.h"
#include "../../riviera/rvm/rvm_use_id_list.h"
#include "../../riviera/rv/rv_defined_swe.h"

#include <string.h>
/******************************************************************************
 * 
 *****************************************************************************/

/* External declaration */
extern T_RV_RET etm_task(void);


/**
 * Pointer on the structure gathering all the global variables
 * used by ETM instance.
 */
T_ETM_ENV_CTRL_BLK *etm_env_ctrl_blk = NULL;


/******************************************************************************
* Function	  : etm_get_info
*
* Description : This function is called by the RV manager to learn 
*				driver requirements in terms of memory, SWEs...
*
* Parameters  : T_RVM_INFO_SWE  * swe_info: pointer to the structure to fill
*				containing infos related to the driver SWE.
*
* Return      :  T_RVM_RETURN
* 
* History	  : 0.1 
*									
*
******************************************************************************/
T_RVM_RETURN etm_get_info(T_RVM_INFO_SWE *swe_info)
{
    /* SWE info */
    swe_info->swe_type = RVM_SWE_TYPE_4;
    swe_info->type_info.type4.swe_use_id = ETM_USE_ID;

    memcpy(swe_info->type_info.type4.swe_name, "ETM", sizeof("ETM"));
    swe_info->type_info.type4.stack_size = ETM_STACK_SIZE;
    swe_info->type_info.type4.priority   = RVM_ETM_TASK_PRIORITY;

    /* Set the return path */
    swe_info->type_info.type4.return_path.callback_func	= NULL;
    swe_info->type_info.type4.return_path.addr_id	    = 0;

    /* memory bank info */
    swe_info->type_info.type4.nb_mem_bank = 1;

    memcpy (swe_info->type_info.type4.mem_bank[0].bank_name, "ETM_PRIM", 9);
    swe_info->type_info.type4.mem_bank[0].initial_params.size          = ETM_MB_PRIM_SIZE;
    swe_info->type_info.type4.mem_bank[0].initial_params.watermark     = ETM_MB_PRIM_WATERMARK;

    /* linked SWE info */
    /* this SWE requires the ATP SWE to run */
#if ETM_ATP_SUPPORT
    swe_info->type_info.type4.nb_linked_swe = 1;
    swe_info->type_info.type4.linked_swe_id[0] = ATP_USE_ID;
#else
    swe_info->type_info.type4.nb_linked_swe = 0;
#endif

    /* generic functions */
    swe_info->type_info.type4.set_info = etm_set_info;
    swe_info->type_info.type4.init     = etm_init;
    swe_info->type_info.type4.core     = etm_start;
    swe_info->type_info.type4.stop     = etm_stop;
    swe_info->type_info.type4.kill     = etm_kill;

    return RVM_OK;
}


/******************************************************************************
* Function	  : etm_set_info
*
* Description : This function is called by the RV manager to inform  
*		the driver SWE about task_id, mb_id and error function.
*
* Parameters  : - T_RVF_ADDR_ID  addr_id: unique path to the SWE.
*		- T_RV_RETURN ReturnPath[], array of return path for linked SWE
*		- T_RVF_MB_ID mbId[]: array of memory bank ids.
*		- callback function to call in case of unrecoverable error.
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1
*									
*
******************************************************************************/

T_RVM_RETURN etm_set_info (T_RVF_ADDR_ID	addr_id, 
                           T_RV_RETURN		return_path[], 
                           T_RVF_MB_ID		bk_id[],
                           T_RVM_RETURN	(*rvm_error_ft)(T_RVM_NAME	swe_name, 
                                                        T_RVM_RETURN	error_cause, 
                                                        T_RVM_ERROR_TYPE error_type, 
                                                        T_RVM_STRING	error_msg))
{
    /* Create instance gathering all the variable used by EXPL instance */
    if (rvf_get_buf(bk_id[0], 
		    sizeof(T_ETM_ENV_CTRL_BLK),
		    (T_RVF_BUFFER**)&etm_env_ctrl_blk) != RVF_GREEN)
    {
	/* The environemnt will cancel the ETM instance creation. */
	return RVM_MEMORY_ERR;
    }

    /* Store the pointer to the error function */
    etm_env_ctrl_blk->error_ft = rvm_error_ft ;
    /* Store the mem bank id. */
    etm_env_ctrl_blk->prim_id = bk_id[0];
    /* Store the addr id */
    etm_env_ctrl_blk->addr_id = addr_id;

    /*
     * Task ID (task_id) and Memory bank ID (mb_id) can be retrieved later 
     * using rvf_get_taskid and rvf_get_mb_id functions.
     */

    /* return_path of linked SWE -> not used */

    return RVM_OK;
}


/******************************************************************************
* Function	  : etm_init
*
* Description : This function is called by the RV manager to initialize the 
*		etm SWE before creating the task and calling etm_start. 
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1 (20-August-2000)
*									
*
******************************************************************************/

T_RVM_RETURN etm_init(void)
{
    return RVM_OK;
}


/******************************************************************************
* Function	  : etm_start
*
* Description : This function is called by the RV manager to start the etm
*		SWE, it is the body of the task.
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1 
*									
*
******************************************************************************/

T_RVM_RETURN etm_start(void)
{
    etm_task();
    return RV_OK;
}


/******************************************************************************
* Function	  : etm_stop
*
* Description : This function is called by the RV manager to stop the etm SWE.
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1
*
******************************************************************************/

T_RVM_RETURN etm_stop(void)
{
	return RVM_OK;
}

/******************************************************************************
* Function	  : etm_kill
*
* Description : This function is called by the RV manager to kill the etm 
*		SWE, after the etm_stop function has been called.
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1 
*									
*
******************************************************************************/

T_RVM_RETURN etm_kill (void)
{
	rvf_free_buf(etm_env_ctrl_blk);
	return RVM_OK;
}

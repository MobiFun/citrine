/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	etm_env.h 
 *
 * Declarations of the Riviera Generic Functions 
 * (except handle message and handle timer).
 *
 * @author	Laurent Sollier (l-sollier@ti.com) and
 *		Kim T. Peteren (ktp@ti.com)
 * @version 0.1
 *

 *
 * History:
 *
 * 	Date       	Modification
 *  ------------------------------------
 *  10/24/2001	Creation
 *
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/


#ifndef _ETM_ENV_H_
#define _ETM_ENV_H_

#include "../../riviera/rvm/rvm_gen.h"
#include "etm_pool_size.h"	/* Stack & Memory Bank sizes definitions */

/**
 * Mailbox ID used by the SWE.
 */
#define ETM_MAILBOX    RVF_TASK_MBOX_0

/**
 * @name Mem bank
 * Memory bank size and watermark.
 */
/*@{*/
#define ETM_MB_PRIM_SIZE	        ETM_MB1_SIZE
#define ETM_MB_PRIM_WATERMARK       ETM_MB_PRIM_SIZE
/*@}*/


/** Define a structure used to store all the informations related to KPD's task
 *	& MBs identifiers.
 */
typedef struct ctrl_blk
{
    T_RVF_MB_ID	    prim_id;
    T_RVF_ADDR_ID   addr_id;
    T_RVM_RETURN    (*error_ft)(T_RVM_NAME        swe_name, 
                                T_RVM_RETURN      error_cause,
                                T_RVM_ERROR_TYPE  error_type,
                                T_RVM_STRING      error_msg);
} T_ETM_ENV_CTRL_BLK;


/**
 * @name Generic functions
 * Generic functions declarations.
 */
/*@{*/

T_RVM_RETURN etm_get_info(T_RVM_INFO_SWE *infoSWE);

T_RVM_RETURN etm_set_info( T_RVF_ADDR_ID  addr_id,
                           T_RV_RETURN    return_path[],
                           T_RVF_MB_ID    bk_id[],
                           T_RVM_RETURN   (*rvm_error_ft)(T_RVM_NAME swe_name, 
                                                          T_RVM_RETURN error_cause, 
                                                          T_RVM_ERROR_TYPE error_type, 
                                                          T_RVM_STRING error_msg));

T_RVM_RETURN etm_init(void);
T_RVM_RETURN etm_start(void);
T_RVM_RETURN etm_stop(void);
T_RVM_RETURN etm_kill(void);

/*@}*/


#endif /* #ifndef _ETM_ENV_H_ */

/*******************************************************************************
 *
 * ti_profiler_env.c
 *
 * This module interfaces the environment and contains all start/init/stop...
 * functions of the TI profiler module.
 *
 * (C) Texas Instruments, all rights reserved
 *
 * Version number	: 0.1
 *
 * History			: 0.1 (7/30/2001) - Created
 *
 * Date             : 7/30/2001
 *
 * Author           : Philippe Martinez, 
 *
 ******************************************************************************/

#include "config/debug.cfg"

#if (TI_PROFILER == 1) || (TI_NUC_MONITOR == 1)

#include <string.h>

#include "nucleus.h"

#include "rvf/rvf_api.h"
#include "rv/rv_general.h"
#include "rvm/rvm_gen.h"
#include "rvm/rvm_priorities.h"
#include "rvm/rvm_use_id_list.h"

#include "rvt/ti_profiler/ti_profiler_env.h"

/*-------------------------------------------------------*/
/* external variables & prototypes                       */
/*-------------------------------------------------------*/

  extern T_RVF_MB_ID   prof_mb_id;
  extern T_RVF_ADDR_ID prof_addr_id;

  extern T_RV_RET ti_prf_core( void );
  extern void     ti_prf_init( void );

/********************************************************************************/
/*                                                                              */
/*    Function Name:   ti_prf_get_info										    */
/*                                                                              */
/*    Purpose:         This function is used to notify the Bluetooth            */
/*                     Environment of the TI profiler's  requirements.          */
/*                                                                              */
/*    Input Parameters:                                                         */
/*                                                                              */
/*    Output Parameters:                                                        */
/*                                                                              */
/*    Global Parameters:                                                        */
/*        None.                                                                 */
/*                                                                              */
/*    Note:                                                                     */
/*        None.                                                                 */
/*                                                                              */
/********************************************************************************/
T_RVM_RETURN
ti_prf_get_info (T_RVM_INFO_SWE *p_info_swe)
{
	if (!p_info_swe)
		return RVM_INVALID_PARAMETER;

   /* SWE info */
   p_info_swe->swe_type = RVM_SWE_TYPE_4;
   p_info_swe->type_info.type4.swe_use_id = TI_PRF_USE_ID;

   memcpy( p_info_swe->type_info.type4.swe_name, "TI_PRF", sizeof("TI_PRF") );

   p_info_swe->type_info.type4.stack_size = TI_PROFILER_TASK_STACK_SIZE;
   p_info_swe->type_info.type4.priority   = TI_PROFILER_TASK_PRIORITY;


   /* Set the return path */
   p_info_swe->type_info.type4.return_path.callback_func	= NULL;
   p_info_swe->type_info.type4.return_path.addr_id	      = 0;


   /* memory bank info */
   p_info_swe->type_info.type4.nb_mem_bank = 1;

   memcpy ((UINT8 *) 	p_info_swe->type_info.type4.mem_bank[0].bank_name, "TI_PRF", RVM_NAME_MAX_LEN);
   p_info_swe->type_info.type4.mem_bank[0].initial_params.size          = TI_PROFILER_MB_SIZE;
   p_info_swe->type_info.type4.mem_bank[0].initial_params.watermark     = TI_PROFILER_MB_WATERMARK;

   /* linked SWE info */
   /* this SWE does not require any SWE to run */
   p_info_swe->type_info.type4.nb_linked_swe = 0;

   /* generic functions */
   p_info_swe->type_info.type4.set_info = ti_prf_set_info;
   p_info_swe->type_info.type4.init     = ti_prf_env_init;
   p_info_swe->type_info.type4.core     = ti_prf_core;
   p_info_swe->type_info.type4.stop     = ti_prf_env_stop;
   p_info_swe->type_info.type4.kill     = ti_prf_kill;

	return RVM_OK;
}



/********************************************************************************/
/*                                                                              */
/*    Function Name:   ti_prf_set_info							    			*/
/*                                                                              */
/*    Purpose:         This function is used to notify the TI profiler          */
/*                     about mb ids and address id	.							*/
/*                                                                              */
/*    Input Parameters:                                                         */
/*        addr_id: Address id.													*/
/*        mb_id[]: array of memory bank ids.									*/
/*                                                                              */
/*    Output Parameters:                                                        */
/*        None.																	*/
/*                                                                              */
/*    Global Parameters:                                                        */
/*        None.                                                                 */
/*                                                                              */
/*    Note:                                                                     */
/*        None.                                                                 */
/*                                                                              */
/********************************************************************************/

T_RVM_RETURN
ti_prf_set_info( T_RVF_ADDR_ID  addr_id,
				      T_RV_RETURN_PATH		return_path[],
                                      T_RVF_MB_ID bk_id[],
				      T_RVM_RETURN (*rvm_error_ft)(T_RVM_NAME swe_name,
					       			               T_RVM_RETURN error_cause,
								                   T_RVM_ERROR_TYPE error_type,
								                   T_RVM_STRING error_msg))
{
	/*
	** Save the parameters given by environment
	*/

	/* Addr Id */
	prof_addr_id = addr_id;

	/* Bank Id */
	prof_mb_id = bk_id [0];

	return RVM_OK;
}




/********************************************************************************/
/*                                                                              */
/*    Function Name:   ti_prf_env_init											*/
/*                                                                              */
/*    Purpose:         This is the initialization function.						*/
/*					   It is called by the RVM									*/
/*                                                                              */
/*    Input Parameters:                                                         */
/*        None.																	*/
/*                                                                              */
/*    Output Parameters:                                                        */
/*        None.																	*/
/*                                                                              */
/*    Global Parameters:                                                        */
/*        None.                                                                 */
/*                                                                              */
/*    Note:                                                                     */
/*        None.                                                                 */
/*                                                                              */
/********************************************************************************/

T_RVM_RETURN
ti_prf_env_init (void)
{

  ti_prf_init();
  return RVM_OK;
}



/*
** Functions stop and kill are defined for compilation only
*/
T_RVM_RETURN
ti_prf_env_stop (void)
{
	return RVM_OK;
}


T_RVM_RETURN
ti_prf_kill (void)
{
	return RVM_OK;
}

#endif

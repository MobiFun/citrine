/******************************************************************************/
/*                                                                            */
/*    File Name:   rtc_env.c                                                  */
/*                                                                            */
/*    Purpose:   This file contains routines that will be called in order     */
/*               to:                                                          */
/*             - notify the Riviera manager of the RTC's Memory               */
/*               Banks requirements,                                          */
/*             - initialize all the RTC's data structures,                    */
/*                                                                            */
/*    Note:      None.                                                        */
/*                                                                            */
/*    Revision History:                                                       */
/*    03/22/01   Laurent Sollier      Create.                                 */
/*                                                                            */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved  */
/*                                                                            */
/******************************************************************************/

#include "../../riviera/rvm/rvm_priorities.h"
#include "../../riviera/rvm/rvm_use_id_list.h"
#include "rtc_env.h"
#include "rtc_i.h"
#include "rtc_api.h"
#include <string.h>


/* Define a pointer to the RTC environment control block */
T_RTC_ENV_CTRL_BLK *rtc_env_ctrl_blk = NULL;

/* External declaration */
extern T_RV_RET rtc_core(void);

/* Define global pointer to the error function */

static T_RVM_RETURN (*rtc_error_ft) (T_RVM_NAME       swe_name,
                                     T_RVM_RETURN     error_cause,
                                     T_RVM_ERROR_TYPE error_type,
                                     T_RVM_STRING     error_msg);


/******************************************************************************
* Function     : rtc_get_info
*
* Description : This function is called by the RV manager to learn 
*            rtc requirements in terms of memory, SWEs...
*
* Parameters  : T_RVM_INFO_SWE  * swe_info: pointer to the structure to fill
*            containing infos related to the rtc SWE.
*
* Return      :  T_RVM_RETURN
* 
* History     : 0.1 (22-March-2001)
*                           
*
******************************************************************************/
T_RVM_RETURN rtc_get_info(T_RVM_INFO_SWE* swe_info)
{
   /* SWE info */
   swe_info->swe_type = RVM_SWE_TYPE_4;
   swe_info->type_info.type4.swe_use_id = RTC_USE_ID;
   memcpy( swe_info->type_info.type4.swe_name, "RTC", sizeof("RTC") );

   swe_info->type_info.type4.stack_size = RTC_STACK_SIZE;
   swe_info->type_info.type4.priority   = RVM_RTC_TASK_PRIORITY;

   /* Set the return path */
   swe_info->type_info.type4.return_path.callback_func   = NULL;
   swe_info->type_info.type4.return_path.addr_id         = 0;

   
   /* memory bank info */
   swe_info->type_info.type4.nb_mem_bank = 1;
   
   memcpy ((UINT8 *)    swe_info->type_info.type4.mem_bank[0].bank_name, "RTC_PRIM", 9);
   swe_info->type_info.type4.mem_bank[0].initial_params.size          = RTC_MB_PRIM_SIZE;
   swe_info->type_info.type4.mem_bank[0].initial_params.watermark     = RTC_MB_PRIM_WATERMARK;

   /* linked SWE info */
   /* this SWE does not require any SWE to run */
   swe_info->type_info.type4.nb_linked_swe = 0;
   
   /* generic functions */
   swe_info->type_info.type4.set_info = rtc_set_info;
   swe_info->type_info.type4.init     = rtc_init;
   swe_info->type_info.type4.core     = rtc_core;
   swe_info->type_info.type4.stop     = rtc_stop;
   swe_info->type_info.type4.kill     = rtc_kill;

   return RV_OK;
}


/******************************************************************************
* Function     : rtc_set_info
*
* Description : This function is called by the RV manager to inform  
*            the rtc SWE about task_id, mb_id and error function.
*
* Parameters  : - T_RVF_ADDR_ID  addrId: Address id.
*               - T_RVF_MB_ID mbId[]: array of memory bank ids.
*            - callback function to call in case of unrecoverable error.
*
* Return      : T_RVM_RETURN
* 
* History     : 0.1 (22-March-2001)
*                           
*
******************************************************************************/
T_RVM_RETURN rtc_set_info(T_RVF_ADDR_ID   addrId,
                          T_RV_RETURN      return_path[],
                          T_RVF_MB_ID      mbId[],
                          T_RVM_RETURN   (*callBackFct) (T_RVM_NAME SWEntName,
                                                         T_RVM_RETURN errorCause,
                                                         T_RVM_ERROR_TYPE errorType,
                                                         T_RVM_STRING errorMsg) )
{
   T_RVF_MB_STATUS mb_status;


   rvf_send_trace("RTC : rtc_set_info: try to init GLOBAL INFO RTC structure ... ",62,
                  NULL_PARAM,
                  RV_TRACE_LEVEL_DEBUG_HIGH,
                  RTC_USE_ID);

   mb_status = rvf_get_buf(mbId[0],sizeof(T_RTC_ENV_CTRL_BLK),(void **) &rtc_env_ctrl_blk);
   
   if (mb_status == RVF_RED) 
   {
      rvf_send_trace("RTC : rtc_set_info: Not enough memory to initiate GLOBAL INFO RTC structure ... ",86,
                     NULL_PARAM,
                     RV_TRACE_LEVEL_ERROR,
                     RTC_USE_ID);

      return (RVM_MEMORY_ERR);
   }


   /* store the pointer to the error function */
   rtc_error_ft = callBackFct ;

   rtc_env_ctrl_blk->prim_id = mbId[0];
   rtc_env_ctrl_blk->addr_id = addrId;


   return RV_OK;
}


/******************************************************************************
* Function     : rtc_init
*
* Description : This function is called by the RV manager to initialize the 
*            rtc SWE before creating the task and calling rtc_start. 
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History     : 0.1 (22-March-2001)
*                           
*
******************************************************************************/
T_RVM_RETURN rtc_init(void)
{
   return RTC_Initialize();
}


/******************************************************************************
* Function     : rtc_stop
*
* Description : This function is called by the RV manager to stop the rtc SWE.
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History     : 0.1 (22-March-2001)
*                           
*
******************************************************************************/
T_RVM_RETURN rtc_stop(void)
{
   /* other SWEs have not been killed yet, rtc can send messages to other SWEs */

   return RV_OK;
}


/******************************************************************************
* Function     : rtc_kill
*
* Description : This function is called by the RV manager to kill the rtc 
*            SWE, after the rtc_stop function has been called.
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History     : 0.1 (22-March-2001)
*                           
*
******************************************************************************/
T_RVM_RETURN rtc_kill (void)
{
   /* free all memory buffer previously allocated */
   rvf_free_buf ((void *) rtc_env_ctrl_blk->msg_alarm_event);
   rvf_free_buf ((void *) rtc_env_ctrl_blk);

   return RV_OK;
}

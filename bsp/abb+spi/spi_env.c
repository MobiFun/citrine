/****************************************************************************/
/*                                                                          */
/*  File Name: spi_env.c                                                    */
/*                                                                          */
/*  Purpose: This file contains definitions for RV manager related          */
/*           functions used to get info, start and stop the SPI SWE.        */
/*                                                                          */
/*  Version    0.1                                                          */
/*                                                                          */
/*  Date       Modification                                                 */
/*  ------------------------------------                                    */
/*  20/08/2000 Create                                                       */
/*                                                                          */
/* Author      David Lamy-Charrier (dlamy@tif.ti.com)                       */
/*                                                                          */
/* (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "spi_env.h"
#include "../../riviera/rv/general.h"
#include "../../riviera/rvm/rvm_gen.h"
#include "../../riviera/rvm/rvm_priorities.h"
#include "../../riviera/rvm/rvm_use_id_list.h"
#include <string.h>

//extern T_RV_RET spi_core(void);
#include "spi_task.h"


/* global pointer to the error function */
static T_RVM_RETURN (*spi_error_ft)(T_RVM_NAME swe_name, T_RVM_RETURN error_cause,
                                    T_RVM_ERROR_TYPE error_type,T_RVM_STRING error_msg);

/* Global variables */
T_SPI_GBL_INFO *SPI_GBL_INFO_PTR = NULL;



/******************************************************************************
* Function	  : spi_get_info
*
* Description : This function is called by the RV manager to learn 
*               spi requirements in terms of memory, SWEs...
*
* Parameters  : T_RVM_INFO_SWE  * swe_info: pointer to the structure to fill
*               containing infos related to the SPI SWE.
*
* Return      : T_RVM_RETURN
* 
* History     : 0.1 (20-August-2000)
*
*
******************************************************************************/
T_RVM_RETURN spi_get_info(T_RVM_INFO_SWE  * infoSWE)
{

   /* SWE info */

   infoSWE->swe_type = RVM_SWE_TYPE_4;
   infoSWE->type_info.type4.swe_use_id = SPI_USE_ID;
   memcpy( infoSWE->type_info.type4.swe_name, "SPI", sizeof("SPI") );

   infoSWE->type_info.type4.stack_size = SPI_STACK_SIZE;
   infoSWE->type_info.type4.priority   = RVM_SPI_TASK_PRIORITY;


   /* memory bank info */
   infoSWE->type_info.type4.nb_mem_bank = 1;

   memcpy ((UINT8 *) infoSWE->type_info.type4.mem_bank[0].bank_name, "SPI_PRIM", 9);
   infoSWE->type_info.type4.mem_bank[0].initial_params.size          = SPI_MB_PRIM_SIZE;
   infoSWE->type_info.type4.mem_bank[0].initial_params.watermark     = SPI_MB_PRIM_WATERMARK;


   /* linked SWE info */
   infoSWE->type_info.type4.nb_linked_swe = 0;


   /* generic functions */
   infoSWE->type_info.type4.set_info = spi_set_info;
   infoSWE->type_info.type4.init     = spi_init;
   infoSWE->type_info.type4.core     = spi_core;
   infoSWE->type_info.type4.stop     = spi_stop;
   infoSWE->type_info.type4.kill     = spi_kill;

   /* Set the return path */
   infoSWE->type_info.type4.return_path.callback_func	= NULL;
   infoSWE->type_info.type4.return_path.addr_id	      = 0;

   return RV_OK;
}


/******************************************************************************
* Function     : spi_set_info
*
* Description : This function is called by the RV manager to inform  
*               the spi SWE about task_id, mb_id and error function.
*
* Parameters  : - T_RVM_TASK_ID  taskId: task_id.
*               - T_RV_RETURN    ReturnPath[], array of return path for linked SWE
*               - T_RVF_MB_ID    mbId[]: array of memory bank ids.
*               - callback function to call in case of unrecoverable error.
*
* Return      : T_RVM_RETURN
* 
* History     : 0.1 (20-August-2000)
*
*
******************************************************************************/
T_RVM_RETURN spi_set_info( T_RVF_ADDR_ID  addr_id,
                           T_RV_RETURN          ReturnPath[],
                           T_RVF_MB_ID          mbId[],
                           T_RVM_RETURN      (*callBackFct)(T_RVM_NAME SWEntName,
                                                            T_RVM_RETURN errorCause,
                                                            T_RVM_ERROR_TYPE errorType,
                                                            T_RVM_STRING errorMsg))
{
   T_RVF_MB_STATUS mb_status;
   UINT16 i;

   rvf_send_trace("SPI : spi_set_info: try to init GLOBAL INFO SPI structure ... ",62, 
                   NULL_PARAM,
                   RV_TRACE_LEVEL_DEBUG_LOW,
                   SPI_USE_ID);

   mb_status = rvf_get_buf(mbId[0],sizeof(T_SPI_GBL_INFO),(void **) &SPI_GBL_INFO_PTR);

   if (mb_status == RVF_RED) 
   {
      rvf_send_trace("SPI : spi_set_info: Not enough memory to initiate GLOBAL INFO SPI structure ... ",80,
                      NULL_PARAM,
                      RV_TRACE_LEVEL_ERROR,
                      SPI_USE_ID);

      return (RVM_MEMORY_ERR);
   }


   /* store the pointer to the error function */
   spi_error_ft = callBackFct ;

   SPI_GBL_INFO_PTR->prim_id = mbId[0];

   /* Store the addr id */
   SPI_GBL_INFO_PTR->addr_id = addr_id;

   for(i=0;i<MADC_NUMBER_OF_MEAS;i++)
   {
      SPI_GBL_INFO_PTR->adc_result[i] = 0;
   }

   SPI_GBL_INFO_PTR->is_gsm_on = FALSE;

   SPI_GBL_INFO_PTR->SpiTaskReady = FALSE;

   /* spi task_id and spi mb_id could be retrieved later 
      using rvf_get_taskid and rvf_get_mb_id functions */

   return RV_OK;
}


/******************************************************************************
* Function  : spi_init
*
* Description : This function is called by the RV manager to initialize the 
*               spi SWE before creating the task and calling spi_start. 
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1 (20-August-2000)
*
*
******************************************************************************/
T_RVM_RETURN spi_init(void)
{

   return RV_OK;
}



/******************************************************************************
* Function	  : spi_stop
*
* Description : This function is called by the RV manager to stop the spi SWE.
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History	  : 0.1 (20-August-2000)
*
*
******************************************************************************/
T_RVM_RETURN spi_stop(void)
{
   /* other SWEs have not been killed yet, spi can send messages to other SWEs */

   return RV_OK;
}


/******************************************************************************
* Function  : spi_kill
*
* Description : This function is called by the RV manager to kill the spi 
*               SWE, after the spi_stop function has been called.
*
* Parameters  : None
*
* Return      : T_RVM_RETURN
* 
* History     : 0.1 (20-August-2000)
*
*
******************************************************************************/
T_RVM_RETURN spi_kill (void)
{
   /* free all memory buffer previously allocated */
   return RV_OK;
}

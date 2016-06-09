/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_env.c                                                   */
/*                                                                          */
/*  Purpose:  This file contains routines that will be called in order to:  */
/*            - notify the Environment of the diagnose's Memory Banks       */
/*              requirements,                                               */
/*            - set diagnose's addr and memory banks IDs                    */
/*            - initialize all the diagnose's data structures,              */
/*            - start the diagnose's task                                   */
/*            - stop the diagnose's task                                    */
/*            - kill the diagnose's task                                    */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date                 Modification                                       */
/*  ------------------------------------                                    */
/*  26 September 2001    Create                                             */
/*                                                                          */
/*  Author     Stephanie Gerthoux                                           */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#ifdef RVM_DAR_SWE
   #include "../../riviera/rv/rv_general.h"
   #include "../../riviera/rvm/rvm_gen.h"
   #include "../../riviera/rvm/rvm_priorities.h"
   #include "../../riviera/rvm/rvm_use_id_list.h"
   #include "dar_api.h"
   #include "dar_env.h"
   #include "dar_macro_i.h"

   #include <string.h>

   extern T_RV_RET dar_core(void);

   /* Initialisation of the pointer to the Global Environment Control block */
   T_DAR_ENV_CTRL_BLK *dar_gbl_var_p = NULL;

   /**** Define extern variables ****/
   /* Get the last status of the system */
   extern T_DAR_RECOVERY_STATUS dar_current_status;
  
   /* Get the last exception of the system */
   extern UINT8 dar_exception_status;

   /* Define the recovery buffer */
   extern UINT32 dar_recovery_buffer;

   /* DAR previous status */
   extern T_DAR_RECOVERY_STATUS  dar_previous_status;

   /* DAR previous status */
   extern UINT8  dar_previous_exception;

   /***************************************************************************/
   /* Function   : dar_get_info                                               */
   /*                                                                         */
   /* Description : This function is called by the RV Environment to learn    */
   /*               diagnose requirements in terms of memory, SWEs...         */
   /*                                                                         */
   /* Parameters  : T_RVM_INFO_SWE  * swe_info: pointer to the structure to   */
   /*               fill containing infos related to the diagnose SWE.        */
   /*                                                                         */
   /* Return      :  T_RV_RETURN                                              */
   /*                                                                         */
   /* History      : 0.1 (26-September-2001)                                  */
   /*                                                                         */
   /*                                                                         */
   /***************************************************************************/
   T_RVM_RETURN dar_get_info(T_RVM_INFO_SWE  *infoSWEnt)
   {
      /* SWE info */
      infoSWEnt->swe_type = RVM_SWE_TYPE_4;
      infoSWEnt->type_info.type4.swe_use_id = DAR_USE_ID;

      memcpy( infoSWEnt->type_info.type4.swe_name, "DAR", sizeof("DAR") );

      infoSWEnt->type_info.type4.stack_size = DAR_STACK_SIZE;
      infoSWEnt->type_info.type4.priority   = RVM_DAR_TASK_PRIORITY;


      /* Set the return path */
      infoSWEnt->type_info.type4.return_path.callback_func = NULL;
      infoSWEnt->type_info.type4.return_path.addr_id       = 0;


      /* memory bank info */
      infoSWEnt->type_info.type4.nb_mem_bank = 0x01;
 
      /* Memory bank used to receive/send the message to/from the entity */
      memcpy ((UINT8 *) infoSWEnt->type_info.type4.mem_bank[0].bank_name, "DAR_MB", sizeof("DAR_MB"));
      infoSWEnt->type_info.type4.mem_bank[0].initial_params.size          = DAR_MB_SIZE;
      infoSWEnt->type_info.type4.mem_bank[0].initial_params.watermark     = DAR_MB_WATERMARK;

      /* linked SWE info */ 
      /* this SWE requires the FFS SWE to run */
      
      infoSWEnt->type_info.type4.nb_linked_swe   = 0x01;
      infoSWEnt->type_info.type4.linked_swe_id[0]= FFS_USE_ID;

      /* generic functions */
      infoSWEnt->type_info.type4.set_info = dar_set_info;
      infoSWEnt->type_info.type4.init     = dar_init;
      infoSWEnt->type_info.type4.core     = dar_core;
      infoSWEnt->type_info.type4.stop     = dar_stop;
      infoSWEnt->type_info.type4.kill     = dar_kill;

      return RVM_OK;
   }

   /**************** End of dar_get_info function *****************************/

   /***************************************************************************/
   /* Function   : dar_set_info                                               */
   /*                                                                         */
   /* Description : This function is called by the RV Environment to inform   */
   /*               the diagnose SWE about addr_id, mb_id and error function. */
   /*                                                                         */
   /* Parameters  : - T_RVF_ADDR_ID  addrId: address Id                       */
   /*               - T_RVF_MB_ID mb_id[]: array of memory bank ids.          */
   /*               - callback function to call in case of unrecoverable error*/
   /*                                                                         */
   /* Return      : T_RVM_RETURN                                              */
   /*                                                                         */
   /* History    : 0.1 (27-September-2001 )                                   */
   /*                                                                         */
   /*                                                                         */
   /***************************************************************************/
   T_RVM_RETURN dar_set_info(T_RVF_ADDR_ID addrId,
                             T_RV_RETURN   return_path[],
                             T_RVF_MB_ID   mbId[],
                             T_RVM_RETURN  (*callBackFctError) ( T_RVM_NAME SWEntName,
                                                                 T_RVM_RETURN errorCause,
                                                                 T_RVM_ERROR_TYPE errorType,
                                                                 T_RVM_STRING errorMsg) )
   {

      /* Declare local variable.*/
      T_RVF_MB_STATUS mb_status = RVF_GREEN;

      /* Allocate memory required to store the Global Environment control
	 Block. */
      mb_status = rvf_get_buf(mbId[0], 
                              sizeof(T_DAR_ENV_CTRL_BLK), 
                              (T_RVF_BUFFER **) & dar_gbl_var_p);

      /* If insufficient resources to properly run the DAR's task,
	 then abort.   */
      switch (mb_status)
      {
         case RVF_GREEN:
         {
            /* Initialize the Global Environment Control Block */
            memset((UINT8 *) dar_gbl_var_p,
                   0x00,
                   sizeof (T_DAR_ENV_CTRL_BLK));

            /* Store the memory bank IDs assigned to the DAR */
            dar_gbl_var_p->mb_dar = mbId[0];

            /* Store the addr ID assigned to the DAR */
            dar_gbl_var_p->addrId = addrId;

            /* Store the function to be called whenever any unrecoverable */
            /* error occurs.                                              */
            dar_gbl_var_p->callBackFctError = callBackFctError; 
            DAR_SEND_TRACE(" Diagnose And Recovery (env). DAR's information set ",
                           RV_TRACE_LEVEL_DEBUG_LOW);
            break;
         }
         case RVF_YELLOW:
         {
            rvf_free_buf((T_RVF_BUFFER *) dar_gbl_var_p);
            DAR_TRACE_WARNING(" DAR memory warning (orange memory)");
            return (RV_MEMORY_ERR);
         }
         default:
         {
            DAR_TRACE_WARNING(" DAR memory warning (red memory)");
            return (RV_MEMORY_ERR);
         }
      } /* switch (mb_status) */

      return (RV_OK);
   } /*************** End of dar_set_info function ****************************/



   /***************************************************************************/
   /* Function   : dar_init                                                   */
   /*                                                                         */
   /* Description : This function is called by the RV Environment to init the */
   /*               diagnose SWE before creating the task and calling         */
   /*		    dar_start.						      */
   /*                                                                         */
   /* Parameters  : None                                                      */
   /*                                                                         */
   /* Return      : T_RVM_RETURN                                              */
   /*                                                                         */
   /* History    : 0.1 (27-September-2001)                                    */
   /*                                                                         */
   /*                                                                         */
   /***************************************************************************/
   T_RVM_RETURN dar_init(void)
   {
      /*** Declare local variables ***/
      UINT16 i = 0;    
    
      /*** Initialization of the DAR global structure ***/

      /* Initialization of the dar_filter_array */ 
      for (i=0;i<DAR_MAX_GROUP_NB;i++)
      {
         dar_gbl_var_p ->dar_filter_array[i].group_nb     = DAR_INITIALIZATION_VALUE;
         dar_gbl_var_p ->dar_filter_array[i].mask_warning = DAR_INITIALIZATION_VALUE;
         dar_gbl_var_p ->dar_filter_array[i].mask_debug   = DAR_INITIALIZATION_VALUE;
      }

      /*Initialization of the index and the free index */
      dar_gbl_var_p ->index       = DAR_INVALID_VALUE;
      dar_gbl_var_p ->free_index  = DAR_INVALID_VALUE;

      /*** Value of the DAR recovery status ***/
      dar_previous_status = dar_current_status;           

      /* erase the dar_current_status value */
      dar_current_status = DAR_POWER_ON_OFF;          

      /*** Value of the DAR exception status ***/
      dar_previous_exception = dar_exception_status;           

      /* erase the dar_exception_status value */
      dar_exception_status = DAR_NO_ABORT_EXCEPTION;          

      return RV_OK;
   }


   /***************************************************************************/
   /* Function    : dar_stop                                                  */
   /*                                                                         */
   /* Description : This function is called by the RV Environment to stop the */
   /*               diagnose SWE.                                             */
   /*                                                                         */
   /* Parameters  : None                                                      */
   /*                                                                         */
   /* Return      : T_RVM_RETURN                                              */
   /*                                                                         */
   /* History     : 0.1 (27-September-2001)				      */
   /*                                                                         */
   /*                                                                         */
   /***************************************************************************/
   T_RVM_RETURN dar_stop(void)
   {
      /* other SWEs have not been killed yet, DAR can send messages to other
	 SWEs        */

      return RV_OK;
   }


   /***************************************************************************/
   /* Function   :  dar_kill                                                  */
   /*                                                                         */
   /* Description : This function is called by the RV Environment to kill the */
   /*               diagnose  SWE, after the diagnose_stop function has been  */
   /*		    called.						      */
   /*                                                                         */
   /* Parameters  : None                                                      */
   /*                                                                         */
   /* Return      : T_RVM_RETURN                                              */
   /*                                                                         */
   /* History     : 0.1 (27-September-2001)                                   */
   /*                                                                         */
   /*                                                                         */
   /***************************************************************************/
   T_RVM_RETURN dar_kill (void)
   {
      /* free all memory buffer previously allocated */
      rvf_free_buf ((T_RVF_BUFFER *) dar_gbl_var_p);
      return RV_OK;
   }

#endif /* #ifdef RVM_DAR_SWE */

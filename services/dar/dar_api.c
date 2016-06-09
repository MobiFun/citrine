/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_api.c                                                   */
/*                                                                          */
/*  Purpose:  This file contains all the functions used to service          */
/*            primitives.                                                   */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date            Modification                                            */
/*  ----------------------------------------------------------------------  */
/*  2 October 2001  Create                                                  */
/*                                                                          */
/*  Author    - Stephanie Gerthoux                                          */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"


#ifdef RVM_DAR_SWE
   #include "../../include/config.h"

   #include "../../bsp/abb+spi/abb.h"
   #include "../../riviera/rv/rv_general.h"
   #include "../../riviera/rvm/rvm_gen.h"
   #include "dar_api.h"
   #include "dar_env.h"    
   #include "dar_error_hdlr_i.h"
   #include "dar_messages_i.h"
   #include "dar_msg_ft.h"
   #include "dar_macro_i.h" 
   #include "dar_diagnose_i.h"
   #include "dar_const_i.h"
   #include "dar_emergency.h"

   #ifndef _WINDOWS
      /* enable the timer */
      #include "../../bsp/mem.h"
      #include "../../bsp/timer.h"
      #include "../../bsp/iq.h"
      #if (CHIPSET == 12)
        #include "inth/sys_inth.h"
      #endif
   #endif

   /**** Global variable ****/
   /* Define load timer */
   static UINT16 dar_load_tim = 0x0;

   /* Increment variable */
   static BOOLEAN dar_increment = TRUE;

   /* DAR previous status */
   T_DAR_RECOVERY_STATUS   dar_previous_status;

   /* DAR previous exception*/
   UINT8    dar_previous_exception;

    /**** Define extern variables ****/
   /* Get the status of the system */
   extern T_DAR_RECOVERY_STATUS dar_current_status;

   /* dar_exception_status : to get the status of the exception */
   extern UINT8 dar_exception_status;

   /* Define the recovery buffer */
   extern UINT8 dar_recovery_buffer[DAR_RECOVERY_DATA_MAX_BUFFER_SIZE];

   /* Define a pointer to the Global Environment Control block   */
   extern T_DAR_ENV_CTRL_BLK *dar_gbl_var_p;

   /* *********************************************************************** */
   /*                                RECOVERY                                 */
   /* *********************************************************************** */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_recovery_get_status                             */
   /*                                                                         */
   /*    Purpose:  This function is called by the MMI at the beginning of the */
   /*              procedure, in order to get the status of the last reset of */
   /*              the system.                                                */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        Dar recovery status                                              */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*         Validation of the function execution.                           */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None.                                                            */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*        None.                                                            */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET dar_recovery_get_status(T_DAR_RECOVERY_STATUS* status)
   {
      /* Variable to know the status of th Omega VRPC register */
      UINT16 dar_pwr_status;

      *status = dar_previous_status;
      switch (dar_previous_status)
      {
         case (DAR_WATCHDOG):
         {
      	   DAR_SEND_TRACE("Dar Entity: Status of the last reset of the system = WATCHDOG",RV_TRACE_LEVEL_DEBUG_HIGH);
            break;
         }
         case (DAR_NORMAL_SCUTTLING):
         {
      	   DAR_SEND_TRACE("Dar Entity: Status of the last reset of the system = NORMAL SCUTTLING",RV_TRACE_LEVEL_DEBUG_HIGH);
            break;
         }
         case (DAR_EMERGENCY_SCUTTLING):
         {
      	   DAR_SEND_TRACE("Dar Entity: Status of the last reset of the system = EMERGENCY SCUTTLING",RV_TRACE_LEVEL_DEBUG_HIGH);
            switch (dar_previous_exception)
            {
                case (DAR_EXCEPTION_DATA_ABORT):
                {
      	            DAR_SEND_TRACE(" A DATA ABORT exception has occured",RV_TRACE_LEVEL_DEBUG_HIGH);
                     break;
                }
                
                case (DAR_EXCEPTION_PREFETCH_ABORT):
                {
      	            DAR_SEND_TRACE("          A PREFETCH ABORT exception has occured",RV_TRACE_LEVEL_DEBUG_HIGH);
                     break;
                }
                
                case (DAR_EXCEPTION_UNDEFINED):
                {
      	            DAR_SEND_TRACE("          A UNDEFINED INSTRUCTION exception has occured",RV_TRACE_LEVEL_DEBUG_HIGH);
                     break;
                }
                
                case (DAR_EXCEPTION_SWI):
                {
      	            DAR_SEND_TRACE("          A SWI exception has occured",RV_TRACE_LEVEL_DEBUG_HIGH);
                     break;
                }
                
                case (DAR_EXCEPTION_RESERVED):
                {
      	            DAR_SEND_TRACE("          A RESERVED exception has occured",RV_TRACE_LEVEL_DEBUG_HIGH);
                     break;
                }

                default:
               {
       	         DAR_SEND_TRACE("          An error has been detected",RV_TRACE_LEVEL_DEBUG_HIGH);
                  break;
               }

            }

            break;
         }
         default:
         {
            #if 1 //(_GSM==1) || (defined _WINDOWS)
               dar_pwr_status = ABB_Read_Status();

               #if ((ANALOG == 1) || (ANALOG == 2))
               if (dar_pwr_status & ONBSTS)
               #elif (ANALOG == 3)
               if (dar_pwr_status & PWONBSTS)
               #endif
               {			    
                  /* Switch on Condition on ON BUTTON Push */
                  DAR_SEND_TRACE("Dar Entity: Status of the last reset of the system = POWER ON/OFF",RV_TRACE_LEVEL_DEBUG_HIGH);
                  break;
               }
               else
               {
                  /* Branch to a reset at adress 0 */
                  DAR_SEND_TRACE("Dar Entity: Status of the last reset of the system = BRANCH to adress 0",RV_TRACE_LEVEL_DEBUG_HIGH);
                  break;
               }
            #else
               /* the SPI is not available in BOARD_TEST configuration */
               DAR_SEND_TRACE("Dar Entity: Status of the last reset of the system = POWER ON/OFF or BRANCH to adress 0",RV_TRACE_LEVEL_DEBUG_HIGH);
            #endif
         }
      }/* switch */

      return(RV_OK);	

   } /* dar_recovery_get_status */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_recovery_config                                 */
   /*                                                                         */
   /*    Purpose:  This function is used to store a callback function that    */
   /*              will be called by the recovery system when a recovery      */
   /*              procedure has been initiated                               */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        dar callback function                                            */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*        Validation of the function execution.                            */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None.                                                            */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*        None.                                                            */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET dar_recovery_config(T_RV_RET (*dar_store_recovery_data)
						(T_DAR_BUFFER     buffer_p,
						UINT16            length))
   {   
      /* call the callback function */
      dar_gbl_var_p->entity_dar_callback = dar_store_recovery_data; 

      return(RV_OK);
   } /* dar_recovery_config */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_get_recovery_data                               */
   /*                                                                         */
   /*    Purpose:  This function is used to retrieve data that have been      */
   /*              stored in the buffer just before a reset.                  */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*       - the buffer in whom important data have been stored before the   */
   /*		reset                                                         */
   /*       - the length of the buffer                                        */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*        Validation of the function execution.                            */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None.                                                            */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*        None.                                                            */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET dar_get_recovery_data( T_DAR_BUFFER buffer_p, UINT16   length )
   { 
      /* Local variables */
      UINT8 i; 

      if (buffer_p != NULL)
      {
         if(length <= DAR_RECOVERY_DATA_MAX_BUFFER_SIZE)
         {
            /* Retrieve data that have been stored in the global buffer */
            for(i=0;i<length;i++)
            {
               buffer_p[i] = dar_recovery_buffer[i];
            }
         }

         else
         /* the lenth is longer than DAR_RECOVERY_DATA_MAX_BUFFER_SIZE */
         {
            /* So retrieve "DAR_RECOVERY_DATA_MAX_BUFFER_SIZE" data that have been stored */
            /* in the global buffer, the other data (length -"DAR_RECOVERY_DATA_MAX_BUFFER_SIZE") */
            /* are lost*/
            for(i=0;i<DAR_RECOVERY_DATA_MAX_BUFFER_SIZE;i++)
            {
                buffer_p[i] = dar_recovery_buffer[i];
            }

            DAR_SEND_TRACE_PARAM("Bytes nb that haven't be saved due to not enough memory space ",(DAR_RECOVERY_DATA_MAX_BUFFER_SIZE-length),DAR_WARNING);
         }
      } /* if (buffer_p != NULL) */
    
      return(RV_OK);

   } /* dar_get_recovery_data */


   /* *********************************************************************** */
   /*                                  WATCHDOG                               */
   /* *********************************************************************** */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_start_watchdog_timer                            */
   /*                                                                         */
   /*    Purpose:  This function uses the timer as a general purpose timer    */
   /*              instead of Watchdog. It loads the timer, starts it and     */
   /*              then unmasks the interrupt.                                */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        time's interval in milliseconds before the timer expires         */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*        Validation of the function execution.                            */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None.                                                            */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*        None.                                                            */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET   dar_start_watchdog_timer(UINT16 timer_expiration_value)
   {
      DAR_SEND_TRACE("DAR Watchdog timer",RV_TRACE_LEVEL_DEBUG_LOW);

      /* use Watchdog timer set as a general purpose timer                    */
      /* Calculate the load value thanks to the formula:                      */
      /*     timer_expiration_value * 1000 = Tclk * (DAR_LOAD_TIM+1)*2^(PTV+1)*/
      /*     (in Watchdog mode, the value of PTV is fixed to 7)               */
      dar_load_tim = ((timer_expiration_value * 1000)/(1.078*256))-1;

      #ifndef _WINDOWS
       /* Load "dar_load_tim" value */
       TIMER_WriteValue(dar_load_tim);

       /* Start timer with PTV = 7, no autoreload, free = 0 */
       *(volatile UINT16*)TIMER_CNTL_REG = 0x0E80;

       /* Unmask IRQ0 */
       #if (CHIPSET == 12)
         F_INTH_ENABLE_ONE_IT(C_INTH_WATCHDOG_IT);
       #else
         IQ_Unmask(IQ_WATCHDOG);
       #endif
      #endif
   
      return(RV_OK);

   } /* dar_start_watchdog_timer */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_reload_watchdog_timer                           */
   /*                                                                         */
   /*    Purpose:  This function is used to maintain the timer in reloading   */
   /*              it periodically before it expires.                         */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        None                                                             */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*        Validation of the function execution.                            */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None.                                                            */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*        None.                                                            */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET   dar_reload_watchdog_timer()
   {
      #ifndef _WINDOWS

       DAR_SEND_TRACE("Reload Watchdog ",RV_TRACE_LEVEL_DEBUG_LOW);

       /* Stop the timer */
       *(volatile UINT16*)TIMER_CNTL_REG = 0x0E00;

       /* Reload the timer with a different value each time */
       if (dar_increment == TRUE)
       {
            TIMER_WriteValue(++dar_load_tim);
            dar_increment = FALSE;
       }
       else
       {
            TIMER_WriteValue(--dar_load_tim);
            dar_increment = TRUE;
       }

       /* Restart timer with PTV = 7, no autoreload, free = 0 */
       *(volatile UINT16*)TIMER_CNTL_REG = 0x0E80;
      #endif

      return(RV_OK);

   } /* dar_reload_watchdog_timer */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_stop_watchdog_timer                             */
   /*                                                                         */
   /*    Purpose:  This function stops the timer used as a general purpose    */
   /*              timer instead of watchdog.                                 */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        None                                                             */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*        Validation of the function execution.                            */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None.                                                            */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*        None.                                                            */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET   dar_stop_watchdog_timer()
   {
      #ifndef _WINDOWS
       /* Stop the timer */
       *(volatile UINT16*)TIMER_CNTL_REG = 0x0E00;

       /* Mask IRQ0 */
       #if (CHIPSET == 12)
         F_INTH_DISABLE_ONE_IT(C_INTH_WATCHDOG_IT);
       #else
         IQ_Mask(IQ_WATCHDOG);
       #endif
      #endif

      return(RV_OK);

   } /* dar_stop_watchdog_timer */


   /* *********************************************************************** */
   /*                                  RESET                                  */
   /* *********************************************************************** */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_reset_system                                    */
   /*                                                                         */
   /*    Purpose:  This function can be used to reset the system voluntarily. */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        None                                                             */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*        Validation of the function execution.                            */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None.                                                            */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*        None.                                                            */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET   dar_reset_system(void)
   {

      /* Update the DAR recovery status */
      dar_current_status = DAR_NORMAL_SCUTTLING;

      /* Call the MMI callback function to save some parameters before reset */
      dar_gbl_var_p->entity_dar_callback(dar_recovery_buffer,DAR_RECOVERY_DATA_MAX_BUFFER_SIZE);

      /* Send a trace before the reset of the system */
      DAR_SEND_TRACE("Voluntary reset of the system",RV_TRACE_LEVEL_DEBUG_HIGH);

      dar_reset();

      return(RV_OK); 
   }

   /* *********************************************************************** */
   /*                                DIAGNOSE                                 */
   /* *********************************************************************** */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_diagnose_swe_filter			      */
   /*                                                                         */
   /*    Purpose: This function is called to configure the Diagnose filtering.*/
   /*             It allows to determine what Software Entity ( dar_use_id )  */
   /*             wants to use the Diagnose and allows to indicate the level  */
   /*             threshold of the diagnose messages. (Warning or Debug)      */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*         - the dar use id                                                */
   /*         - the dar level                                                 */
   /*			                                                      */
   /*    Output Parameters:                                                   */
   /*        Validation of the function execution.	                      */
   /*                                                                         */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None                                                             */
   /*                                                                         */
   /***************************************************************************/

   T_RV_RET dar_diagnose_swe_filter ( T_RVM_USE_ID  dar_use_id, 
                                     T_DAR_LEVEL   dar_level)
   {
      /* Declare local variables */
      T_RVF_MB_STATUS       mb_status   = RVF_GREEN;
      T_DAR_FILTER_START   *use_id_p = NULL;

      /*********************** dar_diagnose_swe_filter function   *************/

      if (dar_gbl_var_p == NULL )
      {
         dar_error_trace(DAR_ENTITY_NOT_START);
         return(RV_NOT_READY);
      }
       
      /* allocate the memory for the message to send */
      mb_status = rvf_get_buf (dar_gbl_var_p->mb_dar,
                               sizeof (T_DAR_FILTER_START),
                               (T_RVF_BUFFER **) (&use_id_p));

      /* If insufficient resources, then report a memory error and abort. */
      if (mb_status == RVF_YELLOW)
      {
         /* deallocate the memory */
         rvf_free_buf((T_RVF_BUFFER *)use_id_p);
         dar_error_trace(DAR_ENTITY_NO_MEMORY);
         return (RV_NOT_SUPPORTED);
      }
      else
      if (mb_status == RVF_RED)
      {
         dar_error_trace(DAR_ENTITY_NO_MEMORY);
         return (RV_MEMORY_ERR);
      }

      /* fill the message id       */
      use_id_p ->os_hdr.msg_id = DAR_FILTER_REQ;

      /* fill the addr source id */
      use_id_p->os_hdr.src_addr_id = dar_gbl_var_p->addrId;

      /* fill the message parameters (group, mask and level) */
      use_id_p->use_msg_parameter.group_nb = (dar_use_id>>16)& 0x7FFF;
      use_id_p->use_msg_parameter.mask     = (dar_use_id)&0xFFFF;
      use_id_p->use_msg_parameter.level    = dar_level;

      /* send the messsage to the DAR entity */
      rvf_send_msg (dar_gbl_var_p->addrId, 
                    use_id_p);

      return (RV_OK);


   } /* dar_diagnose_swe_filter */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_diagnose_write                                  */
   /*                                                                         */
   /*    Purpose:  This function is called to store diagnose data in RAM      */
   /*              buffer.                                                    */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        Pointer to the message to store                                  */
   /*        Data Format, ( the Binary format is not supported)               */
   /*        Data level,                                                      */
   /*        Data Use Id,                                                     */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*         Validation of the diagnose execution.                           */
   /*                                                                         */
   /*                                                                         */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*        None.                                                            */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET dar_diagnose_write(  T_DAR_INFO    *buffer_p,
                                 T_DAR_FORMAT  format,
                                 T_DAR_LEVEL   diagnose_info_level,
                                 T_RVM_USE_ID  dar_use_id)
   {
      /* Declare local variables */
      UINT8 index =0 ; /* local index */

      /* Check if the dar_use_id group_nb exists */
      /* If the group exists... */
      if(dar_search_group((dar_use_id>>16)& 0x7FFF,&index) == RV_OK)
      {
         /* Check the Dar level   */
         switch(diagnose_info_level) 
         {
            case DAR_WARNING:
            { 
               /* The DAR entity wants to process Warning messages */
               /* check if the mask_warning is in the dar_filter array */
               if (((dar_gbl_var_p ->dar_filter_array[index].mask_warning) 
                      & ((dar_use_id)&0xFFFF)) !=0)
               {
                  /* The Warning messages must be diagnosed */
                  dar_send_write_data( buffer_p, format, diagnose_info_level,
					dar_use_id);
               }

               else
               { 
		  /*
                   * There is no mask_warning for this use_id in the dar_filter
		   * array; the warning messages can't be diagnosed.
		   */
                  DAR_TRACE_WARNING("The Warning messages can't be diagnosed");
               }

               break;
            } /* case DAR_WARNING */

            case DAR_DEBUG:
            {
               /* The DAR entity wants to process Debug messages       */
               /* Check if the mask_debug is in the dar_filter array */

               if (((dar_gbl_var_p ->dar_filter_array[index].mask_debug) 
                      & ((dar_use_id)&0xFFFF)) !=0) 
               {
                  /* The Debug messages must be diagnosed */
                  dar_send_write_data( buffer_p, format, diagnose_info_level,
					dar_use_id);
               }

               else
               { 
		  /*
                   * There is no mask_debug for this use_id in the dar_filter
		   * array; the debug messages can't be diagnosed.
                   */
                  DAR_TRACE_WARNING("The Debug messages can't be diagnosed");
               }
               break;
            } /* case DAR_DEBUG */

            default:
            {
               /* Unknow level has been received */
               DAR_TRACE_WARNING("A DAR unknown level has been received");
               break; 
            }
         } /* switch(msg_p->use_msg_parameter.level) */
      } /* if (search_group(dar_use_id.group_nb,&index)== RV_OK) */

      else
      {
        /* An unknow group message has been received */
       DAR_TRACE_WARNING("A DAR unknown group level message has been received");
      }
      
      return (RV_OK);
   } /* dar_diagnose_write */


   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_diagnose_generate_emergency                     */
   /*                                                                         */
   /*    Purpose:  This function is called to store diagnose data in RAM      */
   /*              buffer when an emergency has been detected and goes to     */
   /*              emergency (automatic reset)                                */
   /*                                                                         */
   /*                                                                         */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        Pointer to the message to store                                  */
   /*        Data Format, ( the Binary format is not supported)               */
   /*        Data Use Id,                                                     */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*         Validation of the diagnose execution.                           */
   /*                                                                         */
   /*                                                                         */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*        None.                                                            */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET dar_diagnose_generate_emergency(  T_DAR_INFO    *buffer_p,
                                              T_DAR_FORMAT  format,
                                              T_RVM_USE_ID  dar_use_id)
   {
      if (dar_gbl_var_p != NULL)
      {
         /* Process the diagnose emergency */
         dar_process_emergency(buffer_p, format, dar_use_id,
				DAR_EMERGENCY_RESET|DAR_NEW_ENTRY);

         return (RV_OK);
      }
      else
      {
         return (RV_NOT_READY);
      }

   } /* dar_diagnose_write */

 	   
   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_diagnose_write_emergency                        */
   /*                                                                         */
   /*    Purpose:  This function is called to store diagnose data in RAM      */
   /*              buffer when an emergency has been detected. Data is        */
   /*              written directly compared to dar_diagnode_write where data */
   /*              is sent to DAR via messages. Depending on the passed flags */
   /*              a RESET will be done.                                      */
   /*                                                                         */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        Pointer to the message to store                                  */
   /*        Data Format, ( the Binary format is not supported)               */
   /*        Data Use Id,                                                     */
   /*        Flags                                                            */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*         Validation of the diagnose execution.                           */
   /*                                                                         */
   /*                                                                         */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*        None.                                                            */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET dar_diagnose_write_emergency(  T_DAR_INFO    *buffer_p,
                                           T_DAR_FORMAT  format,
                                           T_RVM_USE_ID  dar_use_id,
                                           UINT32 flags)
   {
      if (dar_gbl_var_p != NULL)
      {
         /* Process the diagnose emergency */
         dar_process_emergency(buffer_p, format, dar_use_id, flags);
           
         return (RV_OK);
      }
      else
      {
         return (RV_NOT_READY);
      }

   } /* dar_diagnose_write_emergency */
   
#else

   /* ************************************************ */
   /*            THE DAR ENTITY IS DISABLED            */
   /* ************************************************ */

   #include "../../riviera/rv/rv_general.h"
   #include "../../riviera/rvm/rvm_gen.h"
   #include "../../riviera/rvm/rvm_priorities.h"
   #include "../../riviera/rvf/rvf_target.h"
   #include "../../riviera/rvf/rvf_i.h"
   #include "dar_gen.h"

   T_RV_RET dar_diagnose_swe_filter ( T_RVM_USE_ID  dar_use_id, 
                                      T_DAR_LEVEL   dar_level)
   {
         return (RV_OK);

   } /* dar_diagnose_swe_filter */
   

   T_RV_RET dar_diagnose_write(  T_DAR_INFO    *buffer_p,
                                 T_DAR_FORMAT  format,
                                 T_DAR_LEVEL   diagnose_info_level,
                                 T_RVM_USE_ID  dar_use_id)
   {
         return (RV_OK);

   } /* dar_diagnose_write */

        
   
   T_RV_RET dar_diagnose_generate_emergency(  T_DAR_INFO    *buffer_p,
                                              T_DAR_FORMAT  format,
                                              T_RVM_USE_ID  dar_use_id)
   {

         return (RV_OK);

   } /* dar_diagnose_generate_emergency */


   T_RV_RET dar_diagnose_write_emergency(  T_DAR_INFO    *buffer_p,
                                           T_DAR_FORMAT  format,
                                           T_RVM_USE_ID  dar_use_id,
                                           UINT32 flags)
   {
         return (RV_OK);
   } /* dar_diagnose_write_emergency */
    	   

#endif /* #ifdef RVM_DAR_SWE */

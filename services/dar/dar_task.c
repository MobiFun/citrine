/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_task.c                                                  */
/*                                                                          */
/*  Purpose:   This function is the main function. It contains the dar_core */
/*             which waits for messages or function calls.                  */
/*                                                                          */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date               Modification                                         */
/*  ------------------------------------                                    */
/*  16 October 2001    Create                                               */
/*                                                                          */
/*  Author     Stephanie Gerthoux                                           */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#ifdef RVM_DAR_SWE
   #ifndef _WINDOWS
      #include "../../include/config.h"
   #endif

   #include <string.h>
   #include "../../riviera/rvm/rvm_gen.h"
   #include "../../riviera/rvm/rvm_use_id_list.h"
   #include "dar_api.h"
   #include "dar_macro_i.h"
   #include "dar_handle_message.h"
   #include "dar_structs_i.h"
   #include "dar_error_hdlr_i.h"
   #include "dar_const_i.h"  

   #include "../ffs/ffs_api.h"

   /* Variables definitions */
   T_DAR_RECOVERY_STATUS status=0;

   /**** Global variables ****/
   /* Define a pointer to the Global Environment Control block  */
   extern T_DAR_ENV_CTRL_BLK *dar_gbl_var_p;

   /* Write buffer*/
   extern char dar_write_buffer[DAR_MAX_BUFFER_SIZE];

   /* index used in the circular buffer*/
   extern UINT16 dar_current_index;

   /* DAR previous status : to get the status of the last exception */
   extern UINT8  dar_previous_exception;

   /* Ram buffer that contains the Debug Unit register */
   extern UINT32  debug_RAM[DEBUG_UNIT_WORD_SIZE];

   /* X_dump buffer defined in the command file */
   /* This buffer contains the general register, the PC, the CPSR....*/
   #ifndef _WINDOWS
      extern INT32 xdump_buffer;
   #endif


   /***************************************************************************/
   /* Function         dar_core                                               */
   /*                                                                         */
   /* Description      Core of the dar task, which scans the dar mailbox and  */
   /*                  waits for messages. When a message arrives, it sends it*/
   /*                  to proper functions                                    */
   /*                                                                         */
   /*                                                                         */
   /***************************************************************************/

   T_RV_RET dar_core(void)
   {
      /* Declare local variables */
      /**** Structs ****/
      /* File descriptor type */
      T_FFS_FD  ffs_fd;
    
      /* Variables */
      T_RV_RET     error_status = RV_OK;
      T_RV_HDR     *msg_p = NULL;
      UINT16       received_event = 0x0000;
      char         dar_ffs[20];
      char         dar_dir_ffs[20];
      char         dar_subdir_ffs[20];
      BOOL         dar_ffs_error = FALSE;

      #ifndef _WINDOWS
         /* Define the Link register and the CPSR */
         INT32 *link_register_p = &(xdump_buffer) + 14;
         INT32 *dar_spsr_p      = &(xdump_buffer) + 16;
         UINT16      i;      
      #endif

      /* Dar_ffs file name */ 
      strcpy(dar_dir_ffs,"/var");
      strcpy(dar_subdir_ffs,"/var/dbg");

      #ifndef _WINDOWS
         strcpy(dar_ffs,"/var/dbg/dar");
      #else
         strcpy(dar_ffs,"\var\dbg\dar");
      #endif

      DAR_SEND_TRACE("DAR_TASK started",RV_TRACE_LEVEL_DEBUG_HIGH);

      /*---------------------------------*/
      /*               FFS               */
      /*---------------------------------*/

      /**** create the dar ffs directories ****/
	  if ((ffs_mkdir(dar_dir_ffs) != EFFS_OK) && (ffs_mkdir(dar_dir_ffs) != EFFS_EXISTS))
	  {
         DAR_SEND_TRACE("DAR entity can't create the '/var' folder into the flash",RV_TRACE_LEVEL_ERROR);
	  }

  	  if ((ffs_mkdir(dar_subdir_ffs) != EFFS_OK) && (ffs_mkdir(dar_subdir_ffs) != EFFS_EXISTS))
	  {
         DAR_SEND_TRACE("DAR entity can't create the '/var/dbg' folder into the flash",RV_TRACE_LEVEL_ERROR);
	  }


      /**** create the ffs file ****/
      ffs_fd = ffs_open(dar_ffs, FFS_O_CREATE | FFS_O_WRONLY | FFS_O_TRUNC | FFS_O_APPEND);

      if (ffs_fd < 0)
      {
         DAR_SEND_TRACE("DAR entity has received wrong file name or the flash is not formatted",RV_TRACE_LEVEL_WARNING);
         dar_ffs_error = TRUE;
      }
    
      #ifndef _WINDOWS

         /**** Save the RAM buffer into the FFS ****/
         /* save the buffer in 2 parts in order to have the information in chronological order */
         /* save the oldest information : from dar_current_index to the end of the buffer*/
         if ((ffs_write (ffs_fd,
                         (void *)(&dar_write_buffer + dar_current_index),
                         (DAR_MAX_BUFFER_SIZE - dar_current_index))) < 0 & (dar_ffs_error == FALSE))
         {
            DAR_SEND_TRACE("DAR entity can't saved the file in flash",RV_TRACE_LEVEL_WARNING);
         }

         /* save the rest of information: from the beginning of the buffer to dar_current_index */
         if ((ffs_write (ffs_fd,
                         (void *)(&dar_write_buffer),
                         (dar_current_index))) < 0 & (dar_ffs_error == FALSE))
         {
            DAR_SEND_TRACE("DAR entity can't saved the file in flash",RV_TRACE_LEVEL_WARNING);
         }

         /* Erase the dar_write_buffer */
         for (i=0;i<DAR_MAX_BUFFER_SIZE; i++)
         {
            dar_write_buffer[i] = 0;
         }

         /**** Save the X_dump_buffer into the Flash. ****/
         /* This buffer is defined in the command file */
         /* And the size of this buffer is defined in the gsm_cs_amd4_lj3_test.cmd */
         /* its size is 38*32 bits = 38*4 bytes = 152 bytes */
         if ((ffs_write (ffs_fd,
                         (void *)(&xdump_buffer),
                         (DAR_X_DUMP_BUFFER_SIZE))) < 0 & (dar_ffs_error == FALSE))
         {
            DAR_SEND_TRACE("DAR entity can't saved the X_dump_buffer in flash",RV_TRACE_LEVEL_WARNING);
         }

         /**** Save the Debug Unit Register into the Flash if necessary (Abort or prefetch)****/
         /* A Prefetch abort exception or a data abort exception is generated */
         #if ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
            if ((dar_previous_exception == DAR_EXCEPTION_PREFETCH_ABORT) ||
                (dar_previous_exception == DAR_EXCEPTION_DATA_ABORT))
            {
               /* Save the Debug Unit into the Flash */
               if ((ffs_write (ffs_fd,
                              (void *) &debug_RAM,
                              (4*DEBUG_UNIT_WORD_SIZE))) < 0 & (dar_ffs_error == FALSE))
               {
                  DAR_SEND_TRACE("DAR entity can't saved the DEBUG UNIT in flash",RV_TRACE_LEVEL_WARNING);
               }
            }
         #endif
         
      #else
         /* save the buffer in 2 parts in order to have the information in chronological order */
         /* save the oldest information : from dar_current_index to the end of the buffer*/
         if ((ffs_write (ffs_fd,
                         (&dar_write_buffer + dar_current_index),
                         (DAR_MAX_BUFFER_SIZE - dar_current_index))) != EFFS_OK)
         {
            DAR_SEND_TRACE("DAR entity can't saved the file in flash",RV_TRACE_LEVEL_WARNING);
         }
         /* save the rest of information: from the beginning of the buffer to dar_current_index */
         if ((ffs_write (ffs_fd,
                         (&dar_write_buffer),
                         (dar_current_index))) != EFFS_OK)
         {
            DAR_SEND_TRACE("DAR entity can't saved the file in flash",RV_TRACE_LEVEL_WARNING);
         }
         /* The X_dump_buffer  and the Debug Unit can't be saved on the PC */
      #endif

      /* Close the ffs file*/
      #ifndef _WINDOWS
         if ( ffs_close(ffs_fd) != EFFS_OK & (dar_ffs_error == FALSE))
         {
            DAR_SEND_TRACE("DAR entity has not closed the file",RV_TRACE_LEVEL_WARNING);
         }
      #else
         if ( ffs_close(ffs_fd) != EFFS_OK )
         {
            dar_error_trace(DAR_ENTITY_FILE_NO_CLOSE);
         }
      #endif

      #ifndef _WINDOWS
         /**** Save the LR and the SPSR when an exception has occured ****/
         if ((dar_previous_exception == DAR_EXCEPTION_PREFETCH_ABORT)||
             (dar_previous_exception == DAR_EXCEPTION_DATA_ABORT)    ||
             (dar_previous_exception == DAR_EXCEPTION_UNDEFINED)     ||
             (dar_previous_exception == DAR_EXCEPTION_SWI)           ||
             (dar_previous_exception == DAR_EXCEPTION_RESERVED))
         {
            /* Displays the Link register saved on exception  */
            DAR_SEND_TRACE_PARAM("Link register = ", *link_register_p,RV_TRACE_LEVEL_DEBUG_HIGH);

            /* Displays the User mode CPSR saved on exception */
            DAR_SEND_TRACE_PARAM("User mode SPSR before the exception=", *dar_spsr_p,RV_TRACE_LEVEL_DEBUG_HIGH); 
         }

         /* Displays the status of the last reset of the system */
         dar_recovery_get_status(&status);
      #endif

      /* ------------------------------------------- */
      /* loop to process messages                    */
      /* ------------------------------------------- */
      while (error_status == RV_OK)
      {
         /* Wait for all events. */
         received_event = rvf_wait(DAR_ALL_EVENT_FLAGS, DAR_NOT_TIME_OUT);
  
         if (received_event & DAR_TASK_MBOX_EVT_MASK)
         {
            /* Read the message in the Dar mailbox and handle it. */
            msg_p = (T_RV_HDR *) rvf_read_mbox(DAR_MBOX);			
            error_status = dar_handle_msg(msg_p);
         }
      }
  
      /* If one of the occured events is unexpected (due to an unassigned  */
      /* mailbox), then report an internal error.                          */
      if (received_event & ~(DAR_TASK_MBOX_EVT_MASK )) 
      {
         DAR_SEND_TRACE(" DAR ERROR (env). One of the occured events is unexpected ",
                         RV_TRACE_LEVEL_ERROR);

         error_status = RV_NOT_SUPPORTED;
      }

      /* If a memory error happened .. */
      if (error_status ==  RV_MEMORY_ERR)
      {
         dar_gbl_var_p->callBackFctError("DAR",
                                         RVM_MEMORY_ERR,
                                         0,
                                         " Memory Error : the DAR primitive memory bank is RED ");
      }

       return RV_OK;
   } /* dar_core */

#endif /* #ifdef RVM_DAR_SWE */

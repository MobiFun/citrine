/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_diagnose.c                                              */
/*                                                                          */
/*  Purpose:   This function contains the DAR diagnose functions            */
/*                                                                          */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date               Modification                                         */
/*  ------------------------------------                                    */
/*  18 October 2001    Create                                               */
/*                                                                          */
/*  Author     Stephanie Gerthoux                                           */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#ifdef RVM_DAR_SWE
   #ifndef _WINDOWS
     #include "../../bsp/timer.h"
   #endif

   #include "../../riviera/rv/rv_general.h"
   #include "../../riviera/rvm/rvm_gen.h"
   #include "../../riviera/rvm/rvm_priorities.h"
   #include "dar_api.h"
   #include "dar_structs_i.h"
   #include "dar_env.h"
   #include "../../riviera/rvf/rvf_target.h"
   #include "dar_const_i.h"
   #include "dar_macro_i.h"
   #include "dar_messages_i.h"
   #include "dar_error_hdlr_i.h"
   //#include "rvf/rvf_i.h"

   /**** Global variables ****/

   /* Define a pointer to the Global Environment Control block   */
   extern T_DAR_ENV_CTRL_BLK *dar_gbl_var_p;



   /***************************************************************************/
   /* Function         dar_search_group                                       */
   /*                                                                         */
   /* Description      This function checks if the use_id group_nb exists:    */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET dar_search_group(UINT16 group, UINT8 *index_p)
   {
      /* Declare local variables */   
      UINT8 i=0;

      /* Check if the DAR entity is started */
      if (dar_gbl_var_p != NULL )
      {
         /* Search in the dar_filter_array if the group exists */
         for (i=0;i< DAR_MAX_GROUP_NB; i++)
         {
            if(dar_gbl_var_p->dar_filter_array[i].group_nb == group)
            {
               *index_p=i;
               //DAR_SEND_TRACE_PARAM("dar filter_array index",*index_p,RV_TRACE_LEVEL_DEBUG_LOW);
               return(RV_OK);
            }
         }
         return(RV_NOT_SUPPORTED);
      }
      else
      {
         return(RV_NOT_READY);
      }
   }

   /***************************************************************************/
   /* Function         dar_add_group                                          */
   /*                                                                         */
   /* Description      This function research the index of the first free     */
   /*                  group                                                  */
   /***************************************************************************/
   T_RV_RET dar_add_group(UINT8 *index_p)
   {
      /* Declare local variables */
      UINT8 i=0;

      /* Check if the DAR entity is started */
      if (dar_gbl_var_p == NULL )
      {
         dar_error_trace(DAR_ENTITY_NOT_START);
         return(RV_NOT_READY);
      }

      /* Search in the dar_filter_array the first free group */
      for (i=0;i< DAR_MAX_GROUP_NB; i++)
      {
         if(dar_gbl_var_p->dar_filter_array[i].group_nb == DAR_INITIALIZATION_VALUE)
         {
            *index_p=i;
            return(RV_OK);
         }
      }
      return(RV_NOT_SUPPORTED);
   }


   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_send_write_data				      */
   /*                                                                         */
   /*    Purpose: This function is called to send write data in the DAR       */
   /*             mailbox.                                                    */
   /*    Input Parameters:                                                    */
   /*        Pointer to the message to store                                  */
   /*        Data Format,                                                     */
   /*        Data level,                                                      */
   /*        Data Use Id,                                                     */
   /*									      */
   /*    Output Parameters:                                                   */
   /*        Validation of the function execution.	                      */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None                                                             */
   /*                                                                         */
   /***************************************************************************/

   T_RV_RET dar_send_write_data (  T_DAR_INFO    *buffer_p,
                                   T_DAR_FORMAT  format,
                                   T_DAR_LEVEL   diagnose_info_level,
                                   T_RVM_USE_ID  dar_use_id)
   {
      /* Declare local variables */
      T_RVF_MB_STATUS      mb_status         = RVF_GREEN;
      T_DAR_WRITE_START   *write_data_p      = NULL;

      /************************** dar_send_write_data **********************/

      if (dar_gbl_var_p != NULL )
      {
         /* allocate the memory for the message to send */
         mb_status = rvf_get_buf (dar_gbl_var_p->mb_dar,
                                  sizeof (T_DAR_WRITE_START),
                                  (T_RVF_BUFFER **) (&write_data_p));

         /* If insufficient resources, then report a memory error and abort.*/
         if (mb_status == RVF_YELLOW)
         {
            /* deallocate the memory */
            rvf_free_buf((T_RVF_BUFFER *)write_data_p);
            dar_error_trace(DAR_ENTITY_NO_MEMORY);
            return (RV_NOT_SUPPORTED);
         }
         else
         if (mb_status == RVF_RED)
         {
            dar_error_trace(DAR_ENTITY_NO_MEMORY);
            return (RV_MEMORY_ERR);
         }

         /* fill the message id         */
         write_data_p->os_hdr.msg_id = DAR_WRITE_REQ;

         /* fill the addr source id     */
         write_data_p->os_hdr.src_addr_id = dar_gbl_var_p->addrId;

         /* fill the message parameters */
         write_data_p->data_write.char_p             = buffer_p ;
         write_data_p->data_write.data_format        = format;
         write_data_p->data_write.level              = diagnose_info_level;
         write_data_p->data_write.use_id.group_nb    = (dar_use_id>>16)& 0x7FFF;
         write_data_p->data_write.use_id.mask        = (dar_use_id)&0xFFFF;

         /* send the messsage to the DAR entity */
         rvf_send_msg (dar_gbl_var_p->addrId, 
                       write_data_p);

         return (RV_OK);
      }
      else
      {
          return(RV_NOT_READY);
      }


   } /* dar_send_write_data */

   /***************************************************************************/
   /* Function         dar_reset                                              */
   /*                                                                         */
   /* Description      This function is used to reset the system              */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        None                                                             */
   /*									      */
   /*    Output Parameters:                                                   */
   /*        Validation of the function execution.			      */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None                                                             */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET dar_reset(void)
   {
      #ifndef _WINDOWS
         /* Declare global variable*/
         volatile UINT16 *register_p;
         volatile UINT8   i;

      
         /* enable the Watchdog timer */
         TM_EnableWatchdog();

         /* Reset the system with the Watchdog */
         /* initialize the adress of the watchdog timer pointer */
         register_p = (volatile UINT16 *)WATCHDOG_TIM_MODE;

         /* Write the 0xF5 value to the Watchdog timer mode register to disable
	    the Watchdog */
         /* Note the bit 15 must be unchanged ( bit 15 = 1 -> 0x8000)*/
         *register_p =0x80F5;

         /* Wait a couple of time to be sure that this register has a new
	    value */
         for (i=0;i<100;i++);

	 /*
          * After having received 0xF5 in the Watchdog timer mode register,
	  * if the second write access is differennt from 0xA0, ARM core is
	  * reset.
	  *
          * The ARM HW core is reset + branch to adress 0x0000 ( SW reset)
          */
         *register_p=0x80F5;

         /* Wait until the ARM reset */
         while(1);
      #endif

      return(RV_OK);
   } /* dar_reset */


   /***************************************************************************/
   /*                                                                         */
   /* Function         dar_read_mbox                                          */
   /*                                                                         */
   /* Description      Called by the dar to read a buffer from its mailboxes. */
   /*                  when the Operating System is out                       */
   /* Input Parameters:                                                       */
   /*        None                                                             */
   /*									      */
   /* Output Parameters:                                                      */
   /*        NULL if the mailbox was empty, else the address of a buffer      */
   /*                                                                         */
   /***************************************************************************/

   void * dar_read_mbox (UINT8 mbox)
   {   
      // void * p_buf	= NULL;
      // T_RVF_INTERNAL_BUF * p_hdr;
	 
      /* Verify if DAR's global struct was set by RVM, then read the mailbox */
      if (dar_gbl_var_p != NULL )
	  return rvf_read_addr_mbox (dar_gbl_var_p->addrId, mbox);

      return NULL;
      
      // Check if the DAR entity is started 
      /*if (dar_gbl_var_p != NULL )
      {

         if ( OSTaskQFirst[dar_gbl_var_p->addrId][mbox] )// if the chained list is not empty 
         {   
            p_hdr = OSTaskQFirst[dar_gbl_var_p->addrId][mbox];
            OSTaskQFirst[dar_gbl_var_p->addrId][mbox] = p_hdr->p_next;

            p_hdr->p_next = NULL;

            #if RVF_ENABLE_BUF_LINKAGE_CHECK
              RVF_SET_BUF_UNLINKED(p_hdr);	// change buffer status 
            #endif

            p_buf = (UINT8 *)p_hdr + sizeof(T_RVF_INTERNAL_BUF);
         }
      }
      return (p_buf); */
   } // dar_read_mbox 
 
#else

   /* ******************************************************* */
   /*            THE DAR ENTITY IS DISABLED                   */
   /* ******************************************************* */
   #ifndef _WINDOWS
       #include "../../bsp/timer.h"
   #endif

   #include "../../riviera/rv/rv_general.h"
   #include "../../riviera/rvm/rvm_gen.h"
   #include "../../riviera/rvm/rvm_priorities.h"
   #include "../../riviera/rvf/rvf_target.h"
   //#include "rvf/rvf_i.h"

   /* Define the Watchdog timer register mode */
   #define WATCHDOG_TIM_MODE                           (0xFFFFF804)


   /***************************************************************************/
   /* Function         dar_reset                                              */
   /*                                                                         */
   /* Description      This function is used to reset the system              */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        None                                                             */
   /*									      */
   /*    Output Parameters:                                                   */
   /*        Validation of the function execution.			      */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None                                                             */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET dar_reset(void)
   {
      #ifndef _WINDOWS
         /* Declare global variable*/
         volatile UINT16 *register_p;
         volatile UINT8   i;

      
         /* enable the Watchdog timer */
         TM_EnableWatchdog();

         /* Reset the system with the Watchdog */
         /* initialize the adress of the watchdog timer pointer */
         register_p = (volatile UINT16 *)WATCHDOG_TIM_MODE;

         /* Write the 0xF5 value to the Watchdog timer mode register to disable
	    the Watchdog */
         /* Note the bit 15 must be unchanged ( bit 15 = 1 -> 0x8000)*/
         *register_p =0x80F5;

         /* Wait a couple of time to be sure that this register has a new
	    value */
         for (i=0;i<100;i++);

	 /*
          * After having received 0xF5 in the Watchdog timer mode register,
	  * if the second write access is differennt from 0xA0, ARM core is
	  * reset.
	  *
          * The ARM HW core is reset + branch to adress 0x0000 ( SW reset)
          */
         *register_p=0x80F5;

         /* Wait until the ARM reset */
         while(1);
      #endif

      return(RV_OK);
   } /* dar_reset */

#endif /* #ifdef RVM_DAR_SWE */

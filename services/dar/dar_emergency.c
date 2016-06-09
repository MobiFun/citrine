/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_emergency.c                                             */
/*                                                                          */
/*  Purpose:   This function is called when the DAR entity has detected     */
/*             an  emergency  ( Error or exception)                         */
/*                                                                          */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date               Modification                                         */
/*  ------------------------------------                                    */
/*  17 October 2001    Create                                               */
/*                                                                          */
/*  Author     Stephanie Gerthoux                                           */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#ifdef RVM_DAR_SWE
   /* ************************************************ */
   /*            THE DAR ENTITY IS ENABLED             */
   /* ************************************************ */

   #include <string.h>
   #include "../../riviera/rvm/rvm_gen.h"
   #include "dar_api.h"
   #include "dar_macro_i.h"
   #include "dar_messages_i.h"
   #include "dar_const_i.h"
   #include "dar_diagnose_i.h"
   #include "dar_msg_ft.h"

   #ifndef _WINDOWS
      #include "../../bsp/timer.h"
   #endif

   /**** Global variable ****/
   /* index used in the circular buffer*/
   extern UINT16 dar_current_index;

   /* Write buffer*/
   extern char dar_write_buffer[DAR_MAX_BUFFER_SIZE];

   /* Get the dar current status */
   extern T_DAR_RECOVERY_STATUS dar_current_status;

   /* Get the dar exception status */
   extern UINT8 dar_exception_status;

   /* Define a pointer to the Global Environment Control block   */
   extern T_DAR_ENV_CTRL_BLK *dar_gbl_var_p;

   /* Define the recovery buffer */
   extern UINT8   dar_recovery_buffer[DAR_RECOVERY_DATA_MAX_BUFFER_SIZE];

   /* Ram buffer that contains the Debug Unit register */
   extern UINT32  debug_RAM[DEBUG_UNIT_WORD_SIZE];


   /**** Extern functions ****/
   extern void * dar_read_mbox (UINT8 mbox);


   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_process_emergency                               */
   /*                                                                         */
   /*    Purpose:  This function is called to process emergency data and to   */
   /*              store them in RAM buffer when an emergency has been        */
   /*              detected.                                                  */
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
   T_RV_RET dar_process_emergency( T_DAR_INFO    *buffer_p,
                                   T_DAR_FORMAT  format,
                                   T_RVM_USE_ID  dar_use_id,
                                   UINT32 flags)
   {
      /* Declare local variables */
      UINT32    i = 0;
      UINT32    length = 0;
      T_RV_HDR   *msg_p = ( T_RV_HDR*) dar_read_mbox(DAR_MBOX);

      /**** Update the DAR recovery status ****/
      dar_current_status = DAR_EMERGENCY_SCUTTLING;

      /**** Empty the mail box before the reset of the system ****/
      while(msg_p != NULL)
      {
         /* If it's a Write message, store it in the RAM      */
         /* ( it is not interesting to store filter message ) */
         if ((msg_p->msg_id) == DAR_WRITE_REQ)
         {
            /* store the message in the RAM*/
            dar_write_data_in_buffer((T_DAR_WRITE_START *)msg_p);
         }

         /* free the Header of the message */
         rvf_free_buf((T_RVF_BUFFER *) msg_p);

         /* Read the next message */ 
         msg_p = ( T_RV_HDR*) dar_read_mbox(DAR_MBOX);

      } /* while (msg_p != NULL) */

      /**** Store emergency data in RAM buffer ****/            
      /* Diagnose string length */
      length = (UINT16) strlen(buffer_p);

      /**** Check if the DAR entity is started ****/
      if (dar_gbl_var_p != NULL )
      {
         /* Data format */
         dar_gbl_var_p->format = format;
      }

      if ( flags & DAR_NEW_ENTRY )
      {
      /**   Circular buffer to store data **/
      /* Add 0xFF to separate 2 strings */
      dar_write_buffer[dar_current_index] = 0xF; 
      DAR_PLUS_PLUS(dar_current_index); /* increment with wraparound */
      dar_write_buffer[dar_current_index] = 0xF; 
      DAR_PLUS_PLUS(dar_current_index); /* increment with wraparound */

      /* The group_nb is 16 bit length, and the buffer is an UINT8 length */
      /* So the group_nb must be stocked by dividing it in 2 parts */
      dar_write_buffer[dar_current_index] = ((dar_use_id>>16)& 0x7FFF)>>8;
                                   /*add the 8 first bits of the Use id group*/
      DAR_PLUS_PLUS(dar_current_index);
      dar_write_buffer[dar_current_index] = (dar_use_id>>16)& 0x7FFF;
                                   /*add the 8 last bits of the Use id group*/
      DAR_PLUS_PLUS(dar_current_index);

      /* The mask is 16 bit length, and the buffer is an UINT8 length */
      /* So the mask must be stocked by dividing it in 2 parts */
      dar_write_buffer[dar_current_index] = ((dar_use_id)&0xFFFF)>>8;
                                   /* add the 8 first bits of the Use id mask */
      DAR_PLUS_PLUS(dar_current_index);
      dar_write_buffer[dar_current_index] = (dar_use_id)&0xFFFF;
                                   /* add the 8 last bits of the Use id mask */
      DAR_PLUS_PLUS(dar_current_index);

      /* Add the dar_level data */
      dar_write_buffer[dar_current_index] = DAR_ERROR;
      DAR_PLUS_PLUS(dar_current_index);
      }

      /* circular buffer to store diagnose data in RAM buffer */
      for (i=0; i < length; i++ )
      {
         /* copy string in the RAM char by char*/
         dar_write_buffer[dar_current_index]=buffer_p[i];
      
         /* detection of the end of the buffer */
         /* When current = DAR_MAX_BUFFER_SIZE , current = 0 */
         DAR_PLUS_PLUS(dar_current_index); 
      }

      /* DAR information is redirected to standard trace */
      //DAR_SEND_TRACE("Circular buffer :",RV_TRACE_LEVEL_ERROR);
      //rvf_send_trace(buffer_p, length, NULL_PARAM, RV_TRACE_LEVEL_ERROR, DAR_USE_ID); 

      /* Trace the buffer (used for debug) */
      rvf_delay(RVF_MS_TO_TICKS(1000));
   

      /**** Check if the DAR entity is started ****/
      if (dar_gbl_var_p != NULL )
      {
         if (dar_gbl_var_p->entity_dar_callback != NULL )
         {
            /* Call the MMI callback function to save some parameters before
	       reset */
            dar_gbl_var_p->entity_dar_callback(dar_recovery_buffer,
					DAR_RECOVERY_DATA_MAX_BUFFER_SIZE);
         }
      }

      /* The system is reset if requested */
      if ( flags & DAR_EMERGENCY_RESET )
      {
        dar_reset();
      }
      return(RV_OK);
    
   } /* dar_process_emergency */


   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_exception_arm_undefined                         */
   /*                                                                         */
   /*    Purpose:  This function is called to process ARM undefined           */
   /*              instruction exception and to store this exception in the   */
   /*              RAM buffer.                                                */
   /*									      */
   /*    Input Parameters:                                                    */
   /*         None                                                            */
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
   void dar_exception_arm_undefined(void)
   {
      /* Declare local variables */
      T_DAR_INFO *buffer_p= " DAR Emergency exception : ARM undefined ";/*exception data to store*/

      /* Update the DAR recovery status */
      dar_current_status = DAR_EMERGENCY_SCUTTLING;

      /* Update the DAR exception status */
      dar_exception_status = DAR_EXCEPTION_UNDEFINED;

      /* Empty the mail box and store data in RAM buffer */
      dar_empty_mb_and_save_data(buffer_p); 

      /* Check if the DAR entity is started */
      if (dar_gbl_var_p != NULL )
      {
         if (dar_gbl_var_p->entity_dar_callback != NULL)
         {
            /* Call the MMI callback function to save some parameters before
	       reset */
            dar_gbl_var_p->entity_dar_callback(dar_recovery_buffer,
					DAR_RECOVERY_DATA_MAX_BUFFER_SIZE);
         }
      }

      dar_reset();

   } /* dar_exception_arm_undefined */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_exception_arm_swi                               */
   /*                                                                         */
   /*    Purpose:  This function is called to process ARM SW Interrupt        */
   /*              exception and to store this exception in the RAM buffer.   */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*         None                                                            */
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
   void dar_exception_arm_swi(void)
   {
      /* Declare local variables */
      T_DAR_INFO *buffer_p= " DAR Emergency exception : ARM SWI ";/*exception data to store*/

      /* Update the DAR recovery status */
      dar_current_status = DAR_EMERGENCY_SCUTTLING;

      /* Update the DAR exception status */
      dar_exception_status = DAR_EXCEPTION_SWI;

      /* Empty the mail box and store data in RAM buffer */
      dar_empty_mb_and_save_data(buffer_p);

      /* Check if the DAR entity is started */
      if (dar_gbl_var_p != NULL )
      {
         if ( dar_gbl_var_p->entity_dar_callback != NULL)
         {
            /* Call the MMI callback function to save some parameters before
	       reset */
            dar_gbl_var_p->entity_dar_callback(dar_recovery_buffer,
					DAR_RECOVERY_DATA_MAX_BUFFER_SIZE);
         }
      }

      dar_reset();

   } /* dar_exception_arm_swi */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_exception_arm_abort_prefetch                    */
   /*                                                                         */
   /*    Purpose:  This function is called to process ARM abort prefetch      */
   /*              exception and to store this exception in the RAM buffer.   */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*         None                                                            */
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
   void dar_exception_arm_abort_prefetch(void)
   {
      #ifndef _WINDOWS
         /* Local variable */
         UINT8 i;
      #endif

      #if ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || \
		(CHIPSET == 11) || (CHIPSET == 12))
	  /* Debug unit pointer */
	  volatile UINT32 *debug_register_p;
      #endif

      /* Declare local variables */
      T_DAR_INFO *buffer_p = " DAR Emergency exception : ARM abort prefetch";/*exception data to store*/
      
      /* Update the DAR recovery status */
      dar_current_status   = DAR_EMERGENCY_SCUTTLING;

      /* Update the DAR exception status */
      dar_exception_status = DAR_EXCEPTION_PREFETCH_ABORT;

      /* Empty the mail box and store data in RAM buffer */
      dar_empty_mb_and_save_data(buffer_p);

      /* Check if the DAR entity is started */
      if (dar_gbl_var_p != NULL )
      {
         if (dar_gbl_var_p->entity_dar_callback != NULL)
         {
            /* Call the MMI callback function to save some parameters before
	       reset */
            dar_gbl_var_p->entity_dar_callback(dar_recovery_buffer,
					DAR_RECOVERY_DATA_MAX_BUFFER_SIZE);
         }
      }

      #if ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || \
		(CHIPSET == 11) || (CHIPSET == 12))
	  /* Initialize the adress of the Debug Unit pointer */
	  debug_register_p = (volatile UINT32*) DAR_DEBUG_UNIT_REGISTER;
      #endif    

      #ifndef _WINDOWS
	  #if ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || \
		(CHIPSET == 11) || (CHIPSET == 12))
	     /* Save the Debug Unit into the RAM */
	     for (i=0; i<DEBUG_UNIT_WORD_SIZE; i++ )
	     {
		debug_RAM[i] = *debug_register_p;
		debug_register_p++;
	     }
	  #endif

         dar_reset();
      #endif

   } /* dar_exception_arm_abort_prefetch */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_exception_arm_abort_data                        */
   /*                                                                         */
   /*    Purpose:  This function is called to process ARM abort data exception*/
   /*              and to store this exception in the RAM buffer              */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*         None                                                            */
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
   void dar_exception_arm_abort_data(void)
   {
      #ifndef _WINDOWS
         /* Local variable */
         UINT8 i;
      #endif

      #if ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || \
		(CHIPSET == 11) || (CHIPSET == 12))
	  /* Debug unit pointer */
	  volatile UINT32 *debug_register_p;
      #endif

      /* Declare local variables */
      T_DAR_INFO *buffer_p= " DAR Emergency exception : ARM abort data";/*exception data to store*/

      /* Update the DAR recovery status */
      dar_current_status = DAR_EMERGENCY_SCUTTLING;

      /* Update the DAR exception status */
      dar_exception_status = DAR_EXCEPTION_DATA_ABORT;

      /* Empty the mail box and store data in RAM buffer */
      dar_empty_mb_and_save_data(buffer_p);

      /* Check if the DAR entity is started */
      if (dar_gbl_var_p != NULL )
      {
         if (dar_gbl_var_p->entity_dar_callback != NULL)
         {

            /* Call the MMI callback function to save some parameters before
	       reset */
            dar_gbl_var_p->entity_dar_callback(dar_recovery_buffer,
					DAR_RECOVERY_DATA_MAX_BUFFER_SIZE);
         }
      }

      #if ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || \
		(CHIPSET == 11) || (CHIPSET == 12))
	/* Initialize the adress of the Debug Unit pointer */
	debug_register_p = (volatile UINT32*) DAR_DEBUG_UNIT_REGISTER;
      #endif

      #ifndef _WINDOWS
	 #if ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || \
		(CHIPSET == 11) || (CHIPSET == 12))
	    /* Save the Debug Unit into the RAM */
	    for (i=0; i<DEBUG_UNIT_WORD_SIZE; i++ )
	    {
				debug_RAM[i] = *debug_register_p;
				debug_register_p++;
	    }
	 #endif

         dar_reset();
      #endif

   } /* dar_exception_arm_abort_data */

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_exception_arm_reserved                          */
   /*                                                                         */
   /*    Purpose:  This function is called to process ARM exception           */
   /*              and to store this exception in the RAM buffer              */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*         None                                                            */
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
   void dar_exception_arm_reserved(void)
   {
      /* Declare local variables */
      T_DAR_INFO *buffer_p= " DAR Emergency exception : ARM reserved";/*exception data to store*/

      /* Update the DAR recovery status */
      dar_current_status = DAR_EMERGENCY_SCUTTLING;

      /* Update the DAR exception status */
      dar_exception_status = DAR_EXCEPTION_RESERVED;
   
      /* Empty the mail box and store data in RAM buffer */
      dar_empty_mb_and_save_data(buffer_p);

      /* Check if the DAR entity is started */
      if (dar_gbl_var_p != NULL )
      {
         if (dar_gbl_var_p->entity_dar_callback != NULL)
         {
            /* Call the MMI callback function to save some parameters before
	       reset */
            dar_gbl_var_p->entity_dar_callback(dar_recovery_buffer,
					DAR_RECOVERY_DATA_MAX_BUFFER_SIZE);
         }
      }

      dar_reset();

   } /* dar_exception_arm_reserved */

  /****************************************************************************/
  /*                                                                          */
  /*    Function Name:   dar_exception                                        */
  /*                                                                          */
  /*    Purpose:  This function is called to vector exceptions to the correct */
  /*              handler                                                     */
  /*                                                                          */
  /*    Input Parameters:                                                     */
  /*         abort type  which come from int.s ( -> magic number)             */
  /*                                                                          */
  /*    Output Parameters:                                                    */
  /*         Validation of the diagnose execution.                            */
  /*                                                                          */
  /*                                                                          */
  /*                                                                          */
  /*    Revision History:                                                     */
  /*        None.                                                             */
  /*                                                                          */
  /****************************************************************************/
  void dar_exception(int abort_type)
  {
      switch (abort_type)
      {
         /* magic numbers come from int.s  There is no way to make compiler
          * derived constants out of them  
          */
         case 1:
         dar_exception_arm_undefined();
         break;
         case 2:
         dar_exception_arm_swi();
         break;
         case 3:
         dar_exception_arm_abort_prefetch();
         break;
         case 4:
         dar_exception_arm_abort_data();
         break;
         case 5:
         dar_exception_arm_reserved();
         break;
      }
  } /* dar_exception */

#else
   /* ************************************************ */
   /*            THE DAR ENTITY IS DISABLED            */
   /* ************************************************ */

   void dar_exception(int abort_type)
   {
	static char msg[] = "*ARM EXCEPTION #?";

	if (abort_type >= 0 && abort_type <= 9)
		msg[16] = abort_type + '0';
	freecalypso_raw_dbgout(msg);
	while (1)
		;
   } /* dar_exception */

#endif /* #ifdef RVM_DAR_SWE */

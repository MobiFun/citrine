/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_msg_ft.c                                                */
/*                                                                          */
/*  Purpose:   This function is called when the DAR entity receives a new   */
/*             message in its mailbox and wants to process the message.     */
/*                                                                          */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date                 Modification                                       */
/*  ------------------------------------                                    */
/*  17 October 2001    Create                                               */
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
   #include "dar_api.h"
   #include "dar_macro_i.h"
   #include "dar_messages_i.h"
   #include "dar_const_i.h"
   #include "dar_diagnose_i.h"
   #include "dar_error_hdlr_i.h"

   #ifndef _WINDOWS
      #include "../../bsp/timer.h"
   #endif

   /**** Global variable ****/
   /* index used in the circular buffer*/
   UINT16 dar_current_index = 0;
   /* Write buffer*/
   extern char dar_write_buffer[DAR_MAX_BUFFER_SIZE];

   /* Get the dar_current status */
   extern T_DAR_RECOVERY_STATUS dar_current_status;

   /* Define a pointer to the Global Environment Control block  */
   extern T_DAR_ENV_CTRL_BLK *dar_gbl_var_p;

   /* Define the recovery buffer */
   extern UINT8 dar_recovery_buffer[DAR_RECOVERY_DATA_MAX_BUFFER_SIZE];

   /**** Extern functions ****/
   extern  void * dar_read_mbox (UINT8 mbox);

   /***************************************************************************/
   /* Function         dar_filter_request                                     */
   /*                                                                         */
   /* Description      This function checks if the use_id group_nb exists:    */
   /*                      - if the group_nb exists, it adds the warning and  */
   /*                        debug masks in the dar_array_filter	      */
   /*                      - otherwise, this function add the new group_nb and*/
   /*                        the masks in the dar_array_filter                */
   /*                                                                         */
   /***************************************************************************/

   T_RV_RET dar_filter_request (T_DAR_FILTER_START *msg_p)
   {
      /* Declare local variables*/
      UINT8 index = 0; 

      /* check if the DAR entity is started */
      if (dar_gbl_var_p == NULL )
      {
         dar_error_trace(DAR_ENTITY_NOT_START);
         return(RV_NOT_READY);
      }

      /*** check if the group exists ****/
      /* If the group exists... */
      if(dar_search_group(msg_p->use_msg_parameter.group_nb,&index)== RV_OK)
      {
         /* Check the Dar level  */
         switch(msg_p->use_msg_parameter.level) 
         {
            case DAR_WARNING:
            {  /* The DAR entity wants to process Warning messages */
               /* add the mask_warning in the dar_filter array */
               dar_gbl_var_p ->dar_filter_array[index].mask_warning |= 
                                                 msg_p->use_msg_parameter.mask;
               dar_gbl_var_p ->dar_filter_array[index].mask_debug    = 0x00;

            break;
            }

            case DAR_DEBUG:
            {
               /* The DAR entity wants to process Debug messages */
               /* As the Warning messages are more important than debug */
               /* messages, it processes warning message too */

               /* add the mask_debug in the dar_filter array */
               dar_gbl_var_p ->dar_filter_array[index].mask_debug |= 
                                                 msg_p->use_msg_parameter.mask;

               /* add the mask_warning in the dar_filter array  */
               dar_gbl_var_p ->dar_filter_array[index].mask_warning |= 
                                                 msg_p->use_msg_parameter.mask;
            break;
            }

            case DAR_NO_DIAGNOSE:
            {
               /* The DAR entity doesn't want to process Diagnose messages */

               /* delete the mask_debug in the dar_filter array */
               dar_gbl_var_p ->dar_filter_array[index].mask_debug = 0x00;

               /* delete the mask_warning in the dar_filter array  */
               dar_gbl_var_p ->dar_filter_array[index].mask_warning = 0x00;
               break;
            }

            default:
            {
               /* Unknow level has been received */
               DAR_TRACE_WARNING("A DAR unknow level has been received ");
               break; 
            }
         } /* switch(msg_p->use_msg_parameter.level) */
      } /* if (search_group(msg_p->use_msg_parameter.group_nb,*index_gbl_p)== RV_OK) */

      else
      {
         /* if the group doesn't exist and if there is enough space in the
	    dar_filter_array */
         if ( dar_add_group(&index)== RV_OK)
         {
            /* ... add the group in the dar_array_filter */
            dar_gbl_var_p ->dar_filter_array[index].group_nb |=
				msg_p->use_msg_parameter.group_nb;
            /* Check the Dar level  */
            switch(msg_p->use_msg_parameter.level) 
            {
               case DAR_WARNING:
               {   /* The DAR entity wants to process Warning messages */
                   /* add the mask_warning in the dar_filter array */
                  dar_gbl_var_p ->dar_filter_array[index].mask_warning |= 
                                                 msg_p->use_msg_parameter.mask;
                  break;
               }

               case DAR_DEBUG:
               {
                  /* The DAR entity wants to process Debug messages */
                  /* As the Warning messages are more important than debug */
                  /* messages, it processes warning message too */

                  /* add the mask_debug in the dar_filter array */
                  dar_gbl_var_p ->dar_filter_array[index].mask_debug |= 
                                                 msg_p->use_msg_parameter.mask;

                  /* add the mask_warning in the dar_filter array  */
                  dar_gbl_var_p ->dar_filter_array[index].mask_warning |= 
                                                 msg_p->use_msg_parameter.mask;
               break;
               }

               default:
               {
                  /* Unknow level has been received */
                  DAR_TRACE_WARNING("A DAR unknow level has been received ");
                  break; 
                };
            } /* switch */
         }/* if ( add_group(msg_p->use_msg_parameter.group_nb,*index_gbl_p)== RV_OK)) */

         else 
         {
            /* There is not enough space in the dar_array_filter */
            DAR_TRACE_WARNING("Not enough space in the dar_array_filter for adding a new group ");
         }
      }
      return(RV_OK);
   }/* dar_filter_request   */


   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_write_data_in_buffer                            */
   /*                                                                         */
   /*    Purpose:  This function is called to store diagnose data in RAM      */
   /*              buffer.                                                    */
   /*									      */
   /*              note: In order to separate the different string, the data  */
   /*              are stored  as follows:                                    */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        Pointer to the message to store                                  */
   /*        Data Format,                                                     */
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
   T_RV_RET dar_write_data_in_buffer( T_DAR_WRITE_START *msg_p)
   {
      /* Local variables */
      UINT8   i = 0;
      UINT8 length = 0;
      
      /* Diagnose string length */
      length = (UINT16) strlen(msg_p->data_write.char_p);
   
      /***   Circular buffer to store data ***/
      /* Add 0xFF to separate 2 strings */
      dar_write_buffer[dar_current_index] = 0xF; 
      DAR_PLUS_PLUS(dar_current_index); /* increment with wraparound */
      dar_write_buffer[dar_current_index] = 0xF; 
      DAR_PLUS_PLUS(dar_current_index); /* increment with wraparound */


      /* The group_nb is 16 bit length, and the buffer is an UINT8 length */
      /* So the group_nb must be stocked by dividing it in 2 parts */
      dar_write_buffer[dar_current_index] =
				   (msg_p->data_write.use_id.group_nb)>>8;
                                   /*add the 8 first bits of the Use id group*/
      DAR_PLUS_PLUS(dar_current_index);
      dar_write_buffer[dar_current_index] = msg_p->data_write.use_id.group_nb;
                                   /*add the 8 last bits of the Use id group*/
      DAR_PLUS_PLUS(dar_current_index);
      /* The mask is 16 bit length, and the buffer is an UINT8 length */
      /* So the mask must be stocked by dividing it in 2 parts */
      dar_write_buffer[dar_current_index] = (msg_p->data_write.use_id.mask)>>8;
                                   /* add the 8 first bits of the Use id mask */
      DAR_PLUS_PLUS(dar_current_index);
      dar_write_buffer[dar_current_index] = msg_p->data_write.use_id.mask;
                                   /* add the 8 last bits of the Use id mask */
      DAR_PLUS_PLUS(dar_current_index);

      /* Add the dar_level data */
      dar_write_buffer[dar_current_index] = msg_p->data_write.level;
      DAR_PLUS_PLUS(dar_current_index);

      /* circular buffer to store diagnose data in RAM buffer */
      for (i=0; i < length; i++ )
      {
         /* copy string in the RAM char by char*/
         dar_write_buffer[dar_current_index]=msg_p->data_write.char_p[i];
         
         /* detection of the end of the buffer */
         /* When current = DAR_MAX_BUFFER_SIZE , current = 0 */
         DAR_PLUS_PLUS(dar_current_index);
      }

      /* DAR information is redirected to standard trace */
      //DAR_SEND_TRACE("circular buffer : ",RV_TRACE_LEVEL_DEBUG_HIGH);
      //rvf_send_trace(msg_p->data_write.char_p, length, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_HIGH, DAR_USE_ID); 

      return(RV_OK);

   } /* dar_send_write_data */

   /***************************************************************************/
   /* Function         dar_empty_mb_and_save_data                             */
   /*                                                                         */
   /* Description      This function is used to empty the mailbox and save    */
   /*                  data in the RAM buffer.                                */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        Pointer to the message to store                                  */
   /*									      */
   /*    Output Parameters:                                                   */
   /*        Validation of the function execution.			      */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None                                                             */
   /*                                                                         */
   /***************************************************************************/
   T_RV_RET dar_empty_mb_and_save_data(  T_DAR_INFO *buffer_p)
   {
      /* Declare local variables */
      UINT8       i        = 0;
      UINT16      length   = 0;
      T_RV_HDR    *msg_p   = ( T_RV_HDR*) dar_read_mbox(DAR_MBOX);

      /**** Empty the mail box ****/
      while(msg_p != NULL)
      {
         /* If it's a Write message, store it in the Ram */
         /* ( it is not interesting to store filter message ) */
         if ((msg_p->msg_id) == DAR_WRITE_REQ)
         {
            /* store themessage in the RAM*/
            dar_write_data_in_buffer((T_DAR_WRITE_START *)msg_p);
         }
         /* free the Header of the message */
         rvf_free_buf((T_RVF_BUFFER *) msg_p);

         /* Read the next message */ 
         msg_p = ( T_RV_HDR*) dar_read_mbox(DAR_MBOX);
      } /* while (msg_p != NULL) */

      /**** Store data in RAM buffer ****/            
      /* Diagnose string length */
      length = (UINT16) strlen(buffer_p);

      /**   Circular buffer to store data **/
      /* Add 0xFF to separate 2 strings */
      dar_write_buffer[dar_current_index] = 0xF; 
      DAR_PLUS_PLUS(dar_current_index); /* increment with wraparound */
      dar_write_buffer[dar_current_index] = 0xF; 
      DAR_PLUS_PLUS(dar_current_index); /* increment with wraparound */

      /* Add the dar_level data */
      dar_write_buffer[dar_current_index] = DAR_EXCEPTION;
      DAR_PLUS_PLUS(dar_current_index);
      /* circular buffer to store diagnose data in RAM buffer */
      for (i=0; i < length; i++ )
      {
         /* copy string in the RAM char by char*/
         dar_write_buffer[dar_current_index]=buffer_p[i];
         
         /* detection of the end of the buffer */
         /* When current = DAR_MAX_BUFFER_SIZE , current = 0 */
         DAR_PLUS_PLUS(dar_current_index);
      }

      return(RV_OK);

   }/* dar_empty_mb_and_save_data*/


#endif /* #ifdef RVM_DAR_SWE */



/******************************************************************************/
/*                                                                            */
/*              ------------------------------------------------              */
/*             |       WARNING       -      IMPORTANT           |             */
/*              ------------------------------------------------              */
/*                                                                            */
/*                                                                            */
/*    Function Name:   dar_lib                                                */
/*                                                                            */
/*    Purpose:  This function is only used in order to have a function in the */
/*               dar_lib when the DAR is NOT_COMPILED                         */
/*                                                                            */
/*    Input Parameters:                                                       */
/*         None                                                               */
/*                                                                            */
/*    Output Parameters:                                                      */
/*         NONE                                                               */
/*                                                                            */
/******************************************************************************/
void dar_lib(void)
{
}

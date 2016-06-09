/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_handle_message.c                                        */
/*                                                                          */
/*  Purpose:   This function is called when the DAR entity receives a new   */
/*             message in its mailbox.                                      */
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

   #include "../../riviera/rv/rv_general.h"
   #include "../../riviera/rvm/rvm_gen.h"
   #include "../../riviera/rvm/rvm_priorities.h"
   #include "dar_api.h"
   #include "dar_env.h"
   #include "dar_macro_i.h"
   #include "dar_messages_i.h"
   #include "dar_msg_ft.h"

   /***************************************************************************/
   /* Function         dar_handle_message                                     */
   /*                                                                         */
   /* Description      This function is called every time the DAR entity      */
   /*                  received a new message in its mailbox                  */
   /*                                                                         */
   /***************************************************************************/

   T_RV_RET dar_handle_msg(T_RV_HDR *msg_p)
   {
      /* Declare local variables */
      T_RV_RET status = RVF_GREEN;

      if (msg_p != NULL)
      {
         switch (msg_p->msg_id)
         {
            case DAR_FILTER_REQ:
            {
               //DAR_SEND_TRACE_PARAM("DAR Diagnose with the msg", msg_p->msg_id, RV_TRACE_LEVEL_DEBUG_LOW);
               /* process the dar filter */
               dar_filter_request((T_DAR_FILTER_START *)msg_p);
               break;
            }

            case DAR_WRITE_REQ:
            {
               //DAR_SEND_TRACE_PARAM("DAR Diagnose with the msg", msg_p->msg_id, RV_TRACE_LEVEL_DEBUG_LOW);
               /* process the dar to write */
               dar_write_data_in_buffer((T_DAR_WRITE_START *)msg_p);
               break;
            }
      
            default: 
            {
               /* Unknow message has been received */
               DAR_TRACE_WARNING("A DAR unknow message has been received ");
               break; 
            };
         } /* switch (msg_p->msg_id) */
  
         /* Free message buffer */
         status = rvf_free_buf((T_RVF_BUFFER *)msg_p);
         if (status != RVF_GREEN)
         {
            DAR_SEND_TRACE(" DAR ERROR (env). A wrong message is deallocated ",
                            RV_TRACE_LEVEL_ERROR);
         }
      } /* if (msg_p != NULL) */
  
      return RV_OK;
   }

#endif /* #ifdef RVM_DAR_SWE */

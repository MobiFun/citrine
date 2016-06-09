/**
 * @file   rtc_task.c
 *
 * Coding of the main RTC function : rtc_core
 * This function loop in the process message function for waiting messages.
 *
 * @author   Laurent Sollier (l-sollier@ti.com)
 * @version 0.1
 */

/*
 * History:
 *
 *   Date          Author       Modification
 *  ------------------------------------
 *  10/24/2001 L Sollier    Create
 *
 *
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */

#include "rtc_env.h"

#include "../../riviera/rv/rv_general.h"
#include "../../riviera/rvf/rvf_api.h"
#include "../../riviera/rvm/rvm_use_id_list.h"

#define RTC_MAILBOX_USED RVF_TASK_MBOX_0

/* External declaration until Riviera 1.6 is available*/
extern UINT8 rtc_handle_msg(T_RV_HDR*  msg_p);



/**
 * @name Functions implementation
 *
 */
/*@{*/

/**
 * function: rtc_core
 */
T_RV_RET rtc_core(void)
{   
   BOOLEAN error_occured = FALSE;
   T_RV_HDR * msg_ptr;
   UINT16 received_event;

   rvf_send_trace("RTC: Initialization", 19, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_HIGH, RTC_USE_ID );

   /* loop to process messages */
   while (error_occured == FALSE)
   {
      /* Wait for the necessary events. */
      received_event = rvf_wait ( 0xffff,0);

      if (received_event & RVF_TASK_MBOX_0_EVT_MASK)
      {
         /* Read the message */
         msg_ptr = (T_RV_HDR *) rvf_read_mbox(RTC_MAILBOX_USED);

         rtc_handle_msg(msg_ptr);
      }
   }

   return RV_OK;   
}

/**
 * @file	rtc_handle_msg.c
 *
 * Coding of the rtc_handle_msg function, which is called when the SWE
 * receives a new message.
 *
 * @author	Laurent Sollier (l-sollier@ti.com)
 * @version 0.1
 */

/*
 * History:
 *
 *	Date       	Author       Modification
 *  ------------------------------------
 *  10/24/2001 L Sollier    Create
 *
 *
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */


#include "../../riviera/rv/rv_general.h"
#include "../../riviera/rvf/rvf_api.h"
#include "../../riviera/rvm/rvm_use_id_list.h"


/* External declaration */
extern void rtc_process(T_RV_HDR * msg_ptr);

/**
 * @name Functions implementation
 *
 */
/*@{*/

/**
 * function: rtc_handle_msg
 */
UINT8 rtc_handle_msg(T_RV_HDR* msg_p)
{	
    if (msg_p != NULL)
    {
       rtc_process(msg_p);
    }

	return RV_OK;
}

/*@}*/

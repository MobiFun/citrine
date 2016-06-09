/**************************************************************************
 *			
 * rvt_api.c
 *
 * This module defines the interface of Trace Module
 *
 * (C) Texas Instruments, all rights reserved
 *
 * Version number	: 0.1
 *
 * History			: 0.1 (7/5/2000) - Created
 *
 * Date             : 7/5/2000
 *
 * Author           : Guido Pagana    g-pagana@ti.com															  *
***************************************************************************/



#include "../rv/general.h"
#include "../rv/rv_general.h"
#include "../rvf/rvf_api.h"
#include "../rvm/rvm_use_id_list.h"

#include "rvt_gen.h"
#include "rvt_def_i.h"
#include "rvt_env.h"
#include "rvt_env_i.h"

#include <string.h>


/*
** Trace User data base global variable
*/
T_RVT_USER_DB rvt_user_db [] = RVT_USER_ID_MAPPING;

/*
** Number of messages lost
*/
T_RVT_LOST_MSG  rvt_lost_msg_cpt = {0};

extern UINT8 rvf_trace_level;

extern UINT32 rvf_layer_mask;


/********************************************************************************/
/*                                                                              */
/*    Function Name:   rvt_set_trace_level                                      */
/*                                                                              */
/*    Purpose:         Updating the level of filtering, as well as the 32-bit   */
/*                     mask related to the software entities to be monitored,   */
/*                     according to the PC.                                     */
/*                                                                              */
/*    Note:            None.                                                    */
/*                                                                              */
/********************************************************************************/
void rvt_set_trace_level (T_RVT_BUFFER p_msg, UINT16 msg_length)
{

	/* Checking for an invalid PDU. */
	if ((p_msg == NULL) || \
		(msg_length != (sizeof (UINT8) + sizeof (UINT32))))
	{
		return;
	}

	/* Update the level of filtering. */
	if (*p_msg <= RV_TRACE_LEVEL_DEBUG_LOW)
	{
		rvf_trace_level = *p_msg;
	}

	/* Update the 32-bit mask related to the software entities to be monitored.
	Note that the 32-bit mask is transmitted LSB first. */
	rvf_layer_mask = *(++p_msg);
	rvf_layer_mask |= *(++p_msg) << 8;
	rvf_layer_mask |= *(++p_msg) << 16;
	rvf_layer_mask |= *(++p_msg) << 24;
}


/********************************************************************************/
/*                                                                              */
/*    Function Name:   rvt_send_trace_no_copy									*/
/*                                                                              */
/*    Purpose:         This function is used send trace messages without        */
/*                     copying.                                                 */
/*                                                                              */
/*    Note:                                                                     */
/*        WARNING!!!! The buffer containing the message has been allocated      */
/*        by the trace module!!!                                                */
/*                                                                              */
/********************************************************************************/

T_RVT_RET
rvt_send_trace_no_cpy(	T_RVT_BUFFER msg, 
			T_RVT_USER_ID user_id, 
			T_RVT_MSG_LG msg_length, 
			T_RVT_FORMAT msg_format)
{
	/* Shift the pointer back to write control values */
	T_RVT_TRACE_RQST * req_msg = (T_RVT_TRACE_RQST * )((UINT8*)msg - RVT_HEADER_SIZE);

	/* Check is Trace Module is running	*/
	if (rvt_module_state != RVT_STARTED)
	{
		(rvt_lost_msg_cpt.bit_mask).count++;
		(rvt_lost_msg_cpt.bit_mask).not_started = 1;
		return RVT_NOT_READY;
	}

    
	/* Add the id, size and format of the msg at the beginning */	
	
	req_msg->header.msg_id	= RVT_TRACE_RQST_ID;
	req_msg->format		= msg_format;
	req_msg->user_id	= user_id;
	req_msg->msg_length	= msg_length;

	if (rvf_send_msg(rvt_addr_id, (void *) req_msg) != RVF_OK)
	{
		(rvt_lost_msg_cpt.bit_mask).count++;
		(rvt_lost_msg_cpt.bit_mask).message_not_sent = 1;
		return RVT_INTERNAL_ERR;
	}
	return RVT_OK;
}		


/********************************************************************************/
/*                                                                              */
/*    Function Name:   send trace copy											*/
/*                                                                              */
/*    Purpose:         this function sends traces with copy						*/
/*																				*/
/*    Note:                                                                     */
/*        No check on id parameter !!!! Must be done in PC.                     */
/*                                                                              */
/********************************************************************************/
T_RVT_RET rvt_send_trace_cpy (T_RVT_BUFFER msg, 
				T_RVT_USER_ID user_id, 
				T_RVT_MSG_LG msg_length, 
				T_RVT_FORMAT msg_format)
{
	T_RVT_TRACE_RQST * req ;

	/* Check is Trace Module is running	*/
	if (rvt_module_state != RVT_STARTED)
	{
		(rvt_lost_msg_cpt.bit_mask).count++;
		(rvt_lost_msg_cpt.bit_mask).not_started = 1;
		return RVT_NOT_READY;
	}
	
	/* Allocate memory (msg_length + necessary room for the header)	*/
	if ((rvf_get_buf (rvt_mb_id, (UINT32) msg_length + RVT_HEADER_SIZE,	(T_RVF_BUFFER**) &req)) == RVF_RED)
	{
		(rvt_lost_msg_cpt.bit_mask).count++;
		(rvt_lost_msg_cpt.bit_mask).insufficient_resources = 1;
		return RVT_MEMORY_ERR;
	}

 	/* Add the id, size and format of the msg at the beginning */	
	req->header.msg_id	= RVT_TRACE_RQST_ID;
	req->format		= msg_format;
	req->user_id		= user_id;
	req->msg_length		= msg_length;

	/* Copy the message	*/
	memcpy( (UINT8*)req + RVT_HEADER_SIZE, msg, msg_length);  

	/*
	** Send it as a message to Trace Task
	*/
	if (rvf_send_msg(rvt_addr_id, (void *) req) != RVF_OK)
	{
		(rvt_lost_msg_cpt.bit_mask).count++;
		(rvt_lost_msg_cpt.bit_mask).message_not_sent = 1;
		rvf_free_buf (req);
		return RVT_INTERNAL_ERR;
	}
	return RVT_OK;
}



/********************************************************************************/
/*                                                                              */
/*    Function Name:   rvt_register_id											*/
/*                                                                              */
/*    Purpose:         function rvt_register id									*/
/*					   returns 0 if string not found, else the number			*/
/*					   stocked in the array										*/
/*                                                                              */
/*    Note:                                                                     */
/*        uses two arrays, one that stockes the messages,and another for the	*/
/*        correspondent values.                                                 */
/*                                                                              */
/********************************************************************************/

T_RVT_RET rvt_register_id(T_RVT_NAME name[], T_RVT_USER_ID *rvt_id, RVT_CALLBACK_FUNC callback)
{
	UINT8 i;

	/*looks in the array if the message is stocked */
	for (i=0;rvt_user_db[i].user_id!=RVT_INVALID_HEADER;i++)
	{
	 if (rvt_name_cmp(rvt_user_db[i].user_name,name))
	 {
	 	*rvt_id = rvt_user_db[i].user_id;
	 	rvt_user_db[i].rx_callback_func = callback;
	 	return(RVT_OK);
	 }			
	}
	
	/*else returns zero */
	return (RVT_INVALID_PARAMETER);

}




/********************************************************************************/
/*                                                                              */
/*    Function Name:   rvt_mem_alloc											*/
/*                                                                              */
/*    Purpose:         this function allocates a buffer for tracing.			*/
/*																				*/
/*    Note:                                                                     */
/*        None.                                                                 */
/*                                                                              */
/********************************************************************************/
T_RVT_RET rvt_mem_alloc(T_RVT_USER_ID user_id, T_RVT_MSG_LG buffer_length, T_RVT_BUFFER * buff)
{
	if (rvt_module_state != RVT_STARTED)
	{
		*buff = NULL;
		(rvt_lost_msg_cpt.bit_mask).count++;
		(rvt_lost_msg_cpt.bit_mask).not_started = 1;
		return RVT_NOT_READY;
	}

	if (rvf_get_buf (rvt_mb_id, (UINT32) buffer_length + RVT_HEADER_SIZE, (T_RVF_BUFFER**) buff) == RVF_RED)
	{
		*buff = NULL;
		(rvt_lost_msg_cpt.bit_mask).count++;
		(rvt_lost_msg_cpt.bit_mask).insufficient_resources = 1;
		return RVT_MEMORY_ERR;
	}
	*buff = *buff + RVT_HEADER_SIZE;
	return RVT_OK;
}


/********************************************************************************/
/*                                                                              */
/*    Function Name:   rvt_mem_free												*/
/*                                                                              */
/*    Purpose:      this function frees an allocated buffer for the trace task	*/
/*																				*/
/*    Note:                                                                     */
/*        None.                                                                 */
/*                                                                              */
/********************************************************************************/
T_RVT_RET rvt_mem_free(T_RVT_BUFFER msg)
{

	/*
	** Shift the pointer back to write control values
	*/
	char *buff = (char*) msg - RVT_HEADER_SIZE;

	// Deallocates the buffer
	if (rvf_free_buf((T_RVF_BUFFER*) buff) == RVF_MEMORY_ERR)
	{
		return RVT_MEMORY_ERR;
	}
	return RVT_OK;
}


/*
** User function used to compare name
*/
BOOLEAN rvt_name_cmp( char * str1, char * str2)
{
	UINT8 i;
	
	for ( i = 0; (str1[i] == str2[i]) && (str1[i] != 0) && (str2[i] != 0) && (i < RVT_NAME_MAX_LEN); i++ );
	if ( i == RVT_NAME_MAX_LEN)
	{	return TRUE;
	}
	if ( (str1[i] == 0) && (str2[i] == 0) )
	{	return TRUE;
	}
	return FALSE;
}

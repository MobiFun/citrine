/****************************************************************************/
/*                                                                          */
/*  Name        rvf_msg.c                                                */
/*                                                                          */
/*  Function    this file contains rvf message handling functions            */
/*                                                                          */
/*  Version		0.1															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  18-March-2002															*/
/*																			*/
/* (C) Copyright 2002 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "rvf_api.h"
#include "rvf_i.h"
#include "../rvm/rvm_use_id_list.h"
#include "../rvm/rvm_api.h"	// RVM_TMS_MSG & RVM_MSG_OFFSET

// PRIVATE
static const UINT8	BOUNDBUFVAL=20; 
T_RVF_RET _send_msg (T_RVF_G_ADDR_ID addr_id, void *msg, UINT8 mbox, UINT8 bindbuf);

/*******************************************************************************
**
** Function         rvf_get_msg_buf
**
** Description      Request for a message buffer.  
**					Automatically update the message id of the buffer
**
**
** Returns          T_RVF_MB_STATUS:	RVF_GREEN if everything is ok,
**										RVF_YELLOW if watermark limit has been reached,
**										RVF_RED if max size has been reached (does not return a buffer)
**
*******************************************************************************/
T_RVF_MB_STATUS rvf_get_msg_buf(T_RVF_MB_ID mb_id,
								UINT32 message_size,
								T_RVF_MSG_ID msg_id,
								T_RVF_MSG ** pp_msg)
{

	T_RVF_MB_STATUS mb_status;


	mb_status = rvf_get_buf(mb_id,message_size,(T_RVF_BUFFER**) pp_msg);

	if (mb_status != RVF_RED)
	{
		(*pp_msg)->msg_id = msg_id;
	}

	return mb_status;
}



/*******************************************************************************
**
** Function         rvf_fre_msg
**
** Description      Deallocate a message buffer
**
**
** Returns          T_RVF_RET:	T_RVF_OK if succesful,...
**
*******************************************************************************/
T_RVF_RET rvf_free_msg(T_RVF_MSG *p_msg)
{
	return rvf_free_buf( (T_RVF_BUFFER *)  p_msg);
}

T_RVF_RET rvf_free_timer_msg(T_RVF_MSG *p_msg)
{
	return rvf_free_buf( (T_RVF_BUFFER *)  p_msg);
}


/*******************************************************************************
**
** Function         rvf_send_msg
**
** Description      Called by applications to send a buffer to a SWE.
**
** Returns			RVF_OK if successful, else an error code.
**
** Comments			Priority is defined the inverse of NU. for legacy reasons
*******************************************************************************/
T_RVF_RET rvf_send_msg (T_RVF_G_ADDR_ID addr_id, void *msg) {
	return _send_msg ( addr_id, msg, 0, FALSE);
}

T_RVF_RET rvf_send_priority_msg (T_RVF_G_ADDR_ID addr_id, void *msg) {
	return _send_msg ( addr_id, msg, 1, TRUE);
}

T_RVF_RET _send_msg (T_RVF_G_ADDR_ID addr_id, void *msg, UINT8 mbox, UINT8 bindbuf) {
    T_RVF_INTERNAL_BUF    *p_hdr;
	T_RVF_INTERNAL_BUF*	cur=NULL;
	UINT8 i=0;   
	T_RVF_G_ADDR_ID tid=RVF_INVALID_ADDR_ID;

    /* If task non-existant or not started, drop buffer */
    if ((addr_id >= MAX_RVF_G_ADDR_ID)) {
        rvf_send_trace( "RVF: rvf_send_msg(): invalid taskid", 35, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
        rvf_free_buf (msg);
        return RVF_INVALID_PARAMETER;
    }
	if((tid=resolveHostAddrId(addr_id))==RVF_INVALID_ADDR_ID) {
		rvf_free_buf (msg);
		return RVF_INTERNAL_ERR;
	}
	if(!pRtAddrIdTable[tid]) {
		rvf_free_buf (msg);
		return RVF_INTERNAL_ERR;
	}

	// Reject all msg or only continuous tm msg if bound buf limit
	if(bindbuf) {
		for(i=0, cur=pRtAddrIdTable[tid]->OSTaskQFirst[mbox];
				 cur && cur->p_next; cur=cur->p_next, i++) ;
		if(((T_RV_HDR*)msg)->msg_id==RVM_TMS_MSG) {
			if(i>=BOUNDBUFVAL && ((T_RVF_TMS_MSG*)msg)->cont) {
				rvf_free_buf(msg);
				return RVF_MEMORY_ERR;
			}
		} else {
			if(i>=BOUNDBUFVAL) {
				rvf_free_buf(msg);
				return RVF_MEMORY_ERR;
			}
		}
	}

#if RVF_ENABLE_BUF_CORRUPTION_CHECK	
	/* check for buffer corruption */
	if (_rvf_chk_buf_damage(msg) == TRUE)
    {	rvf_send_trace( "RVF: rvf_send_msg(): buffer corrupted", 37, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
	}
#endif

	/* update the addr_id field of the message */
	((T_RV_HDR *)msg)->dest_addr_id = addr_id;
	p_hdr = USER2MEM(msg);

#if RVF_ENABLE_BUF_LINKAGE_CHECK
    /* check if the buffer has been already enqueued */
	if ( RVF_BUF_IS_LINKED( p_hdr) )
	{
		rvf_send_trace( "RVF: rvf_send_msg(): buffer already enqueued", 44, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
        return RVF_MEMORY_ERR;
    }
#endif

    rvf_disable(8);		/* enter critical section */

	/* chained the buffer in the mailbox list */
    /*if (OSTaskQFirst[task_id][mbox])	{
		OSTaskQLast[task_id][mbox]->p_next = p_hdr;
	} else {
		OSTaskQFirst[task_id][mbox] = p_hdr;
	}
    OSTaskQLast[task_id][mbox] = p_hdr;
	p_hdr->p_next = NULL;*/

	if (pRtAddrIdTable[tid]->OSTaskQFirst[mbox]) {
		pRtAddrIdTable[tid]->OSTaskQLast[mbox]->p_next = p_hdr;
	} else	{	
		pRtAddrIdTable[tid]->OSTaskQFirst[mbox] = p_hdr;
	}
    pRtAddrIdTable[tid]->OSTaskQLast[mbox] = p_hdr;

	p_hdr->p_next = NULL;

#if RVF_ENABLE_BUF_LINKAGE_CHECK
	RVF_SET_BUF_LINKED(p_hdr);	/* change buffer status */
#endif

    rvf_enable(); 

	/* send a rvf event  */
    rvf_send_event(tid, (UINT16) (EVENT_MASK(mbox)) );

	return RVF_OK;
}

// Poss. Deprecate ! [Only used by: rvf.Buffer::_rvf_send_msg_to_mbox() ]
// May conflict with "msg::rvf_send_priority_msg()"
T_RVF_RET rvf_adapt_send_msg (T_RVF_G_ADDR_ID addr_id, void *msg, UINT8 mbox) {
	return _send_msg ( addr_id, msg, mbox, FALSE); 
}

/**
 *                                                                          
 *  @file	rvm_task.c                                              
 *                                                                          
 *  This file contains the main RVN function: rvm_task.			
	It will initialize the RVM and then wait for messages.
 *
 * @author	Cristian Livadiotti (c-livadiotti@ti.com)
 * @version	0.2
 *
 */

/*
 * Revision History:																			
 *
 * 06/04/2000	Cristian Livadiotti		Create.
 * 10/22/2001	David Lamy-Charrier		Update for new Riviera 1.6.
 *																			
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */
# include <stdio.h>

#include "../rvf/rvf_api.h"

#include "rvm_gen.h"
#include "rvm_api.h"
#include "rvm_i.h"
#include "../rvf/rvf_i.h"			/* only for pRtAddrIdTable[] */
#include "rvm_use_id_list.h"


extern T_RVM_KNOWN_SWE	* rvm_swe_array;  /* for start() */

#ifdef _CONSOLE
	void	_callerProxy(T_RVM_APPLI_RESULT *p_msg_res) ; //A-M-E-N-D-E-D!
#endif

T_RVM_RETURN	_start_group(T_RVF_G_ADDR_ID gid, UINT8* grp);
T_RVM_RETURN	_t3_start(T_RVF_G_ADDR_ID gid);
T_RVM_RETURN	_stop_t2_swe(T_RVM_STOP_MSG* p_msg);
T_RVM_RETURN	_stop_t3_swe(T_RVM_STOP_MSG* p_msg);

/*******************************************************************************
** Function         rvm_init
**
** Description      Initialize all the RVM
**					
*******************************************************************************/
BOOLEAN rvm_init()
{	
	T_RVF_MB_PARAM mb_params;
	mb_params.size		=	RVM_PRIM_MB_SIZE;
	mb_params.watermark =	RVM_PRIM_MB_WATERMARK;

	if(rvm_mem_bank!=RVF_INVALID_MB_ID) return TRUE;
	/* create rvm main Memory Bank */
	if ( rvf_create_mb( RVM_PRIM_MB, mb_params, &rvm_mem_bank) != RVF_OK)
	{	
		// TO DO: remove this call since the RVT software entity has not been started yet.
		RVM_TRACE_ERROR( "RVM_init: unable to create the RVM Main mem bank");
		return FALSE;
	}

	mb_params.size		= RVM_STACK_MB_SIZE;
	mb_params.watermark = RVM_STACK_MB_WATERMARK;

	/* create a second Memory Bank used for stack allocation */
	if ( rvf_create_mb( RVM_STACK_MB, mb_params, &rvm_stack_mem_bank) != RVF_OK)
	{	
		// TO DO: remove this call since the RVT software entity has not been started yet.
		RVM_TRACE_ERROR( "RVM_init: unable to create the RVM stacks mem bank");
		return FALSE;
	}

	mb_params.size		=	RVM_SYS_MB_SIZE;
	mb_params.watermark =	RVM_SYS_MB_WATERMARK;

	if(rvm_sys_mem_bank!=RVF_INVALID_MB_ID) return TRUE;
	/* create rvm main Memory Bank */
	if ( rvf_create_mb( RVM_SYS_MB, mb_params, &rvm_sys_mem_bank) != RVF_OK)
	{	
		// TO DO: remove this call since the RVT software entity has not been started yet.
		RVM_TRACE_ERROR( "RVM_init: unable to create the RVM SYSTEM mem bank");
		return FALSE;
	}

	mb_params.size		=	RVM_TIMER_MB_SIZE;
	mb_params.watermark =	RVM_TIMER_MB_WATERMARK;

	if(rvm_timer_mem_bank!=RVF_INVALID_MB_ID) return TRUE;
	// create rvm main Memory Bank 
	if ( rvf_create_mb( RVM_TIMER_MB, mb_params, &rvm_timer_mem_bank) != RVF_OK)
	{	
		// TO DO: remove this call since the RVT software entity has not been started yet.
		RVM_TRACE_ERROR( "RVM_init: unable to create the RVM TIMER mem bank");
		return FALSE;
	}

	mb_params.size		=	RVM_NOTIFY_MB_SIZE;
	mb_params.watermark =	RVM_NOTIFY_MB_WATERMARK;

	if(rvm_tm_notify_mem_bank!=RVF_INVALID_MB_ID) return TRUE;
	// create rvm main Memory Bank 
	if ( rvf_create_mb( RVM_NOTIFY_MB, mb_params, &rvm_tm_notify_mem_bank) != RVF_OK)
	{	
		// TO DO: remove this call since the RVT software entity has not been started yet.
		RVM_TRACE_ERROR( "RVM_init: unable to create the RVM TIMER NOTIFY MSG mem bank");
		return FALSE;
	}

	return TRUE;
}

/*******************************************************************************
** Function         rvm_task
**
** Description      Core of the RVM task, which initiliazes the RVM and 
**					waits for messages.
**					
*******************************************************************************/
void rvm_task (UINT32 param)
{	BOOLEAN detectedError = FALSE;
	T_RV_HDR * msgPtr;

	/* init the SWEs database */
	if ( rvm_init_swe_db() != RVM_OK)
	{	/* lack of memory */
		detectedError = TRUE;
	}
//printf("RVManager core: %d %s\n", rvf_get_taskid(), rvf_get_taskname());

	/* loop to process messages */
	while (detectedError == FALSE)
	{
		/* Wait for the necessary event (msg in the rve mailbox and no timeout). */
		UINT16 recEvent = rvf_wait ( RVM_EXPECTED_EVENT, 0);
					
		/* If the expected event is received, then */
		if (recEvent & RVM_EXPECTED_EVENT)	{
			/* Read the message in the rve mailbox */
			msgPtr = (T_RV_HDR *) rvf_read_mbox (RVM_MAILBOX);
			if(msgPtr) {
			/* Determine the input message type */
				switch (msgPtr->msg_id)	{
				case (RVM_START_APPLI): {
						rvm_launch_appli((T_RVM_MSG*)msgPtr); 
						rvf_free_buf(msgPtr);
						break;				
					}
					case (RVM_STOP_APPLI): 	{ // still keep original concept
						rvm_shut_down_appli((T_RVM_MSG*)msgPtr);
						rvf_free_buf(msgPtr);
					break;				
				}
					case (RVM_STOP_MSG): { // new concept, eg. reverse init/start concept
						if(((T_RVM_STOP_MSG*)msgPtr)->status==SWE_RUNNING) {
							rvm_stop_appli((T_RVM_STOP_MSG*) msgPtr);
							rvf_free_buf(msgPtr);
						} else {
							rvm_swe_has_stopped((T_RVM_STOP_MSG*) msgPtr);
							rvf_free_buf(msgPtr);
						}

					break;				
				}
#ifdef _CONSOLE
				case (RVM_EVT_TO_APPLI): 	{       /*A-M-E-N-D-E-D! */
					/* proxy msg's sent by "rvm_snd_to_upper()"      
					/* There arrival here is evident that no parent or
					/* calling process, ie. No MMI exists and they were
					/* started by RVM itself.                          */
						RVM_TRACE_WARNING_PARAM("RVM: caller response msg", msgPtr->msg_id);

					_callerProxy((T_RVM_APPLI_RESULT*)msgPtr);
					rvf_free_buf(msgPtr);
					break;				
				}
#endif
				default:	{	
						RVM_TRACE_WARNING_PARAM("rvm_Task: [switch()] unrecognized msg", msgPtr->msg_id);

						rvf_free_buf(msgPtr);
						break;
				}
			}
			}
			/* free the received msg */
//			rvf_free_buf(msgPtr);  // may be used when STOP is completed
		} else {
			RVM_TRACE_WARNING_PARAM("RVM_task: unrecognized event", recEvent);
		}
	}
	RVM_TRACE_ERROR("RVM_task: unrecoverable error --> stopping RVM");
}

T_RVM_RETURN rvm_t2_proxy() {
	T_RVM_RETURN	error_status=RVM_OK;
	T_RV_HDR*		p_msg = NULL ;
	UINT16			rec_event;
	T_RVF_G_ADDR_ID gid=RVF_INVALID_ADDR_ID;
	UINT16			nbTmExp=0;
	UINT8			y=0,F_PRI=0, yield;

	RVM_TRACE_WARNING_PARAM("T2PROXY TACHE DE DEV", RV_TRACE_LEVEL_DEBUG_HIGH);

	rvf_start_timer(0, RVM_YIELD_T2_PS_TM, 1);
	yield=MAX_PARASITES;
	gid=rvf_get_taskid();
	
	while (error_status == RVM_OK) {
		
		rec_event = rvf_evt_wait(gid, 0xffff, 0xFFFFFFFFL); /* Wait (infinite) for all events. */

		if (rec_event & RVF_TASK_MBOX_1_EVT_MASK) {
			F_PRI=1;
			for(y=0; (p_msg=(T_RV_HDR *)rvf_read_addr_mbox (gid, 1)) && (y<yield); y++) {
//RVM_TRACE_WARNING_PARAM("T2 PRI MAILBOX !!!\n", RV_TRACE_LEVEL_DEBUG_HIGH);
				rvf_setRDV(gid, p_msg->dest_addr_id);
				switch(p_msg->msg_id) { 
					case RVM_TMS_MSG:
						if(pRtAddrIdTable[p_msg->dest_addr_id] && pRtAddrIdTable[p_msg->dest_addr_id]->handle_timer) {
						pRtAddrIdTable[p_msg->dest_addr_id]->handle_timer(p_msg);
						} else rvf_free_timer_msg(p_msg);
					break;
					case RVM_START_T2_MSG:
						yield=MAX_PARASITES;
						_start_group(gid, ((T_RVM_START_T2_MSG*)p_msg)->grp);
						rvf_free_buf(p_msg);
					break;
					case RVM_STOP_MSG:
						_stop_t2_swe((T_RVM_STOP_MSG*)p_msg);
					break;
					case RVM_RT_MOD_YIELD_T2_MSG:
						if(((T_RVM_RT_MOD_YIELD_T2_MSG*)p_msg)->val > 1) 
							rvf_start_timer(0, ((T_RVM_RT_MOD_YIELD_T2_MSG*)p_msg)->val, 1);
						rvf_free_buf(p_msg);
					break;
					default:
						if(pRtAddrIdTable[p_msg->dest_addr_id] && pRtAddrIdTable[p_msg->dest_addr_id]->handle_message) {
						pRtAddrIdTable[p_msg->dest_addr_id]->handle_message(p_msg);
						} else rvf_free_msg(p_msg);
					break;
				}
				rvf_setRDV(gid, RVF_INVALID_ADDR_ID);
			}
			yield=RVM_YIELD_T2_PRI_MSG_CNT;
		} 
		if (rec_event & RVF_TASK_MBOX_0_EVT_MASK ) {
//RVM_TRACE_WARNING_PARAM("T2 STND MAILBOX !!!\n", RV_TRACE_LEVEL_DEBUG_HIGH);
			if(p_msg=(T_RV_HDR *) rvf_read_mbox (0)) {
				rvf_setRDV(gid, p_msg->dest_addr_id);
				switch(p_msg->msg_id) { 
					case RVM_TMS_MSG:
						if(pRtAddrIdTable[p_msg->dest_addr_id] && pRtAddrIdTable[p_msg->dest_addr_id]->handle_timer) {
						pRtAddrIdTable[p_msg->dest_addr_id]->handle_timer(p_msg);
						} else rvf_free_timer_msg(p_msg);
							
						//rvf_free_buf(p_msg);
					break;
					case RVM_START_T2_MSG:
						_start_group(gid, ((T_RVM_START_T2_MSG*)p_msg)->grp);
						rvf_free_buf(p_msg);
					break;
					case RVM_STOP_MSG:
						_stop_t2_swe((T_RVM_STOP_MSG*)p_msg); // preserve 
					break;
					default:
						if(pRtAddrIdTable[p_msg->dest_addr_id] && pRtAddrIdTable[p_msg->dest_addr_id]->handle_message) {
						pRtAddrIdTable[p_msg->dest_addr_id]->handle_message(p_msg);
						} else rvf_free_msg(p_msg);
					break;
				}
				rvf_setRDV(gid, RVF_INVALID_ADDR_ID);
			} 
		} 
		if( rec_event & RVF_TIMER_0_EVT_MASK ) {
//RVM_TRACE_WARNING_PARAM("T2 YIELD !!!\n", RV_TRACE_LEVEL_DEBUG_HIGH);
			rvf_yield();
		}
		if( rec_event & RVF_TIMER_1_EVT_MASK || rec_event & RVF_TIMER_2_EVT_MASK   ) {
			
			RVM_TRACE_WARNING_PARAM("RVM: Forbidden timer usage for type 2 entities!\n     Please use rvf_add_timer()", RV_TRACE_LEVEL_DEBUG_HIGH);
		}
		if(rec_event & RVF_TIMER_3_EVT_MASK) { 

			/*nbTmExp=0; //rvf_update_timer_list((pRtAddrIdTable[gid]->p_tm_q));

			while(nbTmExp-- > 0) {
				p_msg=0;//(T_RV_HDR*)rvf_get_expired_entry((pRtAddrIdTable[gid]->p_tm_q));
				if(p_msg->dest_addr_id)
				  pRtAddrIdTable[p_msg->dest_addr_id]->handle_timer(p_msg);
				else RVM_TRACE_WARNING_PARAM("T3PROXY NO TM DESTINATION", RV_TRACE_LEVEL_DEBUG_HIGH);
			}*/
		}
		F_PRI=0;
	}
	
	/*if (error_status == INVKR_MEMORY_ERR) {
		invkr_env_ctrl_blk_p->error_ft("INVKR", RVM_MEMORY_ERR, 0,
						  " Memory Error : the INVKR primitive memory bank is RED ");
	}*/

	RVM_TRACE_WARNING_PARAM("T2PROXY ERROR: CORE TERMINATION", RV_TRACE_LEVEL_DEBUG_HIGH);

	/* BEFORE returning free resources !!!		*/
	return RVM_OK;
}

T_RVM_RETURN rvm_t3_proxy() { 
	T_RVM_RETURN	error_status=RVM_OK;
	T_RV_HDR*		p_msg = NULL ;
	UINT16			rec_event;
	T_RVF_G_ADDR_ID gid;
	UINT8			swe_i=0;
	UINT16			nbTmExp=0;
	T_RV_HDR*		p_hdr=NULL; 
	UINT8			yield=250, y=0; /* arguable whether T3 needs Yield. Hence, high nb */
	UINT8			F_PRI=0;

	RVM_TRACE_WARNING_PARAM("T3PROXY TACHE DE DEV", RV_TRACE_LEVEL_DEBUG_HIGH);
//printf("t2proxy: %d %s\n", rvf_get_taskid(), rvf_get_taskname());
	gid=rvf_get_taskid();
	if(gid) _t3_start(gid);

	RVM_TRACE_WARNING_PARAM("T3PROXY START()", RV_TRACE_LEVEL_DEBUG_HIGH);

	while (error_status == RVM_OK) {
		
		rec_event = rvf_evt_wait(gid, 0xffff, 0xFFFFFFFFL); /* Wait (infinite) for all events. */

		if (rec_event & RVF_TASK_MBOX_1_EVT_MASK) { 
			F_PRI=1;
			for(y=0; (p_msg=(T_RV_HDR *)rvf_read_addr_mbox (gid, 1)) && (y<yield); y++) {
				if(p_msg->msg_id==RVM_TMS_MSG) {
					if(pRtAddrIdTable[p_msg->dest_addr_id] && pRtAddrIdTable[p_msg->dest_addr_id]->handle_timer) {
					pRtAddrIdTable[p_msg->dest_addr_id]->handle_timer(p_msg);
					} else rvf_free_timer_msg(p_msg);
				} else if(p_msg->msg_id==RVM_STOP_MSG) {
					_stop_t3_swe((T_RVM_STOP_MSG*)p_msg);
				} else { 
					if(pRtAddrIdTable[p_msg->dest_addr_id] && pRtAddrIdTable[p_msg->dest_addr_id]->handle_message) {
						pRtAddrIdTable[p_msg->dest_addr_id]->handle_message(p_msg);
					} else rvf_free_msg(p_msg);
				}
			}
		} 
		if (rec_event & RVF_TASK_MBOX_0_EVT_MASK && !F_PRI) {
			if(p_msg=(T_RV_HDR *) rvf_read_addr_mbox (gid, 0)) {
				if(p_msg->msg_id==RVM_TMS_MSG) {
					if(pRtAddrIdTable[p_msg->dest_addr_id] && pRtAddrIdTable[p_msg->dest_addr_id]->handle_timer) {
					pRtAddrIdTable[p_msg->dest_addr_id]->handle_timer(p_msg);
					} else rvf_free_timer_msg(p_msg);
				} else if(p_msg->msg_id==RVM_STOP_MSG) {
					_stop_t3_swe((T_RVM_STOP_MSG*)p_msg);
				} else { 
					if(pRtAddrIdTable[p_msg->dest_addr_id] && pRtAddrIdTable[p_msg->dest_addr_id]->handle_message) {
						pRtAddrIdTable[p_msg->dest_addr_id]->handle_message(p_msg);
					} else rvf_free_msg(p_msg);
				}
			} 
		}
		if(rec_event & RVF_TIMER_0_EVT_MASK) { 
			if(rvf_get_buf(rvm_mem_bank,sizeof(T_RV_HDR),(T_RVF_BUFFER**) &p_hdr)!=RVF_RED) {
				p_hdr->msg_id=RVF_TIMER_0_EVT_MASK;
				p_hdr->dest_addr_id=0;
				pRtAddrIdTable[resolveHostAddrId(gid)]->handle_timer(p_hdr);
			} else RVM_TRACE_WARNING_PARAM("RVM: T3 PROXY TM 0 EVENT DISPATCH MEM ERROR!", RV_TRACE_LEVEL_DEBUG_HIGH);
		}
		if(rec_event & RVF_TIMER_1_EVT_MASK) { 
			if(rvf_get_buf(rvm_mem_bank,sizeof(T_RV_HDR),(T_RVF_BUFFER**) &p_hdr)!=RVF_RED) {
				p_hdr->msg_id=RVF_TIMER_1_EVT_MASK;
				p_hdr->dest_addr_id=0;
				pRtAddrIdTable[resolveHostAddrId(gid)]->handle_timer(p_hdr);
			} else RVM_TRACE_WARNING_PARAM("RVM: T3 PROXY TM 1 EVENT DISPATCH MEM ERROR!", RV_TRACE_LEVEL_DEBUG_HIGH);

		}
		if(rec_event & RVF_TIMER_2_EVT_MASK) { 
			if(rvf_get_buf(rvm_mem_bank,sizeof(T_RV_HDR),(T_RVF_BUFFER**) &p_hdr)!=RVF_RED) {
				p_hdr->msg_id=RVF_TIMER_2_EVT_MASK;
				p_hdr->dest_addr_id=0;
				pRtAddrIdTable[resolveHostAddrId(gid)]->handle_timer(p_hdr);
			} else RVM_TRACE_WARNING_PARAM("RVM: T3 PROXY TM 2 EVENT DISPATCH MEM ERROR!", RV_TRACE_LEVEL_DEBUG_HIGH);

		}
		if(rec_event & RVF_TIMER_3_EVT_MASK) {

			nbTmExp=0; //rvf_update_timer_list((pRtAddrIdTable[gid]->p_tm_q));
//printf("T3 POLL TIMER Id %d ---------------------------------------------> %d!\n",gid, nbTmExp); 

			while(nbTmExp-- > 0) {
				p_msg=0; //(T_RV_HDR*)rvf_get_expired_entry(pRtAddrIdTable[gid]->p_tm_q);
				if(p_msg->dest_addr_id)
				  pRtAddrIdTable[p_msg->dest_addr_id]->handle_timer(p_msg);
				else RVM_TRACE_WARNING_PARAM("T3PROXY NO TM DESTINATION", RV_TRACE_LEVEL_DEBUG_HIGH);
			}
		}
		F_PRI=0;
	}

	/*if (error_status == INVKR_MEMORY_ERR) {
		invkr_env_ctrl_blk_p->error_ft("INVKR", RVM_MEMORY_ERR, 0,
						  " Memory Error : the INVKR primitive memory bank is RED ");
	}*/

	RVM_TRACE_WARNING_PARAM("T3PROXY ERROR: CORE TERMINATION", RV_TRACE_LEVEL_DEBUG_HIGH);

	/* BEFORE returning free resources !!!		*/
	return RVM_OK;
}

T_RVM_RETURN	rvm_start_group_req(T_RVF_G_ADDR_ID addrId, UINT8* grp) {
	T_RVM_START_T2_MSG * p_msg;
	T_RVF_MB_STATUS mb_status;
	UINT8 i=0;

	mb_status = rvf_get_msg_buf(rvm_mem_bank, 
							sizeof(T_RVM_START_T2_MSG),
							RVM_START_T2_MSG,
							(T_RVF_MSG**) &p_msg);

	if (mb_status == RVF_RED)	{
		RVM_TRACE_WARNING_PARAM("rvm_start_group(): Error to get memory ",RV_TRACE_LEVEL_ERROR);			
		return RVM_MEMORY_ERR;
	} else if (mb_status == RVF_YELLOW) {
		RVM_TRACE_WARNING_PARAM("rvm_start_group(): Getting short on memory ", RV_TRACE_LEVEL_WARNING);
	}

	for(i=0;i<10; i++) p_msg->grp[i]=0; // DEFINE MAX !!!

	p_msg->hdr.msg_id			= RVM_START_T2_MSG;   
	for(i=0; i<10 && grp[i]!=0; i++) p_msg->grp[i]=grp[i];

	return rvf_send_priority_msg(addrId, p_msg); //? cast (void*) i/p
}

T_RVM_RETURN _start_group(T_RVF_G_ADDR_ID gid, UINT8* grp) {
	T_RVM_INFO_SWE swe_info;
	UINT8 i=0;
	// for each k_swe_entry, IF NOT started get_info and call start() 
	for(i=0; i<10 && grp[i]!=0; i++ ) {
//		printf("INDEXES %d\n", grp[i]);
			rvm_swe_array[grp[i]].swe_get_info(&swe_info);
			rvf_setRDV(gid, rvm_swe_array[grp[i]].swe_addr_id);
			if(swe_info.type_info.type2.start) {
					swe_info.type_info.type2.start();
					rvm_swe_array[grp[i]].swe_state = SWE_RUNNING;
			}
			rvf_setRDV(gid, RVF_INVALID_ADDR_ID);
	}

	return RVM_OK;   /* ERROR case To do */ 
}

T_RVM_RETURN _t3_start(T_RVF_G_ADDR_ID gid) {
	T_RVM_INFO_SWE swe_info;

	rvm_swe_array[pRtAddrIdTable[gid]->swe_db_index].swe_get_info(&swe_info);
	if(swe_info.type_info.type3.start) {
					swe_info.type_info.type3.start();
					rvm_swe_array[pRtAddrIdTable[gid]->\
						swe_db_index].swe_state = SWE_RUNNING;
	}

	return RVM_OK;  /* ERROR case To do */ 
} 

T_RVM_RETURN _stop_t2_swe(T_RVM_STOP_MSG* p_msg) {
	T_RVM_INFO_SWE swe_info;

	rvm_swe_array[p_msg->swe_num].swe_get_info(&swe_info);
	if(swe_info.type_info.type2.stop) swe_info.type_info.type2.stop((T_RV_HDR*)p_msg);			

	return RVM_OK;  /* ERROR case To do */ 
}

T_RVM_RETURN _stop_t3_swe(T_RVM_STOP_MSG* p_msg) {
	T_RVM_INFO_SWE swe_info;

	rvm_swe_array[p_msg->swe_num].swe_get_info(&swe_info);
	if(swe_info.type_info.type3.stop) swe_info.type_info.type3.stop((T_RV_HDR*)p_msg);			

	return RVM_OK;  /* ERROR case To do */ 
}

T_RVM_RETURN rvm_mod_rt_t2_yield(T_RVF_G_ADDR_ID addrId, UINT16 val) {
	T_RVM_RT_MOD_YIELD_T2_MSG * p_msg;
	T_RVF_MB_STATUS mb_status;

	mb_status = rvf_get_msg_buf(rvm_sys_mem_bank, 
							sizeof(T_RVM_RT_MOD_YIELD_T2_MSG),
							RVM_RT_MOD_YIELD_T2_MSG,
							(T_RVF_MSG**) &p_msg);

	if (mb_status == RVF_RED)	{
		RVM_TRACE_WARNING_PARAM("rvm_start_group(): Error to get memory ",RV_TRACE_LEVEL_ERROR);			
		return RVM_MEMORY_ERR;
	} else if (mb_status == RVF_YELLOW) {
		RVM_TRACE_WARNING_PARAM("rvm_start_group(): Getting short on memory ", RV_TRACE_LEVEL_WARNING);
	}

	p_msg->hdr.msg_id			= RVM_RT_MOD_YIELD_T2_MSG;   /* from internal header */
	p_msg->val					= val;

	/* Send the message using mailbox. */
	return rvf_send_priority_msg(addrId, p_msg); //? cast (void*) i/p
}

/* ONLY used for return path msgs from entities started/stopped by the RVM */
#ifdef _CONSOLE
void _callerProxy(T_RVM_APPLI_RESULT *p_msg_res) {  /*A-M-E-N-D-E-D! */
	char *ret_val=NULL;

	switch(p_msg_res->result) {
		case RV_OK:					ret_val="RVM: Successful entity start or stop:";					
		break;
		case RV_NOT_SUPPORTED:		ret_val="RVM: Not Supported:";
		break;
		case RV_NOT_READY:			ret_val="RVM: Not Ready:";	
		break;
		case RV_MEMORY_ERR:			ret_val="RVM: Memory Alloc. Error:";
		break;
		case RV_MEMORY_REMAINING:	ret_val="RVM: Memory REMAINING AFTER STOP & KILL!: ";
		break;
		case RV_INTERNAL_ERR:		ret_val="RVM: Memory Alloc. Error:";	
		break;
		case RV_INVALID_PARAMETER:	ret_val="RVM: Invalid Parameter:";
		break;
		default:					ret_val="RVM: Not recognised:n";
		break;
	}
	RVM_TRACE_WARNING_PARAM(ret_val, p_msg_res->swe_index);
}
#endif

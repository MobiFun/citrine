/****************************************************************************/
/*                                                                          */
/*  Name        rvf_time.c                                                  */
/*                                                                          */
/*  Function    this file contains rvf time related functions               */
/*                                                                          */
/*  Version		0.1															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  3/12/99		Create														*/
/*  10/27/1999  remove all non-nucleus sections (#ifdef)					*/
/*	30/11/99	compliant to RV coding guidelines							*/
/*  31/01/2003  Timer Redesign/Impl.RV2    Gary TOTNEY						*/
/*																			*/
/*	Author		David Lamy-Charrier (dlamy@tif.ti.com)						*/
/*																			*/
/* (C) Copyright 1999 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../include/config.h"

#include "../rvm/rvm_use_id_list.h"
#include "../rvm/rvm_i.h"	// only for MBs [coupling issue]
#include "../rvm/rvm_api.h"	// MSG		[coupling issue]
#include "rvf_i.h"
#include "rvf_api.h"

#include "../../nucleus/nucleus.h"

#include <string.h>



/*
** Define the time control array
*/
static NU_TIMER		rvf_Timer[MAX_RVF_TASKS][RVF_NUM_TASK_TIMERS];

//extern T_RVF_MB_ID rvm_timer_mem_bank;

#define _RALLOC_TIMER_BLK(tb) rvf_get_buf(rvm_timer_mem_bank, sizeof(NU_TIMER), tb)
#define _RALLOC_TMS_MSG(id, tb) rvf_get_msg_buf(rvm_tm_notify_mem_bank, sizeof(T_RVF_TMS_MSG), id, tb)


/* PRIVATE FUNC FWD DECLs   */
T_RV_TM_ID _invoke_tm(T_RVF_G_ADDR_ID gid, UINT32 t, UINT8 cont, void* action, UINT8 lg);

/*******************************************************************************
**
** Function         _rvf_timer_expire
**
** Description      This internal function is called by Nucleus OS, when a task
**					timer expires. It sets the corresponding event.
**
** Returns          void
**
*******************************************************************************/
void _rvf_timer_expire(UNSIGNED timer_id)
{ 
	rvf_disable(14);
	/* send the corresponding timer event */
	rvf_send_event((UINT8)(timer_id / RVF_NUM_TASK_TIMERS),(UINT16)(EVENT_MASK(((timer_id % RVF_NUM_TASK_TIMERS)+4)) ) );
	rvf_enable();
}

void _rvf_tm_notify(UNSIGNED timer_id) {
	T_RVF_TM_UBLK			tmr;
	T_RVF_TM_ATTRIB_UBLK	attr;
	UINT32 tmid=0, cont=0, d;
	OPTION	opt;
	T_RVF_TMS_MSG*		 p_msg=NULL;
	
	if(timer_id) tmr.id=timer_id;

	if(NU_Timer_Information(tmr.ptr,attr.str, &opt, (UNSIGNED*)&d, (UNSIGNED*)&tmid, (UNSIGNED*)&d, (UNSIGNED*) &cont) == NU_INVALID_TIMER) { // use to get p_attrib
		rvf_send_trace ("FATAL: RVF TIMER ATTIRBUTE ERROR!",
						49,
						NULL_PARAM,
						RV_TRACE_LEVEL_WARNING,
						0);

	}
/*	printf("%ld %ld %d:",	attr.attrib.host_addr_id,
							attr.attrib.action,
							attr.attrib.legacyFlag);
*/
	if(!pRtAddrIdTable[attr.attrib.host_addr_id]) return; // Don't waste time in HISR			/* MUST BE REPLACED WITH CACHED MSG'ing  */

	// Will utilise active caching in near future, limiting HISR duration
	if((_RALLOC_TMS_MSG(RVM_TMS_MSG, (T_RVF_MSG**)&p_msg))==RVF_RED){
		rvf_send_trace ("FATAL: RVF TIMER HISR: Insufficient resources!",
						49,
						NULL_PARAM,
						RV_TRACE_LEVEL_WARNING,
						0);

		return; // (RV_MEMORY_ERR);
	} 
	p_msg->hdr.msg_id=RVM_TMS_MSG;
	p_msg->tm_id=tmr.id;
	p_msg->cont=cont;
	p_msg->action=attr.attrib.action;
	rvf_disable(14);
	rvf_send_priority_msg(attr.attrib.host_addr_id, p_msg); // NOTE: must have id and recognised in FSM core
	rvf_enable();
}

/********************************************************************************/
/*                                                                              */
/*    Function Name:   rvf_create_timer                                            */
/*                                                                              */
/*    Purpose:         This function is reponsible for stopping timers.         */
/*                                                                              */                      
/*    Revision History:                                                         */
/*                                                                              */
/********************************************************************************/
T_RVF_TIMER_ID rvf_create_timer(T_RVF_G_ADDR_ID		g_addrId,
							UINT32					tmDuration,
							BOOLEAN					isContinuous,
							void*					p_action) {
	
	return _invoke_tm(g_addrId, tmDuration, isContinuous, p_action, 0);
}

// this exists as part of an on-going initiative to deprecate rvf: start/stop timers
T_RV_TM_ID _invoke_tm(T_RVF_G_ADDR_ID gid, UINT32 t, UINT8 cont, void* action, UINT8 lg) {
	T_RVF_TM_UBLK			tmr;
	T_RVF_TM_ATTRIB_UBLK	tm_attrib;
	UINT32					re_sched=0;
	OPTION					opt=NU_DISABLE_TIMER;
	STATUS					status;

	if(t) opt=NU_ENABLE_TIMER;
	if(cont) re_sched=t;

	// TO DO... ret. 0 if alloc fails
	if(_RALLOC_TIMER_BLK((T_RVF_BUFFER**)&tmr.ptr)) return 0;
	
	tm_attrib.attrib.host_addr_id	=gid;
	tm_attrib.attrib.action			=action;
	tm_attrib.attrib.legacyFlag		=lg;

   	status=NU_Create_Timer( tmr.ptr,			// TIMER CONTROL BLOCK
							tm_attrib.str,		// timer name (overloaded)
							_rvf_tm_notify,		// expiration routine to call					
							tmr.id, 			// unique id which enables to find a specific task and a specific timer
							t,					// duration				
							re_sched,			// continuous = 1		
							opt);

	if(status!=NU_SUCCESS) tmr.id=0;
	
	return (UINT32)tmr.id;
}

/*T_RVF_RET rvf_get_remaining_time(T_RV_TM_ID tid, UINT32* t) {
	T_RVF_TM_UBLK			tmr;

	tmr.id=tid;
	if(NU_Get_Remaining_Time(tmr.ptr, (UNSIGNED*)&t)==NU_INVALID_TIMER) return RV_INVALID_PARAMETER;

	return RVF_OK;
}*/

/*******************************************************************************
**
** Function         rvf_del_timer
**
** Description      This function is called by an application to delete a timer
**                  entry from a timer list.
**
** Returns          void
**
*******************************************************************************/
void rvf_del_timer(T_RV_TM_ID tm_id) {
	T_RVF_TM_UBLK			tmr;

	if(tm_id) tmr.id=tm_id;

	NU_Control_Timer(tmr.ptr, NU_DISABLE_TIMER);
	NU_Delete_Timer(tmr.ptr);
	rvf_free_buf(tmr.ptr);
}

void rvf_reset_timer(T_RV_TM_ID tm_id, UINT32 new_duration,
										BOOLEAN isContinuous) {
	T_RVF_TM_UBLK			tmr;
	UINT32					re_sched=0;

	if(isContinuous)re_sched=new_duration;
	if(tm_id)		tmr.id=tm_id;

	NU_Control_Timer(tmr.ptr, NU_DISABLE_TIMER);

	if(new_duration) {
		NU_Reset_Timer(	tmr.ptr,
						_rvf_tm_notify,
						new_duration,
						re_sched, 
						NU_ENABLE_TIMER);
	}

}

/*******************************************************************************
**
** Function         _rvf_timers_init
**
** Description      This internal function is called once at startup to initialize
**                  all the timer structures.
**
** Returns          void
**
*******************************************************************************/
void _rvf_timers_init(void)
{
    UINT8   task_num, timer_num;

    for (task_num = 0; task_num< MAX_RVF_TASKS; task_num++)
    {
		for (timer_num = 0; timer_num < RVF_NUM_TASK_TIMERS; timer_num++)
		{
			NU_Create_Timer(	&(rvf_Timer[task_num][timer_num]),/* TIMER CONTROL BLOCK*/
								"", /* timer name */
								_rvf_timer_expire, /* expiration routine to call*/					
								(task_num * RVF_NUM_TASK_TIMERS) +timer_num,
								/* unique id which enables to find a specific task and a specific timer*/
								1,
								0,
								NU_DISABLE_TIMER);
		}		
    }
}

/*******************************************************************************
**
** Function         rvf_get_time_stamp
**
** Description      This function formats the time into a user area
**
** Returns          the address of the user area containing the formatted time
**
*******************************************************************************/
char *rvf_get_time_stamp(char *tbuf)
{
	UINT32 ms_time;
	UINT32 s_time;
	UINT32 m_time;
	UINT32 h_time;
    char   *p_out = tbuf;

	ms_time = rvf_get_tick_count();
	s_time  = ms_time/100;   /* 100 Ticks per second */
	m_time  = s_time/60;
	h_time  = m_time/60;

	ms_time -= s_time*100;
	s_time  -= m_time*60;
	m_time  -= h_time*60;

    *p_out++ = (char)((h_time / 10) + '0');
    *p_out++ = (char)((h_time % 10) + '0');
    *p_out++ = ':';
    *p_out++ = (char)((m_time / 10) + '0');
    *p_out++ = (char)((m_time % 10) + '0');
    *p_out++ = ':';
    *p_out++ = (char)((s_time / 10) + '0');
    *p_out++ = (char)((s_time % 10) + '0');
    *p_out++ = ':';
    *p_out++ = (char)((ms_time / 10) + '0');
    *p_out++ = (char)((ms_time % 10) + '0');
    *p_out++ = ':';
    *p_out   = 0;

	return tbuf;
}


/*******************************************************************************
**
** Function         rvf_get_tick_count
**
** Description      This function returns the current Nucleus system ticks
**
** Returns          ticks
**
*******************************************************************************/
UINT32	rvf_get_tick_count(void)
{
	return NU_Retrieve_Clock();
}


/*******************************************************************************
**
** Function         rvf_start_timer
**
** Description      An application can call this function to start one of
**                  it's four general purpose timers. Any of the four timers
**                  can be 1-shot or continuous. If a timer is already running,
**                  it will be reset to the new paramaters.
**
** Returns          void
**
*******************************************************************************/
void rvf_start_timer (UINT8 tnum, UINT32 ticks, BOOLEAN is_continuous)
{	T_RVF_G_ADDR_ID     task_id = rvf_get_taskid();
	
	if(tnum==RVF_TIMER_3 || tnum==RVF_TIMER_2) { // reserved for rvf timer service 
		rvf_send_trace ("WARNING: Timers 2 & 3 are deprecated!",
						37,
						NULL_PARAM,
						RV_TRACE_LEVEL_DEBUG_LOW,
						0); 
//		return;
	}
	if (!ticks) // check if ticks == 0, set it to 1 
		{	ticks = 1;
		}

	// disable Nucleus timer 
	NU_Control_Timer(	&(rvf_Timer[task_id][tnum]),
						NU_DISABLE_TIMER );

	// reset the timer with the new settings 
	NU_Reset_Timer(	&(rvf_Timer[task_id][tnum]),
					_rvf_timer_expire,
					ticks,
					is_continuous? ticks:0, // if timer is continuous, reload it 
					NU_ENABLE_TIMER);
}

/*******************************************************************************
**
** Function         rvf_stop_timer
**
** Description      An application can call this function to stop one of
**                  it's four general purpose timers. There is no harm in
**                  stopping a timer that is already stopped.
**
** Returns          void
**
*******************************************************************************/
void rvf_stop_timer (UINT8 tnum)
{	T_RVF_G_ADDR_ID  task_id = rvf_get_taskid();

	/* disable Nucleus timer */
	NU_Control_Timer(	&(rvf_Timer[task_id][tnum]),
						NU_DISABLE_TIMER );
}

/*******************************************************************************
**
** Function         rvf_init_timer_list
**
** Description      This function is called by applications when they
**                  want to initialize a timer list.
**
** Returns          void
**
*******************************************************************************/
void rvf_init_timer_list (T_RVF_TIMER_LIST_Q *p_timer_listq)
{
    p_timer_listq->p_first    = NULL;
    p_timer_listq->p_last     = NULL;
    p_timer_listq->last_ticks = 0;
}

/*******************************************************************************
**
** Function         rvf_init_timer_list_entry
**
** Description      This function is called by the applications when they
**                  want to initialize a timer list entry. This must be
**                  done prior to first use of the entry.
**
** Returns          void
**
*******************************************************************************/
void rvf_init_timer_list_entry (T_RVF_TIMER_LIST_ENT  *p_tle)
{
    p_tle->p_next  = NULL;
    p_tle->p_prev  = NULL;
    p_tle->ticks   = 0xFFFFFFFFL;
}


/*******************************************************************************
**
** Function         rvf_update_timer_list
**
** Description      This function is called by the applications when they
**                  want to update a timer list. This should be at every
**                  timer list unit tick, e.g. once per sec, once per minute etc.
**
** Returns          the number of timers that have expired
**
*******************************************************************************/
UINT16 rvf_revise_timer_list (T_RVF_G_ADDR_ID gid){
	if(!pRtAddrIdTable[gid] );
//	   !pRtAddrIdTable[gid]->p_tm_q ||
//		pRtAddrIdTable[gid]->p_tm_q->timerCnt==0 ||
//		pRtAddrIdTable[gid]->polling_tm!=0 ) return 0;			/* rvf abort     */
//	return rvf_update_timer_list(pRtAddrIdTable[gid]->p_tm_q);	/* tms notify    */
	return 0;
}
UINT16 rvf_update_timer_list (T_RVF_TIMER_LIST_Q *p_timer_listq)
{
    T_RVF_TIMER_LIST_ENT  *p_tle;
    UINT16          num_time_out = 0;

    p_tle = p_timer_listq->p_first;

    /* First, get the guys who have previously timed out */
    while ((p_tle) && (p_tle->ticks == 0))
    {
        num_time_out++;
        p_tle = p_tle->p_next;
    }

    /* Now, get the guys who have just timed out */
    if ((p_tle) && (p_tle->ticks))
    {
        if (--p_tle->ticks == 0)
        {
            while (p_tle != NULL && p_tle->ticks == 0)
            {
                num_time_out++;
                p_tle = p_tle->p_next;
            }
        }
    }

    if (p_timer_listq->last_ticks)
        p_timer_listq->last_ticks--;

    return (num_time_out);
}



/*******************************************************************************
**
** Function         rvf_add_to_timer_list
**
** Description      This function is called by an application to add a timer
**                  entry to a timer list.
**
** Returns          void
**
*******************************************************************************/
void rvf_add_to_timer_list (T_RVF_TIMER_LIST_Q *p_timer_listq, T_RVF_TIMER_LIST_ENT  *p_tle)
{
    UINT32           nr_ticks_total;
    T_RVF_TIMER_LIST_ENT  *p_temp;

    if (p_tle->ticks >= p_timer_listq->last_ticks)
    {
        if (p_timer_listq->p_first == NULL)
            p_timer_listq->p_first = p_tle;
        else
        {
            if (p_timer_listq->p_last != NULL)
                p_timer_listq->p_last->p_next = p_tle;

            p_tle->p_prev = p_timer_listq->p_last;
        }

        p_tle->p_next = NULL;
        p_timer_listq->p_last = p_tle;
        nr_ticks_total = p_tle->ticks;
        p_tle->ticks = p_tle->ticks - p_timer_listq->last_ticks;

        p_timer_listq->last_ticks = nr_ticks_total;
    }
    else
    {
        p_temp = p_timer_listq->p_first;
        while (p_tle->ticks > p_temp->ticks)
        {
            p_tle->ticks = p_tle->ticks - p_temp->ticks;
            p_temp = p_temp->p_next;
        }
        if (p_temp == p_timer_listq->p_first)
        {
            p_tle->p_next = p_timer_listq->p_first;
            p_timer_listq->p_first->p_prev = p_tle;
            p_timer_listq->p_first = p_tle;
        }
        else
        {
            p_temp->p_prev->p_next = p_tle;
            p_tle->p_prev = p_temp->p_prev;
            p_temp->p_prev = p_tle;
            p_tle->p_next = p_temp;
        }
        p_temp->ticks = p_temp->ticks - p_tle->ticks;
    }
}

/*******************************************************************************
**
** Function         rvf_remove_from_timer_list
**
** Description      This function is called by an application to remove a timer
**                  entry from a timer list.
**
** Returns          void
**
*******************************************************************************/
void rvf_remove_from_timer_list (T_RVF_TIMER_LIST_Q *p_timer_listq, T_RVF_TIMER_LIST_ENT  *p_tle)
{
    if (p_tle == NULL || p_tle->ticks == 0xFFFFFFFFL || p_timer_listq->p_first == NULL)
    {
        return;
    }

    /* Add the ticks remaining in this timer to the next guy in the list.
    */
    if (p_tle->p_next != NULL)
    {
        p_tle->p_next->ticks += p_tle->ticks;
    }
    else
    {
        p_timer_listq->last_ticks -= p_tle->ticks;
    }

    /* Unlink timer from the list.
    */
    if (p_timer_listq->p_first == p_tle)
    {
        p_timer_listq->p_first = p_tle->p_next;

        if (p_timer_listq->p_first != NULL)
            p_timer_listq->p_first->p_prev = NULL;

        if (p_timer_listq->p_last == p_tle)
            p_timer_listq->p_last = NULL;
    }
    else
    {
        if (p_timer_listq->p_last == p_tle)
        {
            p_timer_listq->p_last = p_tle->p_prev;

            if (p_timer_listq->p_last != NULL)
                p_timer_listq->p_last->p_next = NULL;
        }
        else
        {
            if (p_tle->p_next != NULL && p_tle->p_next->p_prev == p_tle)
                p_tle->p_next->p_prev = p_tle->p_prev;
            else
            {
                /* Error case - chain messed up ?? */
                return;
            }

            if (p_tle->p_prev != NULL && p_tle->p_prev->p_next == p_tle)
                p_tle->p_prev->p_next = p_tle->p_next;
            else
            {
                /* Error case - chain messed up ?? */
                return;
            }
        }
    }

    p_tle->p_next = p_tle->p_prev = NULL;
    p_tle->ticks = 0xFFFFFFFFL;
}


/*******************************************************************************
**
** Function         rvf_get_expired_entry
**
** Description      This function returns a pointer to the first expired entry in
**					the timer list queue.
**					If no entry in the queue has expired, it returns NULL.
**
** Returns          T_RVF_TIMER_LIST_ENT * : pointer to the expired entry if any.
**
*******************************************************************************/
T_RVF_TIMER_LIST_ENT * rvf_get_expired_entry (T_RVF_TIMER_LIST_Q *p_timer_listq)
{	
	T_RVF_TIMER_LIST_ENT * p_tle;
	/* test if the first entry in the queue has expired */
	if ( (p_timer_listq->p_first != NULL) && (p_timer_listq->p_first->ticks == 0) )
	{	

		p_tle = p_timer_listq->p_first;
		if(p_tle->t_init) {
			p_tle->ticks=p_tle->t_init;
		} else {
			/* unlink entry from the list */
			p_timer_listq->p_first = p_tle->p_next;

			if (p_timer_listq->p_first != NULL)
				p_timer_listq->p_first->p_prev = NULL;

			if (p_timer_listq->p_last == p_tle)
				p_timer_listq->p_last = NULL;
			
			p_tle->p_next = NULL;
			p_tle->p_prev = NULL;
			p_tle->ticks = 0xFFFFFFFFL;
			//return p_tle;
		}
		return p_tle;
	}
	else
	{	return NULL;
	}
}

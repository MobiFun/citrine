/****************************************************************************/
/*                                                                          */
/*  Name        rvf_buffer.c                                                */
/*                                                                          */
/*  Function    this file contains rvf buffer handling functions            */
/*                                                                          */
/*  Version		0.1															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  3/12/99		Create														*/
/*  10/27/1999	remove all non-nucleus sections (#ifdef)					*/
/*	30/11/1999	compliant to RV coding guidelines							*/
/*  12/23/1999	change buffer management, add memory bank handling and		*/
/*				remove useless functions									*/
/*  07/12/2000	implement dynamic memory allocation.						*/
/*																			*/
/*	Author		David Lamy-Charrier (dlamy@tif.ti.com)						*/
/*																			*/
/* (C) Copyright 1999 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../include/config.h"

#include "../rvm/rvm_use_id_list.h"
#include "rvf_api.h"
#include "rvf_i.h"
#include "../support/exception.h"
#include "rvf_pool_size.h"

#include "../../nucleus/nucleus.h"

#include <string.h>
#include <stdio.h>


#if RVF_ENABLE_STATS	/* conditional inclusion of stdio.h for sprintf() function */
#ifdef _WINDOWS
#include <stdio.h>
#endif
#endif

#include "../rv/rv_defined_swe.h"

#ifdef RVM_DAR_SWE
	#include "dar/dar_api.h"
#endif
#define RVF_INVALID_MD_ID_ERRROR  ("RVF: Invalid MB ID")

#define INC_END_INITIALIZE		2
#define RVF_INVALID_INDEX		0xFFFF

/************* TASK MAILBOXES *******************/
/* chained lists for task mailboxes			*/
T_RVF_INTERNAL_BUF *OSTaskQFirst[1][1]; //[MAX_RVF_TASKS][RVF_NUM_TASK_MBOX]; 
T_RVF_INTERNAL_BUF *OSTaskQLast [1][1]; //[MAX_RVF_TASKS][RVF_NUM_TASK_MBOX];

extern INC_Initialize_State;

T_RVF_RT_ADDR_ID_DATA*	pRtAddrIdTable[MAX_RVF_G_ADDR_ID]; 


/******** MEMORY POOLS ******************/
/* Define the buffer pools */
extern T_RVF_POOL		_rvf_pools[];

/*********** MEMORY BANKS ***********/
/* array of memory bank				*/
static T_RVF_MB			rvf_banks[RVF_MAX_REAL_MB];

/* array of waiting buffers */
static T_RVF_BUFFER *	waiting_buffers[RVF_MAX_WAITING_BUF];
static UINT16			next_buffer[RVF_MAX_WAITING_BUF];
static UINT16			first_free_element;

/* array of memory bank name and id*/
static T_RVF_MB_NAME_ID rvf_name_id[RVF_MAX_TOTAL_MB] = RVF_MB_MAPPING;


/* variable for statistics */
#if RVF_ENABLE_STATS
static UINT32 required_size = 0;
static UINT32 obtained_size = 0;
static UINT32 used_size		= 0;
static UINT32 mem_in_use	= 0;
#endif

/* lists of free buffers */
static T_RVF_INTERNAL_BUF * lists[RVF_NB_FREE_LISTS];

/* last split-off buffer */
T_RVF_INTERNAL_BUF * last_remainder = NULL;

/* allocated static buffer pools */
extern UINT8 Buf0[];
extern UINT8 Buf1[];

/********Internal windows function used to display memory status *******/
#ifdef _WINDOWS
extern void AddNewState(void *s,char *name,
				unsigned long  current,unsigned long  peak,
				unsigned long  water,unsigned long  bank);
#endif


/*******************************************************************************
**
** Function         _rvf_init_free_queue
**
** Description      Function called at startup to initialize a free
**                  pool (statically or dynamically allocated). 
**					It is called once for each free pool.
**
** Returns          void
**
*******************************************************************************/
void _rvf_init_free_queue (UINT8 id, UINT32 size, void *p_mem)
{
   T_RVF_INTERNAL_BUF	*hdr;
	UINT8 list_idx;

    /* round to up size to a multiple of 4 */
    size = (size + 3) & ~0x0003;

    /* store pool start address and size */
	_rvf_pools[id].start_address	= p_mem;

    _rvf_pools[id].pool_size		= size;
   
	/* Initialize the pool as a big free buffer */
	hdr = (T_RVF_INTERNAL_BUF *) p_mem;
	hdr->buf_size = size - sizeof(T_RVF_INTERNAL_BUF) - sizeof(UINT32); /* last 4 bytes of the pool losts*/ 
	hdr->header.p_prev = NULL;
	RVF_SET_PREV_IN_USE(hdr);

	NEXTCHUNK(hdr)->buf_size = 0;
	
	ENDSIZE(hdr) = hdr->buf_size;

	
	/* get the corresponding list and insert the buffer */
	list_idx		= RVF_BUF_LIST_INDEX( hdr->buf_size);
	hdr->p_next		= lists[list_idx];
	lists[list_idx] = hdr;

	last_remainder	= hdr;

}

void rvf_mbox_buffer_init(T_RVF_RT_ADDR_ID_DATA* pRtAddrIdElement) {
   UINT8   task_num, mbox_num;
    /* Initialize all mailboxes queues of all tasks*/
    for (task_num = 0; task_num < MAX_RVF_TASKS; task_num++) {
        for (mbox_num = 0; mbox_num < RVF_NUM_TASK_MBOX; mbox_num++) {
			pRtAddrIdElement->OSTaskQFirst[mbox_num] = NULL;
			pRtAddrIdElement->OSTaskQLast[mbox_num] = NULL;
        }
    }
}
/*******************************************************************************
**
** Function         _rvf_buffer_init
**
** Description      Called once internally by rvf at startup to initialize all
**                  buffers and free buffer pools.
**
** Returns          void
**
*******************************************************************************/
void _rvf_buffer_init(void)
{
    UINT8   list_num;
	UINT16	memory_bank_num;

    /* Initialize all mailboxes queues of all tasks*/
    /*  for (task_num = 0; task_num < MAX_RVF_TASKS; task_num++)
    {
        for (mbox_num = 0; mbox_num < RVF_NUM_TASK_MBOX; mbox_num++)
        {
            OSTaskQFirst[task_num][mbox_num] = NULL;
            OSTaskQLast [task_num][mbox_num] = NULL;
			//pRtAddrIdTable[task_num]->OSTaskQFirst[mbox_num] = NULL; // only if static
			//pRtAddrIdTable[task_num]->OSTaskQLast[mbox_num] = NULL; // only if static
        }
    } */

   /* initialize free lists */
   for (list_num = 0; list_num < RVF_NB_FREE_LISTS; list_num++)
   {
      lists[list_num] = NULL;
   }


   /* initialize buffer pools */
   _rvf_init_mem_pool();

	/* Initialize real memory banks */
	for (memory_bank_num = 0; memory_bank_num < RVF_MAX_REAL_MB; memory_bank_num++)
	{	
		rvf_banks[memory_bank_num].cur_memory_used		= 0;
		rvf_banks[memory_bank_num].watermark			= 0;
		rvf_banks[memory_bank_num].max					= 0;
		rvf_banks[memory_bank_num].first_buffer_index	= RVF_INVALID_INDEX;
		rvf_banks[memory_bank_num].last_buffer_index	= RVF_INVALID_INDEX;
		rvf_banks[memory_bank_num].func					= 0;
		rvf_banks[memory_bank_num].returned_red			= FALSE;
#if RVF_ENABLE_STATS
		rvf_banks[memory_bank_num].max_reached			= 0;
		rvf_banks[memory_bank_num].required_size		= 0;
		rvf_banks[memory_bank_num].num_buf				= 0;
#endif
	}

	/* Initialize total memory banks */
	for (memory_bank_num = 0; memory_bank_num < RVF_MAX_TOTAL_MB; memory_bank_num++)
	{	
		rvf_name_id[memory_bank_num].mb_params.size			= 0;
		rvf_name_id[memory_bank_num].mb_params.watermark	= 0;
	}


	/* initialize array of waiting buffers */
	first_free_element = 0;
	for( memory_bank_num = 0; memory_bank_num < RVF_MAX_WAITING_BUF; memory_bank_num++)
	{	waiting_buffers[memory_bank_num] = NULL;
		next_buffer[memory_bank_num] = memory_bank_num + 1;
	}
	next_buffer[RVF_MAX_WAITING_BUF-1] = RVF_INVALID_INDEX;
}

/*******************************************************************************
**
** Function         _rvf_send_msg_to_mbox
**
** Description      Called by applications to send a buffer to a SWE.
**                  (Temporary internal use - DO NOT USE IT !)
**					(Presently, only for Christophe          )
** Returns			RVF_OK if successful, else an error code.
**
*******************************************************************************/
T_RVF_RET _rvf_send_msg_to_mbox (T_RVF_G_ADDR_ID addr_id, UINT8 mbox,void *msg){
	return rvf_adapt_send_msg(addr_id, msg, mbox);
}

/*T_RVF_RET _rvf_send_msg_to_mbox (T_RVF_ADDR_ID addr_id, UINT8 mbox,void *msg)
{
    T_RVF_INTERNAL_BUF    *p_hdr;
	UINT8 task_id = (UINT8)addr_id;

    
    if ((task_id >= MAX_RVF_TASKS) )
    {
        rvf_send_trace( "RVF: rvf_send_msg(): invalid taskid", 35, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
        rvf_free_buf (msg);
        return RVF_INVALID_PARAMETER;
    }

#if RVF_ENABLE_BUF_CORRUPTION_CHECK	

	if (_rvf_chk_buf_damage(msg) == TRUE)
    {	rvf_send_trace( "RVF: rvf_send_msg(): buffer corrupted", 37, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
	}
#endif

	
	((T_RV_HDR *)msg)->dest_addr_id = addr_id;
	p_hdr = USER2MEM(msg);

#if RVF_ENABLE_BUF_LINKAGE_CHECK
  
	if ( RVF_BUF_IS_LINKED( p_hdr) )
	{
		rvf_send_trace( "RVF: rvf_send_msg(): buffer already enqueued", 44, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
        return RVF_MEMORY_ERR;
    }
#endif

    rvf_disable(8);		

	
    if (OSTaskQFirst[task_id][mbox])	
	{	OSTaskQLast[task_id][mbox]->p_next = p_hdr;
	}
    else
	{	OSTaskQFirst[task_id][mbox] = p_hdr;
	}
    OSTaskQLast[task_id][mbox] = p_hdr;

	p_hdr->p_next = NULL;

	if (pRtAddrIdTable[task_id]->OSTaskQFirst[mbox]) {
		pRtAddrIdTable[task_id]->OSTaskQLast[mbox]->p_next = p_hdr;
	} else {
		pRtAddrIdTable[task_id]->OSTaskQFirst[mbox] = p_hdr;
	}
    pRtAddrIdTable[task_id]->OSTaskQLast[mbox] = p_hdr;

	p_hdr->p_next = NULL;

#if RVF_ENABLE_BUF_LINKAGE_CHECK
	RVF_SET_BUF_LINKED(p_hdr);	
#endif

    rvf_enable();		

    rvf_send_event(task_id, (UINT16) (EVENT_MASK(mbox)) );
	return RVF_OK;
}*/

/*******************************************************************************
**
** Function         rvf_read_mbox
**
** Description      Called by applications to read a buffer from one of
**                  the task mailboxes.
**
** Returns          NULL if the mailbox was empty, else the address of a buffer
**
*******************************************************************************/
void * rvf_read_mbox (UINT8 mbox) {   
	T_RVF_G_ADDR_ID  task_id	= rvf_get_taskid();

	return rvf_read_addr_mbox (task_id, mbox) ;   
}
void * rvf_read_addr_mbox (T_RVF_G_ADDR_ID  task_id, UINT8 mbox) {   
    void * p_buf	= NULL;
    T_RVF_INTERNAL_BUF * p_hdr;

	mbox=resolveHostAddrId(mbox);

    if ((task_id >= MAX_RVF_TASKS) || (mbox >= RVF_NUM_TASK_MBOX))
    {   rvf_send_trace( "RVF: rvf_read_mbox(): invalid taskid or mbox", 44, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
        return NULL;
    } 

    rvf_disable(9);		/* enter critical section */

	/* if the chained list is not empty */
    /*if ( OSTaskQFirst[task_id][mbox] )	
    {   p_hdr = OSTaskQFirst[task_id][mbox];
        OSTaskQFirst[task_id][mbox] = p_hdr->p_next;

        p_hdr->p_next = NULL;*/
	if ( pRtAddrIdTable[task_id]->OSTaskQFirst[mbox] )	{
        p_hdr = pRtAddrIdTable[task_id]->OSTaskQFirst[mbox];
        pRtAddrIdTable[task_id]->OSTaskQFirst[mbox] = p_hdr->p_next;

        p_hdr->p_next = NULL;

#if RVF_ENABLE_BUF_LINKAGE_CHECK
	RVF_SET_BUF_UNLINKED(p_hdr);	/* change buffer status */
#endif
        p_buf = (UINT8 *)p_hdr + sizeof(T_RVF_INTERNAL_BUF);
    }
 
	rvf_enable();		/* exit critical section */

    return (p_buf);
}

/*******************************************************************************
**
** Function         rvf_enqueue
**
** Description      Enqueue a buffer at the tail of the queue
**
** Returns          RVF_OK if successful, else an error code.
**
*******************************************************************************/
T_RVF_RET rvf_enqueue (T_RVF_BUFFER_Q *p_q, void *p_buf)
{
    T_RVF_INTERNAL_BUF *p_hdr;

#if RVF_ENABLE_BUF_CORRUPTION_CHECK
    if ( _rvf_chk_buf_damage(p_buf) )
    {	rvf_send_trace( "RVF: rvf_enqueue(): buffer corrupted", 36, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID);
    }
#endif

   	/* check if the buffer has been already enqueued */
    p_hdr = USER2MEM(p_buf);

#if RVF_ENABLE_BUF_LINKAGE_CHECK
	if( RVF_BUF_IS_LINKED(p_hdr) )
	{
		rvf_send_trace( "RVF: rvf_enqueue(): buffer already enqueued", 43, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID);
        return RVF_MEMORY_ERR;
    }
#endif

    rvf_disable(10);	/* enter critical section */

    /* Since the queue is exposed (C vs C++), keep the pointers in exposed format */
	/* p_q->p_last and p_q->p_first point to the user buffer, since p_hdr->p_next points to the T_RVF_INTERNAL_BUF */
    if (p_q->p_first)	/* if the queue is not empty */
    {
		T_RVF_INTERNAL_BUF * p_last_hdr = (T_RVF_INTERNAL_BUF *) ((UINT8 *)p_q->p_last - sizeof(T_RVF_INTERNAL_BUF) );
        p_last_hdr->p_next = p_hdr;
    }
    else
        p_q->p_first = p_buf;

    p_q->p_last = p_buf;
    p_q->count++;

    p_hdr->p_next = NULL;
#if RVF_ENABLE_BUF_LINKAGE_CHECK
    RVF_SET_BUF_LINKED(p_hdr);	/* change buffer status */
#endif

    rvf_enable();		/* exit critical section */
	return RVF_OK;
}



/*******************************************************************************
**
** Function         rvf_dequeue
**
** Description      Dequeue a buffer from the head of a queue
**
** Returns          NULL if queue is empty, else buffer
**
*******************************************************************************/
void * rvf_dequeue (T_RVF_BUFFER_Q *p_q)
{
    T_RVF_INTERNAL_BUF    *p_hdr;

    if (!p_q->count)	/* if the queue is empty */
        return (NULL);

    rvf_disable(12);	/* enter critical section */

    p_hdr = USER2MEM(p_q->p_first );


    /* Keep buffers such that RVF header is invisible */
    if (p_hdr->p_next)
        p_q->p_first = ((UINT8 *)p_hdr->p_next + sizeof(T_RVF_INTERNAL_BUF));
    else
    {
        p_q->p_first = NULL;
        p_q->p_last  = NULL;
    }

    p_q->count--;

    p_hdr->p_next = NULL;

#if RVF_ENABLE_BUF_LINKAGE_CHECK
    RVF_SET_BUF_UNLINKED(p_hdr);	/* change buffer status */
#endif

    rvf_enable();		/* exit critical section */

    return (MEM2USER(p_hdr) );
}



/*******************************************************************************
**
** Function         rvf_enqueue_head
**
** Description      Enqueue a buffer at the head of the queue
**
** Returns          RVF_OK if successful, else an error code.
**
*******************************************************************************/
T_RVF_RET rvf_enqueue_head (T_RVF_BUFFER_Q *p_q, void *p_buf)
{
    T_RVF_INTERNAL_BUF    *p_hdr;

#if RVF_ENABLE_BUF_CORRUPTION_CHECK
    if ( _rvf_chk_buf_damage(p_buf) )
    {	rvf_send_trace( "RVF: rvf_enqueue_head(): buffer corrupted", 41, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
    }
#endif

    p_hdr = USER2MEM(p_buf);

#if RVF_ENABLE_BUF_LINKAGE_CHECK
	if( RVF_BUF_IS_LINKED(p_hdr) )
    {	rvf_send_trace( "RVF: rvf_enqueue_head(): buffer already enqueued", 48, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
        return RVF_MEMORY_ERR;
    }
#endif

    rvf_disable(11);		/* enter critical section */

    if (p_q->p_first)		/* if the queue is not empty */
    {   p_hdr->p_next = (T_RVF_INTERNAL_BUF *)((UINT8 *)p_q->p_first - sizeof(T_RVF_INTERNAL_BUF) );
        p_q->p_first = p_buf;
    }
    else
    {    p_q->p_first = p_buf;
        p_q->p_last  = p_buf;
        p_hdr->p_next = NULL;
    }
    p_q->count++;

#if RVF_ENABLE_BUF_LINKAGE_CHECK
	RVF_SET_BUF_LINKED(p_hdr);	/* change buffer status */
#endif
		

    rvf_enable();		/* exit critical section */
	
	return RVF_OK;
}



/*******************************************************************************
**
** Function         rvf_get_buf_size
**
** Description      Called by an application to get the size of a buffer.
**
** Returns          the size of the buffer or 0 if the address is invalid.
**
*******************************************************************************/
INLINE UINT32 rvf_get_buf_size (void *bptr)
{
    T_RVF_INTERNAL_BUF *p_hdr;

    p_hdr = (T_RVF_INTERNAL_BUF *)((UINT8 *) bptr - sizeof(T_RVF_INTERNAL_BUF) );

    if ((UINT32)p_hdr & 1) return 0; /* invalid pointer: odd address*/

    return (GETSIZE(p_hdr) - RVF_CORRUPT_OVERHEAD) ;
}


/*******************************************************************************
**
** Function         _rvf_chk_buf_damage
**
** Description      Called internally by rvf to check for buffer corruption.
**
** Returns          TRUE if there is a problem, else FALSE
**
*******************************************************************************/
#if RVF_ENABLE_BUF_CORRUPTION_CHECK
BOOLEAN _rvf_chk_buf_damage(void *bptr)
{
   UINT32 *lg;
	T_RVF_INTERNAL_BUF * p_hdr;
	
	if((UINT32)bptr & 1)	/* odd address */
	{	return TRUE;
	}

	p_hdr = USER2MEM(bptr);	/* get the internal header */

    lg = (UINT32*)( (UINT8*)bptr +  GETSIZE(p_hdr) - sizeof(UINT32) );

	 if (lg == 0)
		 return TRUE;

    if(*lg == GETSIZE(p_hdr) )
    {
        return FALSE;
    }
    return TRUE;
}
#endif



/*******************************************************************************
**
** Function         _find_buf
**
** Description      Internal function which is in charge of finding a free buffer
**					of the requested size in one of the lists.
**
** Returns          A pointer to the buffer header, or NULL if none available
**
*******************************************************************************/
INLINE void * _find_buf (UINT32 size)
{    
	T_RVF_INTERNAL_BUF * p_hdr;
	UINT8 idx;

    if (size == 0)	/* if user requires a 0 byte buffer !! */
    {    return (NULL);
	}

	/* add the overhead for buffer corruption check */
	size = REQ2SIZE(size);

	/* find the corresponding list */
	idx = RVF_BUF_LIST_INDEX(size);

	/* 1. try in the bin corresponding to the requested size. */
	/* 2. try to use the last_remainder chunk. */
	/* 3. try in the others bins of greater size. */

	if( (lists[idx] == NULL) || ( GETSIZE(lists[idx]) < size) )  
	/* if the first buffer in the appropriate bin is not big enough. */
	{
		rvf_disable(4);		/* enter critical section */

		if( last_remainder != NULL)
		{
			p_hdr = last_remainder;

			/* if the last remainder is big enough */
			if( GETSIZE(p_hdr) >= size )
			{

				if( GETSIZE(p_hdr) >= (size + RVF_MIN_USABLE_SIZE ) ) /* if the free part may be used */
				{	/* create a new free buffer and link it in the appropriate list*/

					T_RVF_INTERNAL_BUF * new_buf;
					UINT8 new_idx;
					
					new_buf = (T_RVF_INTERNAL_BUF *) ((UINT8*)p_hdr + size + sizeof(T_RVF_INTERNAL_BUF));
					new_buf->buf_size =  GETSIZE(p_hdr) - size - sizeof(T_RVF_INTERNAL_BUF);
					ENDSIZE(new_buf) = new_buf->buf_size;

					/* remove the used buffer from the list */
					if( p_hdr->header.p_prev != NULL)
					{	(p_hdr->header.p_prev)->p_next = p_hdr->p_next;
					}
					else
					{	lists[ RVF_BUF_LIST_INDEX( GETSIZE(p_hdr) )] = p_hdr->p_next;
					}
					if( p_hdr->p_next != NULL)
					{	(p_hdr->p_next)->header.p_prev = p_hdr->header.p_prev;
					}
					p_hdr->p_next = NULL;

					SETSIZE(p_hdr, size);
					ENDSIZE(p_hdr) = size;  /* to CHANGE */

					/* insert the new buffer in the appropriate list */
					new_idx = RVF_BUF_LIST_INDEX(new_buf->buf_size);
					new_buf->p_next = lists[new_idx];
					lists[new_idx] = new_buf;
					new_buf->header.p_prev = NULL;
					if( new_buf->p_next != NULL)
					{	(new_buf->p_next)->header.p_prev = new_buf;
					}
					RVF_SET_PREV_IN_USE(new_buf);
						
					last_remainder = new_buf;

					rvf_enable();		/* exit critical section */
					return p_hdr;				

				}
				else	/* return the entire buffer */
				{
					/* remove the used buffer from the list */
					if( p_hdr->header.p_prev != NULL)
					{	(p_hdr->header.p_prev)->p_next = p_hdr->p_next;
					}
					else
					{	lists[ RVF_BUF_LIST_INDEX( GETSIZE(p_hdr) )] = p_hdr->p_next;
					}
					if( p_hdr->p_next != NULL)
					{	(p_hdr->p_next)->header.p_prev = p_hdr->header.p_prev;
					}
					p_hdr->p_next = NULL;

					RVF_SET_PREV_IN_USE( NEXTCHUNK(p_hdr) );

					last_remainder = NULL;
				
					rvf_enable();		/* exit critical section */
					return p_hdr;
				}
			}
			else /* the last remainder is too small */
			{
				/* clear the last remainder */
				last_remainder = NULL;
			}
			
		}
		rvf_enable();		/* exit critical section */
	}

	while( idx < RVF_NB_FREE_LISTS )
	{
		rvf_disable(4);		/* enter critical section */

		if( lists[idx] != NULL )  /*if the list is not empty */
		{	/* remove the first buffer from the list */

			p_hdr = lists[idx];
			if( GETSIZE(p_hdr) >= size)
			{
				if( GETSIZE(p_hdr) >= (size + RVF_MIN_USABLE_SIZE ) ) /* if the free part may be used */
				{	/* create a new free buffer and link it in the appropriate list*/

					T_RVF_INTERNAL_BUF * new_buf;
					UINT8 new_idx;
					
					new_buf = (T_RVF_INTERNAL_BUF *) ((UINT8*)p_hdr + size + sizeof(T_RVF_INTERNAL_BUF));
					new_buf->buf_size =  GETSIZE( p_hdr) - size - sizeof(T_RVF_INTERNAL_BUF);
					ENDSIZE(new_buf) = new_buf->buf_size;

					/* remove the used buffer from the list */
					lists[idx] = p_hdr->p_next;
					if( p_hdr->p_next != NULL)
					{	(p_hdr->p_next)->header.p_prev = NULL;
					}
					p_hdr->p_next = NULL;

					SETSIZE(p_hdr, size);
					ENDSIZE(p_hdr) = size;

					/* insert the new buffer in the appropriate list */
					new_idx = RVF_BUF_LIST_INDEX(new_buf->buf_size);
					new_buf->p_next = lists[new_idx];
					lists[new_idx] = new_buf;
					new_buf->header.p_prev = NULL;
					if( new_buf->p_next != NULL)
					{	(new_buf->p_next)->header.p_prev = new_buf;
					}
					RVF_SET_PREV_IN_USE(new_buf);

					last_remainder = new_buf; /* set this new buffer as the last remainder */

					rvf_enable();		/* exit critical section */
					return p_hdr;				

				}
				else	/* return the entire buffer */
				{	lists[idx] = p_hdr->p_next;
					if( p_hdr->p_next != NULL)
					{	(p_hdr->p_next)->header.p_prev = NULL;
					}
					p_hdr->p_next = NULL;
					RVF_SET_PREV_IN_USE( NEXTCHUNK(p_hdr) );
					
					if( last_remainder == p_hdr) /* if it was the last_remainder, clear it. */
					{
						last_remainder = NULL;
					}

					rvf_enable();		/* exit critical section */
					return p_hdr;
				}
			}
		}

		rvf_enable();		/* exit critical section */

		idx++;	/* search in the next list */
	}

    return NULL;
}


/*******************************************************************************
**
** Function         _release_buf
**
** Description      Internal function called to release a buffer after use.
**					The parameter points to the beginning of the header.
**
** Returns          BOOLEAN: TRUE if successful, else FALSE
**
*******************************************************************************/
INLINE BOOLEAN _release_buf (T_RVF_INTERNAL_BUF *p_hdr)
{	UINT8 idx;

#if RVF_ENABLE_BUF_CORRUPTION_CHECK		/* check for buffer corruption:i.e. if user wrote data after the end of the buffer */
    if ( _rvf_chk_buf_damage( MEM2USER(p_hdr) ) )
    {	rvf_send_trace( "RVF: _release_buf(): buffer corrupted", 37, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
		return FALSE;
    }

	/* check if the buffer has been already freed */
	if ( RVF_IS_PREV_FREE( NEXTCHUNK(p_hdr) ) )	/* check buffer status */
    {	rvf_send_trace( "RVF: _release_buf(): buffer already freed", 41, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
        return FALSE;
    }

#endif

#if RVF_ENABLE_BUF_LINKAGE_CHECK		/* check for buffer linkage */
    if ( RVF_BUF_IS_LINKED(p_hdr) )	/* check buffer status */
    {	rvf_send_trace( "RVF: _release_buf(): free buf buffer linked", 43, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
        return FALSE;
    }
#endif


	rvf_disable(6);		/* enter critical section */

	/* try to coalesce the buffer with its neighbors (left and right) */

	if( RVF_IS_PREV_FREE(p_hdr) )
	{	/* merge the buffer with its left neighbor */
		UINT32 left_buf_size = *((UINT32*)((UINT8*)p_hdr - sizeof(UINT32) ) );
		T_RVF_INTERNAL_BUF * left_buf = (T_RVF_INTERNAL_BUF *) ((UINT8 *)p_hdr - left_buf_size - sizeof(T_RVF_INTERNAL_BUF) );

		/* remove the left buffer from its list */
		if( left_buf->header.p_prev == NULL)
		{	lists[ RVF_BUF_LIST_INDEX(left_buf_size)] = left_buf->p_next;
		}
		else
		{	(left_buf->header.p_prev)->p_next = left_buf->p_next;
		}
		if( left_buf->p_next != NULL)
		{
			(left_buf->p_next)->header.p_prev = left_buf->header.p_prev;
		}
		/* set the size of the newly created buffer */
		SETSIZE(left_buf, (left_buf_size + GETSIZE(p_hdr) + sizeof(T_RVF_INTERNAL_BUF) ) );
		/* set the current buffer as free to allow check for double free */
		RVF_SET_PREV_FREE( NEXTCHUNK(p_hdr) );

		p_hdr = left_buf;
	}

	/* check for pool limits */
	if( GETSIZE( NEXTCHUNK(p_hdr) ) != 0)
	{	T_RVF_INTERNAL_BUF * right_buf = NEXTCHUNK(p_hdr);
		/* merge the buffer with its right neighbor */
		
		if( RVF_IS_PREV_FREE( NEXTCHUNK(right_buf) ) )
		{	/* remove the right buffer from its list */
			UINT32 right_buf_size = GETSIZE( right_buf);
	
			if( right_buf->header.p_prev == NULL)
			{	lists[ RVF_BUF_LIST_INDEX(right_buf_size)] = right_buf->p_next;
			}
			else
			{	(right_buf->header.p_prev)->p_next = right_buf->p_next;
			}
			if( right_buf->p_next != NULL)
			{	(right_buf->p_next)->header.p_prev = right_buf->header.p_prev;
			}
			
			right_buf_size += GETSIZE(p_hdr);
			SETSIZE(p_hdr, (right_buf_size + sizeof(T_RVF_INTERNAL_BUF) ) );

			if( last_remainder == right_buf) /* keep as last_remainder */
			{	last_remainder = p_hdr;
			}
		}
	}
	
	/* enqueue the free buffer in the appropriate list */
	idx = RVF_BUF_LIST_INDEX( GETSIZE(p_hdr) );
	p_hdr->header.p_prev = NULL;
	p_hdr->p_next = lists[idx];
	lists[idx] = p_hdr;
	if( p_hdr->p_next != NULL)
	{	(p_hdr->p_next)->header.p_prev = p_hdr;
	}

	ENDSIZE(p_hdr) = GETSIZE(p_hdr);
	RVF_SET_PREV_FREE( NEXTCHUNK(p_hdr) );

    rvf_enable();		/* exit critical section */
	return TRUE;
}


/*******************************************************************************
** Function         _str_cmp
**
** Description      Internal function which compares two null-terminated string.
**					Returns TRUE if they are equal, else FALSE.
*******************************************************************************/
BOOLEAN _str_cmp( char *str1, char * str2)
{	UINT8 i;
	for ( i = 0; (str1[i] == str2[i]) && (str1[i] != 0) && (str2[i] != 0) && (i < RVF_MAX_MB_LEN); i++ );
	if ( i == RVF_MAX_MB_LEN)
	{	return TRUE;
	}
	
	if ( (str1[i] == 0) && (str2[i] == 0) )
	{	return TRUE;
	}
	return FALSE;
}

/******************************************************************************
**
**						MEMORY BANK RELATED FUNCTIONS
**
******************************************************************************/

/*******************************************************************************
**
** Function         _remove_from_list
**
** Description      Internal function called to remove a buffer from the list of
**					buffer waiting to be counted on the memory bank.
**					The parameter points to the beginning of the header.
**
** Returns          BOOLEAN: TRUE if successful, else FALSE
**
*******************************************************************************/
BOOLEAN _remove_from_list (void *bptr, T_RVF_MB * mb)
{	UINT16 * index;
	UINT16 free_elem, prec;

	/* check all elements of the list */	
	index = &(mb->first_buffer_index);
	prec = RVF_INVALID_INDEX;

	while ( (*index != RVF_INVALID_INDEX) && (waiting_buffers[*index]!=bptr) )
	{	prec = *index;
		index = &(next_buffer[*index]);
	}
	
	if (waiting_buffers[*index] == bptr)
	{	free_elem = *index;
		*index = next_buffer[free_elem]; /* link preceding element to the next one */
		
		if (next_buffer[free_elem] == RVF_INVALID_INDEX ) /* last element in the list */
		{	mb->last_buffer_index = prec;
		}

		waiting_buffers[free_elem] = NULL;
		next_buffer[free_elem] = first_free_element; /* link free elements */
		first_free_element = free_elem;
		return TRUE;
	}
	return FALSE; /* buffer not found */
}


/*******************************************************************************
**
** Function         _add_to_list
**
** Description      Internal function called to add a buffer to the list of
**					buffer waiting to be counted on the memory bank.
**					The parameter points to the beginning of the header.
**
** Returns          BOOLEAN: TRUE if successful, else FALSE
**
*******************************************************************************/
BOOLEAN _add_to_list (void *bptr, T_RVF_MB * mb)
{	UINT16 index = first_free_element;
	
	first_free_element = next_buffer[index];
	
	waiting_buffers[index] = bptr;
	next_buffer[index] = RVF_INVALID_INDEX;

	if ( mb->last_buffer_index == RVF_INVALID_INDEX) /* empty list */
	{	mb->first_buffer_index	= index;
		mb->last_buffer_index	= index;
	}
	else
	{	next_buffer[mb->last_buffer_index] = index;
		mb->last_buffer_index = index;
	}
	
	return TRUE;
}


/*******************************************************************************
**
** Function         rvf_create_mb
**
** Description      Called by an application to create a memory bank
**
** Parameters:		memory bank name, memory bank param
**					(return) memory bank id
**
** Returns          T_RVF_RET: RVF_OK if success
**
*******************************************************************************/
T_RVF_RET		rvf_create_mb(T_RVF_MB_NAME mb_name, T_RVF_MB_PARAM mb_param, T_RVF_MB_ID *mb_id)
{	UINT8 num_mb;
	T_RVF_MB * mb;
	UINT32 available_mem = 0;
	UINT32 required_mem = 0;

	/* find the mb name in the array */
	for ( num_mb = 0; (num_mb < RVF_MAX_TOTAL_MB) && (!_str_cmp(mb_name, rvf_name_id[num_mb].mb_name) ); num_mb++);

	if ( num_mb == RVF_MAX_TOTAL_MB ) /* mb name not found */
	{	
		/* DLC added for dynamic memory bank creation*/

		/* search the first available place in the array */
		T_RVF_MB_ID first_available_mb_id = 0;
		BOOLEAN mb_id_found = FALSE; 

		for ( num_mb = 0; (num_mb < RVF_MAX_TOTAL_MB) && (rvf_name_id[num_mb].mb_name[0] != 0) ; num_mb++)
		{	if( rvf_name_id[num_mb].mb_id == first_available_mb_id)
			{	first_available_mb_id ++;
			}
		}
		
		while( (first_available_mb_id < RVF_MAX_REAL_MB) &&  (mb_id_found == FALSE) )
		{	
			for ( num_mb = 0; (num_mb < RVF_MAX_TOTAL_MB) && (rvf_name_id[num_mb].mb_name[0] != 0) && (rvf_name_id[num_mb].mb_id != first_available_mb_id) ; num_mb++);
			if ( rvf_name_id[num_mb].mb_id != first_available_mb_id)
			{	/* available mb id found */
				mb_id_found = TRUE;
			}
			else
			{	/* try the next one */
				first_available_mb_id++;
			}
		}

		if ( (num_mb == RVF_MAX_TOTAL_MB) || (first_available_mb_id + 1 >= RVF_MAX_REAL_MB ) ) /* no available space in the array */

	{	*mb_id = RVF_INVALID_MB_ID;
		return RVF_INVALID_PARAMETER;
	}

	if(INC_Initialize_State==INC_END_INITIALIZE) rvf_disable(20);		/* enter critical section */

		/* create the new mb name and id */
		strcpy( rvf_name_id[num_mb].mb_name, mb_name);
		rvf_name_id[num_mb].mb_id = first_available_mb_id;
		
		/* initialize the next one */
		rvf_name_id[num_mb+1].mb_name[0]	= 0;
		rvf_name_id[num_mb+1].mb_id			= 0;

	if(INC_Initialize_State==INC_END_INITIALIZE) rvf_enable();		/* exit critical section */
	}

	/* check if the memory bank has been already created */
	if ( rvf_name_id[ num_mb].mb_params.size != 0) 
	{	*mb_id = RVF_INVALID_MB_ID;
		return RVF_INTERNAL_ERR;
	}

	rvf_get_available_mem( &available_mem, &required_mem);

	if ( ( (required_mem + mb_param.size)*100) > ( available_mem * _rvf_get_mem_usage_ratio()) )	/* if there is not enough available memory to create this mb */
	{	*mb_id = RVF_INVALID_MB_ID;				/* In a next version: try to create a dynamic pool */
		return RVF_MEMORY_ERR;
	}

	rvf_disable(20);		/* enter critical section */

	/* save the mb parameters for deletion */
	rvf_name_id[num_mb].mb_params.size = mb_param.size;
	rvf_name_id[num_mb].mb_params.watermark = mb_param.watermark;

	* mb_id = rvf_name_id[num_mb].mb_id;
	mb = &rvf_banks[ *mb_id ];
	/* initialize the memory bank structure */
	mb->watermark				+= mb_param.watermark;
	mb->max						+= mb_param.size;
	
	rvf_enable();		/* exit critical section */

	return RVF_OK;
}


/*******************************************************************************
**
** Function         rvf_get_mb_id
**
** Description      Called by an application to get the memory bank id from its name
**
** Parameters:		memory bank name
**					(return) memory bank id
**
** Returns          T_RVF_RET: RVF_OK if success
**
*******************************************************************************/
T_RVF_RET		rvf_get_mb_id(T_RVF_MB_NAME mb_name, T_RVF_MB_ID *mb_id)
{	UINT8 num_mb;
	

	/* find the mb name in the array */
	for ( num_mb = 0; (num_mb < RVF_MAX_TOTAL_MB) && (!_str_cmp(mb_name, rvf_name_id[num_mb].mb_name) ); num_mb++);
	if ( num_mb == RVF_MAX_TOTAL_MB ) /* mb name not found */
	{	*mb_id = RVF_INVALID_MB_ID;
		return RVF_INVALID_PARAMETER;
	}
	if ( rvf_banks[ rvf_name_id[num_mb].mb_id ].max == 0 )
	{	/* the memory bank has not been created */
		*mb_id = RVF_INVALID_MB_ID;
		return 	RVF_NOT_READY;
	}
	*mb_id = rvf_name_id[num_mb].mb_id;
	return RVF_OK;
}


/*******************************************************************************
**
** Function         rvf_delete_mb
**
** Description      Called by an application to delete a memory bank
**
** Parameters:		memory bank name
**
** Returns          T_RVF_RET: RVF_OK if success
**
*******************************************************************************/
T_RVF_RET		rvf_delete_mb(T_RVF_MB_NAME mb_name)
{	UINT8 num_mb;
	T_RVF_MB * mb;

	/* find the mb name in the array */
	for ( num_mb = 0; (num_mb < RVF_MAX_TOTAL_MB) && (!_str_cmp(mb_name, rvf_name_id[num_mb].mb_name) ); num_mb++);
	if ( num_mb == RVF_MAX_TOTAL_MB ) /* mb name not found */
	{	return RVF_INVALID_PARAMETER;
	}
	mb = &rvf_banks[ rvf_name_id[num_mb].mb_id ];

	/* check if the mb is used more than once or not */
	if ( mb->max == rvf_name_id[num_mb].mb_params.size )
	{	/* mb is used only once, check if cur_memory_used > 0 */
		if ( mb->cur_memory_used > 0)
		{	rvf_send_trace( "RVF: rvf_delete_mb(): not all buffers have been freed", 53, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
			return RVF_MEMORY_ERR;
			/*	free all buffers ????? -> NOT POSSIBLE */
		}
		/* initialize mb params */
		mb->max					= 0;
		mb->first_buffer_index	= RVF_INVALID_INDEX;
		mb->last_buffer_index	= RVF_INVALID_INDEX;
		mb->watermark			= 0;
		mb->returned_red		= FALSE;
		mb->func				= 0;
#if RVF_ENABLE_STATS
	mb->max_reached				= 0;			
	mb->required_size			= 0;			
	mb->num_buf					= 0;				
#endif

	}
	else	/* mb is still used by another entity */
	{	/* change mb params */
		mb->max			-= rvf_name_id[num_mb].mb_params.size;
		mb->watermark	-= rvf_name_id[num_mb].mb_params.watermark;
	}
	
	rvf_name_id[num_mb].mb_params.size = 0;
	rvf_name_id[num_mb].mb_params.watermark = 0;

	return RVF_OK;
}

T_RVF_RET		rvf_mb_is_used(T_RVF_MB_NAME mb_name, UINT8* isUsed) 	{
	UINT8 num_mb;
	T_RVF_MB * mb;

	*isUsed=0;

	/* find the mb name in the array */
	for ( num_mb = 0; (num_mb < RVF_MAX_TOTAL_MB) && (!_str_cmp(mb_name, rvf_name_id[num_mb].mb_name) ); num_mb++);
	if ( num_mb == RVF_MAX_TOTAL_MB ) {/* mb name not found */
		return RVF_INVALID_PARAMETER;
	}
	mb = &rvf_banks[ rvf_name_id[num_mb].mb_id ];

	/* check if the mb is used more than once or not */
	if ( mb->max == rvf_name_id[num_mb].mb_params.size ) {
		/* mb is used only once, check if cur_memory_used > 0 */
		if ( mb->cur_memory_used > 0) *isUsed=1;
	}
	return RVF_OK;
}



/*******************************************************************************
**
** Function         rvf_get_mb_status
**
** Description      Called by an application to get the status of a memory bank
**
** Parameters:		memory bank id
**
** Returns          T_RVF_MB_STATUS:	RVF_GREEN if everything is ok,
**										RVF_YELLOW if watermark limit has been reached,
**										RVF_RED if max size has been reached
**
*******************************************************************************/
T_RVF_MB_STATUS	rvf_get_mb_status(T_RVF_MB_ID mb_id)
{	T_RVF_MB * mb;

	/* checking for invalid memory bank IDs */
	if (mb_id >= RVF_MAX_REAL_MB)
	{
#ifdef RVM_DAR_SWE
		dar_diagnose_generate_emergency (RVF_INVALID_MD_ID_ERRROR,
										 DAR_ASCII_FORMAT,
										 RVM_USE_ID);
#else
		rvf_send_trace (RVF_INVALID_MD_ID_ERRROR,
						sizeof (RVF_INVALID_MD_ID_ERRROR) - 1,
						NULL_PARAM,
						RV_TRACE_LEVEL_ERROR,
						RVM_USE_ID);
#endif
		return RVF_RED;
	}
	mb = &rvf_banks[mb_id];
	
	if ( mb->returned_red == TRUE) /* if a previous count buf or get buf has failed */
	{	return RVF_RED;
	}

	if ( mb->cur_memory_used < mb->watermark )
	{	return RVF_GREEN;
	}
	else
	{	if ( mb->cur_memory_used < mb->max )
		{	return RVF_YELLOW;
		}
		else
		{	return RVF_RED; /* since max and cur_memory_used are set to 0 for not-created mb, it will return RED*/ 
		}
	}
}

/*******************************************************************************
**
** Function         rvf_get_mb_unused_mem
**
** Description      Called by an application to get the number of bytes available
**					until the memory bank size.
**
** Parameters:		memory bank id
**
** Returns          UINT32:	number of bytes available
**					returns 0 if the memory bank has not been created.
**
*******************************************************************************/
UINT32	rvf_get_mb_unused_mem(T_RVF_MB_ID mb_id)
{	T_RVF_MB * mb;
	mb = &rvf_banks[mb_id];
	
	if ( mb->returned_red == TRUE) /* if a previous count buf or get buf has failed */
	{	return 0;
	}

	return( mb->max - mb->cur_memory_used);
}

/*******************************************************************************
**
** Function         rvf_get_mb_unused_green_mem
**
** Description      Called by an application to get the number of bytes available
**					until the memory bank watermark.
**
** Parameters:		memory bank id
**
** Returns          UINT32:	number of bytes available
**					returns 0 if the memory bank has not been created.
**
*******************************************************************************/
UINT32	rvf_get_mb_unused_green_mem(T_RVF_MB_ID mb_id)
{	T_RVF_MB * mb;
	mb = &rvf_banks[mb_id];
	
	if ( mb->returned_red == TRUE) /* if a previous count buf or get buf has failed */
	{	return 0;
	}

	if( mb->cur_memory_used > mb->watermark)
	{	return 0;
	}

	return( mb->watermark - mb->cur_memory_used);
}

/*******************************************************************************
**
** Function         rvf_get_buf
**
** Description      Called by an application to get a buffer from a memory bank
**
** Parameters:		memory bank id, buffer size
**					(return) buffer pointer to the allocated buffer or null if 
**					mb status is RVF_RED
**
** Returns          T_RVF_MB_STATUS:	RVF_GREEN if everything is ok,
**										RVF_YELLOW if watermark limit has been reached,
**										RVF_RED if max size has been reached (does not return a buffer)
**
*******************************************************************************/
T_RVF_MB_STATUS rvf_get_buf(T_RVF_MB_ID mb_id, UINT32 buffer_size, T_RVF_BUFFER** p_buffer)
{	T_RVF_MB * mb;

	/* checking for invalid memory bank IDs */
	if (mb_id >= RVF_MAX_REAL_MB)
	{
#ifdef RVM_DAR_SWE
		dar_diagnose_generate_emergency (RVF_INVALID_MD_ID_ERRROR,
										 DAR_ASCII_FORMAT,
										 RVM_USE_ID);
#else
		rvf_send_trace (RVF_INVALID_MD_ID_ERRROR,
						sizeof (RVF_INVALID_MD_ID_ERRROR) - 1,
						NULL_PARAM,
						RV_TRACE_LEVEL_ERROR,
						RVM_USE_ID);
#endif
		return RVF_RED;
	}
	mb = &rvf_banks[ mb_id ];
	
	/* check memory usage */
	if ( ( mb->cur_memory_used + buffer_size) >  mb->max )
	{	/* over the limits, return RED and do not allocate a buffer */
		mb->returned_red = TRUE;
		*p_buffer = NULL;
		return RVF_RED;
	}
	
	/* find a buffer of the requested size */
	*p_buffer = _find_buf( buffer_size );
	if ( *p_buffer == NULL ) /* error during allocation, not enough memory */
	{	//rvf_send_trace( "RVF: rvf_get_buf(): not enough available physical memory", 56, NULL_PARAM, RV_TRACE_LEVEL_ERROR, TRACE_RVF_BUFFER );
		mb->returned_red = TRUE;
		return RVF_RED;
	}

#if RVF_ENABLE_STATS
	required_size	+= buffer_size;
	obtained_size	+= rvf_get_buf_size((UINT8*)(*p_buffer) + sizeof(T_RVF_INTERNAL_BUF));
	used_size		+= rvf_get_buf_size((UINT8*)(*p_buffer) + sizeof(T_RVF_INTERNAL_BUF)) + sizeof(T_RVF_INTERNAL_BUF) + RVF_CORRUPT_OVERHEAD;
	mem_in_use		+= rvf_get_buf_size((UINT8*)(*p_buffer) + sizeof(T_RVF_INTERNAL_BUF)) + sizeof(T_RVF_INTERNAL_BUF) + RVF_CORRUPT_OVERHEAD;
#endif

	if(INC_Initialize_State==INC_END_INITIALIZE) rvf_disable(20);		/* enter critical section */
#if RVF_ENABLE_BUF_LINKAGE_CHECK
	/* set as unlinked */
	RVF_SET_BUF_UNLINKED((T_RVF_INTERNAL_BUF*)(*p_buffer));
#endif

	/* increase memory use counter */
	mb->cur_memory_used += rvf_get_buf_size( (UINT8*)(*p_buffer) + sizeof(T_RVF_INTERNAL_BUF) );
	( (T_RVF_INTERNAL_BUF *) (*p_buffer))->header.external.mb_id = mb_id;
	( (T_RVF_INTERNAL_BUF *) (*p_buffer))->header.external.mb_expected = RVF_INVALID_MB_ID;
	
	*p_buffer = (UINT8*)(*p_buffer) + sizeof(T_RVF_INTERNAL_BUF);

#if RVF_ENABLE_STATS
	if ( mb->cur_memory_used > mb->max_reached )
	{	mb->max_reached = mb->cur_memory_used;
	}
	mb->required_size += buffer_size;
	mb->num_buf++;
#endif

	if(INC_Initialize_State==INC_END_INITIALIZE) rvf_enable();			/* exit critical section */

	/* return the correct flag */
	if ( mb->cur_memory_used > mb->watermark )
	{	return RVF_YELLOW;
	}
	else
	{	return RVF_GREEN;
	}
}

/*******************************************************************************
**
** Function         rvf_count_buf
**
** Description      Called by an application to change the memory bank on which a buffer is counted
**
** Parameters:		new memory bank id,
**					pointer to the buffer.
**
** Returns          T_RVF_MB_STATUS:	RVF_GREEN if everything is ok,
**										RVF_YELLOW if watermark limit has been reached,
**										RVF_RED if max size has been reached
**
*******************************************************************************/
T_RVF_MB_STATUS rvf_count_buf(T_RVF_MB_ID mb_id, T_RVF_BUFFER * p_buffer)
{	T_RVF_INTERNAL_BUF * buf;
	UINT32 buf_size;
	T_RVF_MB * new_mb;
	T_RVF_MB * old_mb;

	/* checking for invalid memory bank IDs */
	if (mb_id >= RVF_MAX_REAL_MB)
	{
#ifdef RVM_DAR_SWE
		dar_diagnose_generate_emergency (RVF_INVALID_MD_ID_ERRROR,
										 DAR_ASCII_FORMAT,
										 RVM_USE_ID);
#else
		rvf_send_trace (RVF_INVALID_MD_ID_ERRROR,
						sizeof (RVF_INVALID_MD_ID_ERRROR) - 1,
						NULL_PARAM,
						RV_TRACE_LEVEL_ERROR,
						RVM_USE_ID);
#endif
		return RVF_RED;
	}
	buf = (T_RVF_INTERNAL_BUF *) ( (UINT8 *) p_buffer - sizeof(T_RVF_INTERNAL_BUF) );
	new_mb = &rvf_banks[mb_id];
	old_mb = &rvf_banks[buf->header.external.mb_id];
		
	/* get the size of the buffer and try to count it on the new mb */
	buf_size = rvf_get_buf_size( p_buffer );
	
	if ( ( new_mb->cur_memory_used + buf_size ) < new_mb->max )
	{	/* there is enough memory in the new mb */
		
		if ( buf->header.external.mb_expected != RVF_INVALID_MB_ID )
		{	/* remove the buffer from the list of waiting buffers in mb expected */
			_remove_from_list( buf, &rvf_banks[buf->header.external.mb_expected] );
			buf->header.external.mb_expected = RVF_INVALID_MB_ID;
		}
		
		rvf_disable(20);		/* enter critical section */

		/* decrease the memory used in the old mb */
		old_mb->cur_memory_used -= buf_size;
	
		/* increase memory used in the new mb */
		new_mb->cur_memory_used += buf_size;

		rvf_enable();			/* exit critical section */

		/* call the callback function if state of the old mb switches to RVF_GREEN and there is no buffer waiting */
		if ( (old_mb->cur_memory_used < old_mb->watermark) && (old_mb->first_buffer_index == RVF_INVALID_INDEX) && (old_mb->returned_red == TRUE) && ( old_mb->func != NULL) )
		{	old_mb->returned_red = FALSE;
			old_mb->func( buf->header.external.mb_id);
		}
		else
		{	/* count as many waiting buffers as possible on the old mb */
			while( (old_mb->first_buffer_index != RVF_INVALID_INDEX) && (rvf_count_buf( buf->header.external.mb_id, (UINT8 *) (waiting_buffers[old_mb->first_buffer_index]) + sizeof(T_RVF_INTERNAL_BUF)) != RVF_RED) );
			if ( (old_mb->cur_memory_used < old_mb->watermark) && (old_mb->first_buffer_index == RVF_INVALID_INDEX) && (old_mb->returned_red == TRUE) && ( old_mb->func != NULL) )
			{	old_mb->returned_red = FALSE;
				old_mb->func( buf->header.external.mb_id);
			}
		}
		/* change mb_id of the buffer */
		buf->header.external.mb_id = mb_id;



#if RVF_ENABLE_STATS
	if ( new_mb->cur_memory_used > new_mb->max_reached )
	{	new_mb->max_reached = new_mb->cur_memory_used;
	}
#endif

		if ( new_mb->cur_memory_used > new_mb->watermark )
		{	return RVF_YELLOW;
		}
		else
		{	return RVF_GREEN;
		}
	}
	else
	{	/* there is not enough memory in the new mb_id */
		if ( buf->header.external.mb_expected != RVF_INVALID_MB_ID)	/* remove the buffer from old expected mb */
		{	_remove_from_list( buf, &rvf_banks[ buf->header.external.mb_expected] );
		}
		/* enqueue the buffer in the list of waiting buffer */
		buf->header.external.mb_expected = mb_id;
		_add_to_list( buf, new_mb );
		new_mb->returned_red = TRUE;
		return RVF_RED;
	}
}

/*******************************************************************************
**
** Function         rvf_free_buf
**
** Description      Called by an application to free a buffer
**
** Parameters:		buffer pointer
**
** Returns          T_RVF_RET:	T_RVF_OK if succesful,
**
*******************************************************************************/
T_RVF_RET rvf_free_buf( T_RVF_BUFFER * p_buffer)
{	T_RVF_INTERNAL_BUF * buf;
	T_RVF_MB * mb;
	T_RVF_MB_ID mb_id;
	UINT32 buf_size;
	
	
	buf = USER2MEM(p_buffer);

	mb_id = buf->header.external.mb_id;
	mb = &rvf_banks[mb_id];
	buf_size = rvf_get_buf_size(p_buffer);

	/* do not need to change the mb id of the buffer, since it will be overwritten by p_prev */

	/* free the buffer */
	if ( !_release_buf(buf) )
	{	return RVF_MEMORY_ERR;
	}

#if RVF_ENABLE_STATS
	mem_in_use		-= buf_size + sizeof(T_RVF_INTERNAL_BUF) + RVF_CORRUPT_OVERHEAD;
#endif

	rvf_disable(20);		/* enter critical section */

	/* decrease mb memory use */
	mb->cur_memory_used -= buf_size;

	rvf_enable();			/* exit critical section */


	/* call the callback function if state of the mb switches to RVF_GREEN and there is no buffer waiting */
	if ( (mb->returned_red == TRUE) && (mb->cur_memory_used < mb->watermark) && ( mb->first_buffer_index == RVF_INVALID_INDEX) && ( mb->func != NULL) )
		{	mb->returned_red = FALSE;
			mb->func( mb_id);
		}
		else
		{	/* count as many waiting buffers as possible on the mb */
			while( (mb->first_buffer_index != RVF_INVALID_INDEX) && (rvf_count_buf( mb_id, (UINT8 *) (waiting_buffers[mb->first_buffer_index]) + sizeof(T_RVF_INTERNAL_BUF)) != RVF_RED) );
			if ( (mb->cur_memory_used < mb->watermark) && ( mb->first_buffer_index == RVF_INVALID_INDEX) && (mb->returned_red == TRUE) && ( mb->func != NULL) )
			{	mb->returned_red = FALSE;
				mb->func( mb_id);
			}
		}

	return RVF_OK;
}


/*******************************************************************************
**
** Function         rvf_set_callback_func
**
** Description      Called the first time an application wants to set the callback
**					function associated to a memory bank.
**
** Parameters:		memory bank id, pointer to the callback function.
**
** Returns          T_RVF_RET:	T_RVF_OK if succesful,
**
*******************************************************************************/
T_RVF_RET		rvf_set_callback_func(T_RVF_MB_ID mb_id, MB_CALLBACK_FUNC func)
{	T_RVF_MB * mb;

	/* checking for invalid memory bank IDs */
	if (mb_id >= RVF_MAX_REAL_MB)
		return RVF_INVALID_PARAMETER;
	mb = &rvf_banks[mb_id];
	if ( (mb->max != 0) && (mb->func == NULL) )	/* if the mb has been created and the callback func has never been set */
	{	mb->func = func;
		return RVF_OK;
	}
	return RVF_INVALID_PARAMETER;	
}


/*******************************************************************************
**
** Function         rvf_change_callback_func
**
** Description      Called by an application to change the callback
**					function associated to a memory bank.
**					It means the callback function has to be set before.
**
** Parameters:		memory bank id, pointer to the callback function.
**
** Returns          T_RVF_RET:	T_RVF_OK if succesful,
**
*******************************************************************************/
T_RVF_RET		rvf_change_callback_func(T_RVF_MB_ID mb_id, MB_CALLBACK_FUNC func)
{	T_RVF_MB * mb;

	/* checking for invalid memory bank IDs */
	if (mb_id >= RVF_MAX_REAL_MB)
		return RVF_INVALID_PARAMETER;
	mb = &rvf_banks[mb_id];
	if ( (mb->max != 0) && (mb->func != NULL) )	/* if the mb has been created and the callback func has already been set */
	{	mb->func = func;
		return RVF_OK;
	}
	return RVF_INVALID_PARAMETER;	
}

#ifdef _WINDOWS_
/*******************************************************************************
**
** Function         _rvf_window_dump_mem
**
** Description      Called by a C++ object to update the graphical display of
**                  memory
**
** Parameters:		Opaque pointer to C++ object
**
** Returns          void
**
** WARNING          DON'T CHANGE THE SYNTAX OF DISPLAYED DATAS SINCE
**                  THEY ARE PARSED TO DISPLAY GRAPHICALLY THE MEMORY
**                  STATUS
**
*******************************************************************************/
void _rvf_window_dump_mem(void *m)
{

	
#if RVF_ENABLE_STATS	/* conditional compilation if stats are enabled */
	char mb_info[100];
	UINT16 num_mb, num_buf, index;
	UINT32 total_mem_size = 0;
	UINT32 total_max_used = 0;
	UINT32 total_cur_used = 0;
	T_RVF_MB * mb;


	for ( num_mb = 0; num_mb < RVF_MAX_TOTAL_MB; num_mb++ )
	{	
		/* trace the mb if it has been created*/
		if ( rvf_name_id[num_mb].mb_params.size > 0 )
		{	mb = &rvf_banks[ rvf_name_id[num_mb].mb_id ]; 
			num_buf = 0;
			index = mb->first_buffer_index;
			while ( index != RVF_INVALID_INDEX)
			{	num_buf++;
				index = next_buffer[index];
			}
			strcpy(mb_info,rvf_name_id[num_mb].mb_name);
			mb_info[10]='\0';
			AddNewState(m,mb_info,
				mb->cur_memory_used,mb->max_reached,mb->watermark,mb->max);
			
			
			total_mem_size += mb->max;
			total_max_used += mb->max_reached;
			total_cur_used += mb->cur_memory_used;
		}
	}
	
#endif /* RVF_ENABLE_STATS */
}

#endif

/*******************************************************************************
**
** Function         rvf_dump_mem
**
** Description      Called by an application to dump mb state and display statistics
**
** Parameters:		None
**
** Returns          void
**
** WARNING          DON'T CHANGE THE SYNTAX OF DISPLAYED DATAS SINCE
**                  THEY ARE PARSED TO DISPLAY GRAPHICALLY THE MEMORY
**                  STATUS
**
*******************************************************************************/
void rvf_dump_mem()
{
#if RVF_ENABLE_STATS	/* conditional compilation if stats are enabled */
	char mb_info[100];
	UINT16 num_mb, num_buf, index;
	UINT32 total_mem_size = 0;
	UINT32 total_max_used = 0;
	UINT32 total_cur_used = 0;
	T_RVF_MB * mb;


	/* display memory required, obtained and ratio */
	rvf_send_trace("MEM STAT: Total memory required", 31, required_size, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);
	rvf_send_trace("MEM STAT: Total memory obtained", 31, obtained_size, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);
	rvf_send_trace("MEM STAT: Total memory used    ", 31, used_size, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);
	sprintf( mb_info,"MEM STAT: Mem usage ratio      : %010f%%",  ( (double)(required_size) / (double)(obtained_size) ) *100 );
	rvf_send_trace(mb_info, 44, NULL_PARAM , RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);

	rvf_send_trace("*** START DUMPING MEMORY BANK ***", 33, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);
	/* for each mb, display its name, its id, its cur_size, its watermark, its limit, its max reached, the number of callback functions enqueued,
	(the requested size, the number of allocated buffer), the average buffer size for this mb */
	rvf_send_trace("**MB_NAME* Id Used_mem Watermark  Limit   Peak Nb_buff Avg_buf_size", 67, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);

	for ( num_mb = 0; num_mb < RVF_MAX_TOTAL_MB; num_mb++ )
	{	
		/* trace the mb if it has been created*/
		if ( rvf_name_id[num_mb].mb_params.size > 0 )
		{	mb = &rvf_banks[ rvf_name_id[num_mb].mb_id ]; 
			num_buf = 0;
			index = mb->first_buffer_index;
			while ( index != RVF_INVALID_INDEX)
			{	num_buf++;
				index = next_buffer[index];
			}
			sprintf( mb_info, "%10.10s %2d   %6d    %6d %6d %6d      %2d       %6d",
				rvf_name_id[num_mb].mb_name, rvf_name_id[num_mb].mb_id, 
				mb->cur_memory_used, mb->watermark, mb->max, mb->max_reached, num_buf,
				mb->num_buf == 0? 0:mb->required_size / mb->num_buf);

			rvf_send_trace( mb_info, 67, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);		

			total_mem_size += mb->max;
			total_max_used += mb->max_reached;
			total_cur_used += mb->cur_memory_used;
		}
	}
	sprintf(	mb_info, "TOTAL: ******** %6d********** %6d %6d",
				total_cur_used, total_mem_size, total_max_used );

	rvf_send_trace( mb_info, 46, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);		

#endif /* RVF_ENABLE_STATS */
}


/*******************************************************************************
**
** Function         rvf_get_available_mem
**
** Description      Called to get the total size of the memory and the used size.
**
** Parameters:		(return) UINT32 * total_size: contains the number of bytes in the memory.
**					(return) UINT32 * used_size:  contains the number of used bytes in the memory.
**
** Returns          T_RVF_RET: RVF_OK if successful, else a negative value.
**
*******************************************************************************/
T_RVF_RET rvf_get_available_mem( UINT32 * total_size, UINT32 * used_size )
{
	UINT16 count;
	*total_size = 0;
	*used_size	= 0;

	/* check if there is enough available physical memory (static or dynamic): if Sum(mb size) <= Sum(pool size)*/
	for ( count = 0; count < _rvf_get_number_of_pool(); count ++ )		/* sum the memory in pools */
	{	*total_size += _rvf_pools[count].pool_size;
	}
	/* sum the memory required by existing mb */
	for ( count = 0; count < RVF_MAX_REAL_MB; count ++ )
	{	*used_size	+= rvf_banks[count].max;
	}

	return RVF_OK;
}


/*******************************************************************************
**
** Function         rvf_get_mb_param
**
** Description      return the parameters of a specific memory bank 
**
** Parameters:		T_RVF_MB_NAME: name of the memory bank
**					(return) T_RVF_MB_PARAM*: parameter of the memory bank.
**
** Returns          T_RVF_RET: RVF_OK if successful, else a negative value.
**
*******************************************************************************/
T_RVF_RET rvf_get_mb_param( T_RVF_MB_NAME mb_name, T_RVF_MB_PARAM * param)
{	UINT8 num_mb;
	
	/* find the mb name in the array */
	for ( num_mb = 0; (num_mb < RVF_MAX_TOTAL_MB) && (!_str_cmp(mb_name, rvf_name_id[num_mb].mb_name) ); num_mb++);
	if ( num_mb == RVF_MAX_TOTAL_MB ) /* mb name not found */
	{	return RVF_INVALID_PARAMETER;
	}
	
	/* copy the parameters of the memory bank */
	*param = rvf_name_id[num_mb].mb_params;

	return RVF_OK;
}


/*******************************************************************************
**
** Function         rvf_set_mb_param
**
** Description      change the parameters of a specific memory bank 
**
** Parameters:		T_RVF_MB_NAME: name of the memory bank
**					T_RVF_MB_PARAM*: parameter of the memory bank.
**
** Returns          T_RVF_RET: RVF_OK if successful, else a negative value.
**
*******************************************************************************/
T_RVF_RET rvf_set_mb_param( T_RVF_MB_NAME mb_name, T_RVF_MB_PARAM * param)
{	UINT8 num_mb;
	T_RVF_MB_ID mb_id;
	T_RVF_MB * mb;

	/* find the mb name in the array */
	for ( num_mb = 0; (num_mb < RVF_MAX_TOTAL_MB) && (!_str_cmp(mb_name, rvf_name_id[num_mb].mb_name) ); num_mb++);
	if ( num_mb == RVF_MAX_TOTAL_MB ) /* mb name not found */
	{	return RVF_INVALID_PARAMETER;
	}

	mb_id = rvf_name_id[num_mb].mb_id;
	mb = &rvf_banks[ mb_id ];
	
	rvf_disable(20);		/* enter critical section */
	
	/* decrease the mb param by the old parameters */
	mb->watermark				-= rvf_name_id[num_mb].mb_params.watermark;
	mb->max						-= rvf_name_id[num_mb].mb_params.size;

	/* save the mb parameters */
	rvf_name_id[num_mb].mb_params.size = param->size;
	rvf_name_id[num_mb].mb_params.watermark = param->watermark;

	/* increase the mb param by the new parameters */
	mb->watermark				+= param->watermark;
	mb->max						+= param->size;
	
	rvf_enable();		/* exit critical section */

	/* dequeue callback functions if state switches to RVF_GREEN */

	return RVF_OK;
}




/*******************************************************************************
**
** Function         rvf_empty_mailboxes
**
** Description      Called by rvf_exit_task to empty a task's mailboxes before
**					killing it
**
** Parameters:		task_id: task of wich the mailboxes have to be emptied
**
** Returns          RV_OK
**
*******************************************************************************/
T_RV_RET _rvf_empty_mailboxes (T_RVF_G_ADDR_ID task_id)
{
    void * p_buf	= NULL;
    T_RVF_INTERNAL_BUF * p_hdr;
	UINT8 mbox_id = 0;

    if (task_id >= MAX_RVF_TASKS)
    {
		rvf_send_trace( "RVF: rvf_empty_mbox(): invalid taskid: ", 39, task_id, RV_TRACE_LEVEL_ERROR, RVM_USE_ID );
        return RVF_INVALID_PARAMETER;
    } 

	for (mbox_id = 0; mbox_id< RVF_NUM_TASK_MBOX; mbox_id++)
	{   /* while the chained list is not empty */
		/*while ( OSTaskQFirst[task_id][mbox_id] )
		{
		    rvf_disable(9);		

			p_hdr = OSTaskQFirst[task_id][mbox_id];
			OSTaskQFirst[task_id][mbox_id] = p_hdr->p_next;
		*/
		while ( pRtAddrIdTable[task_id]->OSTaskQFirst[mbox_id]  )
		{
		    rvf_disable(9);		

			p_hdr = pRtAddrIdTable[task_id]->OSTaskQFirst[mbox_id];
			pRtAddrIdTable[task_id]->OSTaskQFirst[mbox_id] = p_hdr->p_next;

#if RVF_ENABLE_BUF_LINKAGE_CHECK
			/* set as unlinked */
			RVF_SET_BUF_UNLINKED(p_hdr);
#endif

			p_buf = (UINT8 *)p_hdr + sizeof(T_RVF_INTERNAL_BUF);

			rvf_enable();		/* exit critical section */

			rvf_free_buf(p_buf);
		}
	}


    return (RV_OK);
}


/*******************************************************************************
**
** Function         rvf_scan_next
**
** Description      return the next item in the queue if any.
**					If the end of the queue is reached, returns NULL.
**					If current item is NULL, returns the first item in the queue.
**
** Returns          NULL if the end of the queue is reached, else a pointer to the buffer.
**
*******************************************************************************/
T_RVF_BUFFER *	rvf_scan_next (T_RVF_BUFFER_Q * p_q, T_RVF_BUFFER * p_buf)
{
    T_RVF_INTERNAL_BUF    *p_hdr;

    if (!p_q->count)	/* if the queue is empty */
        return (NULL);

	if (p_buf == NULL)  /* if current item == NULL, returns the first one */
	{
		return (T_RVF_BUFFER*)p_q->p_first;
	}

    rvf_disable(12);	/* enter critical section */

    p_hdr = (T_RVF_INTERNAL_BUF *)((UINT8 *)p_buf - sizeof(T_RVF_INTERNAL_BUF));
	p_hdr = p_hdr->p_next;
	if (p_hdr != NULL)
	{	p_hdr = (T_RVF_INTERNAL_BUF*) ((UINT8 *)p_hdr + sizeof(T_RVF_INTERNAL_BUF));
	}
	
	rvf_enable();

	return (T_RVF_BUFFER*)p_hdr;
}


/*******************************************************************************
**
** Function         rvf_remove_from_queue
**
** Description      remove a specific item from a queue
**
** Returns          RVF_OK if the item is removed, RVF_INTERNAL_ERR else.
**
*******************************************************************************/
T_RVF_RET		rvf_remove_from_queue (T_RVF_BUFFER_Q * p_q, T_RVF_BUFFER * p_buf)
{
    T_RVF_INTERNAL_BUF    *p_hdr;
	T_RVF_INTERNAL_BUF    *p_target;

	
    if (!p_q->count)	/* if the queue is empty */
        return (RVF_INTERNAL_ERR);

    rvf_disable(12);	/* enter critical section */

	/* find the specific item in the queue */
	p_target = USER2MEM((UINT8*)p_buf);
	p_hdr = USER2MEM( (UINT8 *)p_q->p_first);
	
	if( p_hdr == p_target )
	{	/* the specific item is the first one */
		rvf_dequeue( p_q);
		rvf_enable();
		return (RVF_OK);
	}
	
	while( (p_hdr->p_next != p_target) && (p_hdr->p_next != NULL) )
	{	p_hdr = p_hdr->p_next;
	}
	
	if( p_hdr->p_next == NULL) /* item not found */
	{	rvf_enable();
		return (RVF_INTERNAL_ERR);
	}


	p_hdr->p_next = p_target->p_next;
	p_q->count--;
    
    /* if we removed the last buffer */
    if (p_q->p_last == p_buf)
	{	p_q->p_last = p_hdr + sizeof(T_RVF_INTERNAL_BUF);
	}   

    p_target->p_next = NULL;

#if RVF_ENABLE_BUF_LINKAGE_CHECK
	/* set as unlinked */
	RVF_SET_BUF_UNLINKED(p_target);
#endif

    rvf_enable();		/* exit critical section */

    return (RVF_OK);
}

/*******************************************************************************
**
** Function         rvf_dump_pool
**
** Description      Called by an application to dump memory pool usage
**
** Parameters:		None
**
** Returns          void
**
*******************************************************************************/
void rvf_dump_pool()
{
#if RVF_ENABLE_STATS	/* conditional compilation if stats are enabled */
	
	UINT16 num_pool;
	UINT32 total_mem = 0;

	/* display memory stats */
	rvf_send_trace("*** START DUMPING MEMORY ***", 28, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);

	/* display the total amount of memory available in the system */
	/* and the total amount of memory currently in use */
	for( num_pool = 0; num_pool < _rvf_get_number_of_pool(); num_pool++)
	{	total_mem	+= _rvf_pools[num_pool].pool_size;
	}
		
	rvf_send_trace( "Total memory available ", 23, total_mem, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);		
	rvf_send_trace( "Memory currently in use", 23, mem_in_use, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);		

#endif /* RVF_ENABLE_STATS */
}

static void rvf_free_protected_buf(void *buf)
{
	rvf_free_buf(buf);
}

void rvf_get_protected_buf(T_RVF_MB_ID mb_id, UINT32 buffer_size, T_RVF_BUFFER** p_buffer)
{
   T_RVF_MB_STATUS err;
   err=rvf_get_buf(mb_id,buffer_size,p_buffer);
   if (err==RVF_GREEN)
   {
	   struct _protectedPtr_ *ptr;
	   struct _exceptionContext_ *context;

	   context=_currentExceptionContext_[rvf_get_taskid()];
	   err=rvf_get_buf(mb_id,sizeof(struct _protectedPtr_),(void*)&ptr);
	   ptr->next=NULL;
	   ptr->previous=NULL;
	   ptr->ptr=*p_buffer;
	   ptr->func=rvf_free_protected_buf;

	   if (err==RVF_GREEN)
	   {
          if (context->stack==NULL)
		  {
			  context->stack=ptr;
		  }
		  else
		  {
			  ptr->previous=context->stack;
			  context->stack->next=ptr;
			  context->stack=ptr;
		  }
	   }
	   else
	   {
		   rvf_free_buf(*p_buffer);
	   }
   }
   if (err!=RVF_GREEN)
	   throw(E_not_enough_memory);
}

//TISHMMS Project
/* add by xmzhou_trace_string to trace debug trace message */
void xmzhou_trace_string(char * string2trace)
{
rvf_send_trace(string2trace,strlen(string2trace),NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, RVM_USE_ID);
}

void xmzhou_trace_string_value(char * string2trace,UINT32 value)
{
rvf_send_trace(string2trace,strlen(string2trace),value, RV_TRACE_LEVEL_DEBUG_LOW, RVM_USE_ID);
}

static void mychar2hex(char ch, char *str)
{
    unsigned char   h_nibble = (ch >> 4) & 0x0F;
    unsigned char   l_nibble = ch & 0x0F;

    if (h_nibble < 10)
        str[0] = 48 + h_nibble;
    else
        str[0] = 55 + h_nibble;

    if (l_nibble < 10)
        str[1] = 48 + l_nibble;
    else
        str[1] = 55 + l_nibble;
}

void xmzhou_trace_n_bytes(char * buffer0, UINT32 len)
{
int i;
int traced_len=0;
char mybuffer[40];
char *tracebuffer;

if(buffer0==NULL) return;

xmzhou_trace_string_value("xmzhou dumping data length=",len);
xmzhou_trace_string("-------");
	

while(traced_len<len){

	if((len-traced_len)>=16){
	  tracebuffer=mybuffer;
	  for (i = 0; i < 16; i++) {
    	mychar2hex (buffer0[traced_len+i], tracebuffer);
	    tracebuffer += 2;
  	  }
	  traced_len+=16;
	  *tracebuffer = '\0';
  	  rvf_send_trace(mybuffer,strlen(mybuffer),NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, RVM_USE_ID);
  	  rvf_delay(20);
	}else{
      tracebuffer=mybuffer;
	  for(i=0;i<(len-traced_len);i++)
	  	{
	    	mychar2hex (buffer0[traced_len+i], tracebuffer);
		    tracebuffer += 2;
	  	}
	  *tracebuffer = '\0';
  	  rvf_send_trace(mybuffer,strlen(mybuffer),NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, RVM_USE_ID);
      rvf_delay(20);
	  break;
	}

}
}

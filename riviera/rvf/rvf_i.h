/****************************************************************************/
/*                                                                          */
/*  Name        rvf_i.h                                                     */
/*                                                                          */
/*  Function    this file contains rvf private definitions                  */
/*                                                                          */
/*  Version		0.4															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  3/12/99		Create														*/
/*	30/11/99	compliant to RV coding guidelines							*/
/*  12/23/1999	change buffer structures, add memory bank related structures*/
/*  02/21/2000	change memory bank implementation.							*/
/*  12/07/2000	implement dynamic memory allocation.						*/
/*																			*/
/*	Author		David Lamy-Charrier (dlamy@tif.ti.com)						*/
/*																			*/
/* (C) Copyright 1999 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/


#ifndef _RVF_I_H
#define _RVF_I_H


#include "../rv/general.h"
#include "rvf_api.h"
#include "../../nucleus/nucleus.h" 	/* A-M-E-N-D-E-D! */
#include "../rvm/rvm_i.h"

/********************************************************************/
/**  Buffer Management Data Structures                             **/
/********************************************************************/
#define MAX_RVF_G_ADDR_ID	200


/* define the OVERHEAD per buffer */
#if RVF_ENABLE_BUF_CORRUPTION_CHECK
	#define RVF_CORRUPT_OVERHEAD (sizeof(UINT32))
	BOOLEAN _rvf_chk_buf_damage(void *bptr);
#else
	#define RVF_CORRUPT_OVERHEAD 0
#endif
/* calculate the size required in bytes, add the overhead for buffer corruption and round up to a multiple of 4 */
#define REQ2SIZE(size) ( ((size) + 3 + RVF_CORRUPT_OVERHEAD) & ~0x00000003)


#define RVF_MIN_USABLE_SIZE	( sizeof(T_RVF_INTERNAL_BUF) + sizeof(UINT32) )

/* macros to get the internal header from the user pointer and vice-versa */
#define USER2MEM(buf)  ( (T_RVF_INTERNAL_BUF*)(((UINT8*)(buf)) - sizeof(T_RVF_INTERNAL_BUF) ) )
#define MEM2USER(buf)  ( ((UINT8*)(buf)) + sizeof(T_RVF_INTERNAL_BUF) )


/* NOTE: since buffer size is always a multiple of 4, the last 2 bits may be used for flags */

#define GETSIZE(hdr)	((UINT32)(hdr->buf_size & ~0x03) )

/* macros to manage if buffers are linked or not */
#define RVF_BUF_IS_LINKED(hdr) 		(0x00000001 & hdr->buf_size)
#define RVF_SET_BUF_LINKED(hdr)		( (hdr)->buf_size |= 0x00000001)

#define RVF_BUF_IS_UNLINKED(hdr)	(!(RVF_BUF_IS_LINKED(hdr)) )
#define RVF_SET_BUF_UNLINKED(hdr)	( (hdr)->buf_size &= ~0x00000001)


/* macros to set and check the usage of the previous buffer */
#define RVF_IS_PREV_IN_USE(hdr) 	(0x00000002 & hdr->buf_size)
#define RVF_SET_PREV_IN_USE(hdr)	( (hdr)->buf_size |= 0x00000002)

#define RVF_IS_PREV_FREE(hdr)		(!(RVF_IS_PREV_IN_USE(hdr)) )
#define RVF_SET_PREV_FREE(hdr)		( (hdr)->buf_size &= ~0x00000002)


#define SETSIZE(hdr, size)			{	(hdr)->buf_size &= 0x03;\
										(hdr)->buf_size |= (size); }
									


#define NEXTCHUNK(hdr)	( (T_RVF_INTERNAL_BUF *)( (UINT8*)hdr + GETSIZE(hdr) + sizeof(T_RVF_INTERNAL_BUF) ) )

#define ENDSIZE(hdr)	( ((T_RVF_INTERNAL_BUF*)( (UINT8*)hdr + GETSIZE(hdr) + sizeof(T_RVF_INTERNAL_BUF) - sizeof(UINT32) ))->buf_size )


#define RVF_NB_FREE_LISTS	32
/* macro used to get the list index from the buffer size */
/* 32 lists :	8  lists for buffer < 256  all spaced 32 bytes apart,
				8  lists for buffer < 1280 all spaced 128 bytes apart,
				8  lists for buffer < 5376 all spaced 512 bytes apart, 
				8  lists for buffer > 5376 all spaced 16384 bytes apart.*/

#define RVF_BUF_LIST_INDEX(size)	( ((size) < 256 ) ? ( (UINT8)((size)>>5) ) :\
									( ((size) < 1280 ) ? ((UINT8)(8 + ((size-256)>>7) ) ) :\
									( ((size) < 5376) ? ((UINT8)(16 + ((size-1280)>>9) ) ) :\
									( ((size) < 136448)? ((UINT8)(24 + ((size-5376)>>14) ) ): (UINT8)(31) ) ) ) )

/* internal buffer structure */
typedef struct _t_internal_buf
{	UINT32	buf_size;					/* size of the user buffer */
	struct	_t_internal_buf * p_next;	/* pointer to the next buffer in the queue */ 
	
	union header
	{	struct external
		{	UINT16	mb_id;						/* id of the memory bank which owns the buffer */
			UINT16	mb_expected;				/* id of the memory bank on which the buffer want to be counted */
		}external;
		
		struct	_t_internal_buf * p_prev;		/* pointer to the previous buffer in the queue */ 
	}header;
	
	
} T_RVF_INTERNAL_BUF;





/* RVF will managed at most 2 pools of memory for dynamic allocation */
/* buffer pool structure*/
typedef struct _t_rvf_pool
{
	void * start_address;			/* address of the beginnig of the pool */
	UINT32 pool_size;				/* total size of the pool */
} T_RVF_POOL;




/* memory bank structure */
typedef struct _t_rvf_mb
{	UINT32	cur_memory_used;		/* size of current memory usage */
	UINT32	watermark;				/* watermark */
	UINT32	max;					/* max size */
	MB_CALLBACK_FUNC func;			/* function to call when mb_state switch to GREEN */
	BOOLEAN	returned_red;			/* flag indicating that this memory bank returned a RED value, */
									/* its callback function has to be called and/or buffer are waiting */	
	UINT16	first_buffer_index;		/* index of the first waiting buffer in the array */
	UINT16	last_buffer_index;		/* index of the last waiting buffer in the array */
#if RVF_ENABLE_STATS
	UINT32	max_reached;			/* maximum memory usage reached */
	UINT32	required_size;			/* total size in byte required by the rvf_get_buf function */
	UINT32	num_buf;				/* total number of buffer allocated by rvf_get_buf function */
#endif

} T_RVF_MB;


/* structure which associates mb name and mb id */
typedef struct _t_rvf_mb_name_id
{	char mb_name[RVF_MAX_MB_LEN];	/* name of the memory bank */
	UINT16 mb_id;					/* id of the memory bank */
	T_RVF_MB_PARAM mb_params;		/* parameters of the memory bank */
} T_RVF_MB_NAME_ID;


/* note:
 * - hosting_list overhead is reduced with an 8 bit addr id
 * - is hosting_list needed? would known_swe struct be enough to derive all info?  */
typedef struct _rvf_rt_addr_id_data {					/* A-M-E-N-D-E-D!  */
	UINT8				type_code;						/* poss. derived?  */
	UINT8				priority;
	T_RVF_G_ADDR_ID		host_addr_id;					/* Poss. union: JavaRef-32b or addrId-16 */
	UINT8				hosting_count;
	T_RVF_G_ADDR_ID		parasites[MAX_PARASITES];
	UINT8				swe_db_index;
	char*				symbolic_name;
	NU_TASK*			pOSTCB;
	NU_EVENT_GROUP*		pOSEvtGrp;
	UINT8*				p_os_stack;
	UINT16				os_stack_size;
	T_RVF_G_ADDR_ID		virtualContext;
	UINT8				gdHost;
	T_RVF_INTERNAL_BUF* OSTaskQFirst[RVF_NUM_TASK_MBOX]; 
	T_RVF_INTERNAL_BUF*	OSTaskQLast [RVF_NUM_TASK_MBOX];
	T_RV_RET			(* handle_message)	(T_RV_HDR * msg);  // note: T_RV_RETURN and not T_RVM...
	T_RV_RET			(* handle_timer)	(T_RV_HDR * msg);
} T_RVF_RT_ADDR_ID_DATA;

typedef struct _rvf_tm_attrib {					
	T_RVF_G_ADDR_ID		host_addr_id;		
	UINT8				legacyFlag;
	void*				action;
} T_RVF_TM_ATTRIB;

typedef NU_TIMER T_RV_TM;

typedef union _rvf_tm_ublk {					
	T_RV_TM*		ptr;		
	UINT32			id;
	
} T_RVF_TM_UBLK;

typedef union _rvf_tm_attib {
	char				str[8];
	T_RVF_TM_ATTRIB		attrib;
} T_RVF_TM_ATTRIB_UBLK;

typedef union _rvf_tm_action {
	UINT32				action_id;
	void*				p_action;
} T_RVF_TM_ACTION_UBLK;

#ifdef __cplusplus
extern "C" {
#endif

T_RVF_RET	rvf_send_priority_msg	(T_RVF_G_ADDR_ID addr_id, void *msg) ;
T_RVF_RET	rvf_adapt_send_msg (T_RVF_G_ADDR_ID addr_id, void *msg, UINT8 mbox) ;

void		rvf_yield();

void		rvf_mbox_buffer_init(T_RVF_RT_ADDR_ID_DATA* pRtAddrIdElement);
void		_rvf_buffer_init (void);
void		_rvf_timers_init(void);
T_RVF_RET	rvf_get_available_mem( UINT32 * total_size, UINT32 * used_size );
T_RV_RET	_rvf_empty_mailboxes (T_RVF_G_ADDR_ID task_id);

void		_rvf_init_mem_pool(void);
void		_rvf_init_free_queue (UINT8 id, UINT32 size, void *p_mem);
UINT16		_rvf_get_mem_usage_ratio(void);
UINT16		_rvf_get_number_of_pool(void);
#ifdef		_WINDOWS
  void		_rvf_window_dump_mem(void *m);
#endif


T_RVF_RET		rvf_free_sys_resources(T_RVF_G_ADDR_ID gid, UINT8 rm);
T_RVF_G_ADDR_ID	rvf_allocate_task_id(UINT8 isRealTask) ;   /* return should be changed to 16 or 32 bit val */
T_RVF_RET		rvf_setRtAddrSweIndex(T_RVF_G_ADDR_ID id, UINT8 sweIndex);
T_RVF_G_ADDR_ID	resolveHostAddrId(T_RVF_G_ADDR_ID id) ;
T_RVF_RET		rvf_create_virtual_task(T_RV_RET (* handle_message)(T_RV_HDR * msg),
										T_RV_RET (* handle_timer)(T_RV_HDR * msg),
										T_RVF_G_ADDR_ID task_id, T_RVF_G_ADDR_ID host_task_id, char *taskname, UINT8 priority, UINT8 tcode) ;
T_RVF_RET		rvf_register_t3_handlers (T_RVF_G_ADDR_ID task_id,
								   T_RV_RET (* handle_message)(T_RV_HDR * msg),
								   T_RV_RET (* handle_timer)(T_RV_HDR * msg) ) ;
T_RVF_RET		rvf_create_host_task (T_RV_RET (* proxy)(void), T_RVF_G_ADDR_ID task_id, char *taskname, UINT8 *stack, UINT16 stacksize,\
										 UINT8 priority, UINT8 tcode, UINT8 time_slicing, T_RVF_TASK_STATE suspend);
T_RVF_RET		rvf_registerToHost(T_RVF_G_ADDR_ID host_id, T_RVF_G_ADDR_ID eid) ;
T_RVF_RET		rvf_unregisterFromHost(T_RVF_G_ADDR_ID host_id, T_RVF_G_ADDR_ID pid) ;
T_RVF_RET		rvf_setHostTaskStackPtr(T_RVF_G_ADDR_ID id, UINT8* pStack) ;

T_RVF_G_ADDR_ID rvf_resolveHostingAddrId(T_RVM_GROUP_DIRECTIVE gd);
T_RVF_RET		rvf_associateGrpToHost(T_RVF_G_ADDR_ID host_id, T_RVF_GD_ID gd_id);
T_RVF_RET		rvf_isHostingTaskIdle(T_RVF_G_ADDR_ID id, UINT8* status);
void			rvf_setRDV(T_RVF_G_ADDR_ID tid,T_RVF_G_ADDR_ID vid);

/* Internal RVF data structures*/

extern T_RVF_INTERNAL_BUF *OSTaskQFirst[1][1]; //MAX_RVF_TASKS][RVF_NUM_TASK_MBOX];
extern T_RVF_INTERNAL_BUF *OSTaskQLast[1][1]; //[MAX_RVF_TASKS][RVF_NUM_TASK_MBOX];

extern T_RVF_RT_ADDR_ID_DATA* pRtAddrIdTable[MAX_RVF_G_ADDR_ID];



#ifdef __cplusplus
}
#endif

#endif /* _RVF_I_H */

/****************************************************************************/
/*                                                                          */
/*  Name        rvf_api.h                                                       */
/*                                                                          */
/*  Function    this file contains rvf public definitions.                  */
/*                                                                          */
/*  Version		0.1															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  3/12/99		Create														*/
/*	11/15/99	remove useless functions,									*/
/*				change tasks priority and time_slicing						*/
/*	11/15/1999	compliant to RV coding guidelines							*/
/*  12/23/1999	change buffer management, add memory bank handling			*/
/*																			*/
/*	Author		David Lamy-Charrier (dlamy@tif.ti.com)						*/
/*																			*/
/* (C) Copyright 1999 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#ifndef _RVF_API_H
#define _RVF_API_H

#include "../rv/rv_general.h"
#include "rvf_target.h"

#if 0 //#ifndef _WINDOWS
  #include "config/swconfig.cfg"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* macros used to trace messages */

// WCP patch: default config is no RV trace for WCP
#if (OP_WCP == 0)
  #define DEBUG_ENABLE 1
#endif


#ifndef DEBUG_ENABLE 
#define rvf_send_trace 
#else 
void rvf_send_trace1(char * msg, UINT8 msg_length, UINT32 val, UINT8 trace_level, UINT32 swe_use_id);
#define rvf_send_trace rvf_send_trace1
#endif 	

#define T_RVF_GD_ID UINT8

/* return parameters definition */
typedef T_RV_RET T_RVF_RET;

#define	RVF_OK			RV_OK				
#define	RVF_NOT_SUPPORTED	RV_NOT_SUPPORTED		
#define	RVF_NOT_READY		RV_NOT_READY			
#define	RVF_MEMORY_ERR		RV_MEMORY_ERR			
#define	RVF_INTERNAL_ERR	RV_INTERNAL_ERR		
#define	RVF_INVALID_PARAMETER	RV_INVALID_PARAMETER	

/* memeory bank state definition */
typedef enum{
	RVF_GREEN,
	RVF_YELLOW,
	RVF_RED
}T_RVF_MB_STATUS;

/* task state */
typedef enum { SUSPEND, RUNNING} T_RVF_TASK_STATE;

/************************************************************************
** Mailbox definitions. Each task has 4 mailboxes that are used to
** send buffers to the task.
*/
#define RVF_TASK_MBOX_0    0
#define RVF_TASK_MBOX_1    1
#define RVF_TASK_MBOX_2    2
#define RVF_TASK_MBOX_3    3

#define RVF_NUM_TASK_MBOX  4

#define RVF_TASK_MBOX_0_EVT_MASK   0x0001
#define RVF_TASK_MBOX_1_EVT_MASK   0x0002
#define RVF_TASK_MBOX_2_EVT_MASK   0x0004
#define RVF_TASK_MBOX_3_EVT_MASK   0x0008

/************************************************************************
** Event definitions.
**
** There are 4 reserved events used to signal messages rcvd in task mailboxes.
** There are 4 reserved events used to signal timeout events.
** There are 8 general purpose events available for applications.
*/
#define RVF_MAX_EVENTS	          16

#define RVF_NUM_TASK_TIMERS     4

#define RVF_TIMER_0             0
#define RVF_TIMER_1             1
#define RVF_TIMER_2             2
#define RVF_TIMER_3             3

#define RVF_TIMER_0_EVT_MASK    0x0010
#define RVF_TIMER_1_EVT_MASK    0x0020
#define RVF_TIMER_2_EVT_MASK    0x0040
#define RVF_TIMER_3_EVT_MASK    0x0080

#define RVF_APPL_EVT_0          8
#define RVF_APPL_EVT_1          9
#define RVF_APPL_EVT_2          10
#define RVF_APPL_EVT_3          11
#define RVF_APPL_EVT_4          12
#define RVF_APPL_EVT_5          13
#define RVF_APPL_EVT_6          14
#define RVF_APPL_EVT_7          15

#define EVENT_MASK(evt)	   ((UINT16)0x0001 << evt)

#define MAX_HOSTING_TASKS	10
#define MAX_PARASITES		10


/* define memory bank related types */
typedef UINT16 T_RVF_MB_ID;
//typedef UINT8 T_RVF_MB_NAME[RVF_MAX_MB_LEN];
typedef char T_RVF_MB_NAME[RVF_MAX_MB_LEN];
#define T_RVF_BUFFER void 
typedef void (*CALLBACK_FUNC)(void *);
typedef void (*MB_CALLBACK_FUNC)(T_RVF_MB_ID);

typedef struct{
	UINT32 size;
	UINT32 watermark;
} T_RVF_MB_PARAM;



/* Define a timer list entry
*/
typedef struct _tle
{
	T_RV_HDR	  hdr;
	struct _tle  *p_next;
	struct _tle  *p_prev;
	UINT32        ticks;
	UINT16        event;			/* event & param must be revised possibily       */
	UINT32        param;			/* sub struct pointed to by p_data if required   */
	UINT32		  t_init;
	void*		  p_data;
} T_RVF_TIMER_LIST_ENT;

typedef T_RVF_TIMER_LIST_ENT*	P_NODE;

/* Define a timer list queue          ?? QUEUE or LIST ??
*/
typedef struct 
{
    T_RVF_TIMER_LIST_ENT   *p_first;
    T_RVF_TIMER_LIST_ENT   *p_last;
    UINT32           last_ticks;
	UINT16			 timerCnt;
} T_RVF_TIMER_LIST_Q;


/***********************************************************************
** This queue is a general purpose buffer queue, for application use.
*/
typedef struct 
{   void    *p_first;
    void    *p_last;
    UINT16  count;
} T_RVF_BUFFER_Q;

#define RVF_IS_QUEUE_EMPTY(p_q) (p_q.count == 0)

/*******************************************************
* Message parameter of "handle_timer()"  
********************************************************/
#define T_RVF_TIMER_ID		UINT32 

typedef struct {
	T_RV_HDR				hdr;
	T_RVF_TIMER_ID			tm_id;
	UINT8					cont;
	void*					action;
} T_RVF_TMS_MSG;

/* define a mutex structure */
typedef struct
{	UINT32	words[11];
}T_RVF_MUTEX;

/***********************************************************************
** Define the basic message header that RVF expects on all inter-computer
** communications. Applications should use this in their own definitions
** and add on their own message contents.
*/
/*typedef struct {
	UINT16  prim;  // Message Type 
	UINT16  len;   // Message Length (including header) 
} msg_hdr_t;

typedef struct {
	msg_hdr_t  hdr;
	UINT8      data[1];
} msg_type1;
*/

/* Task constants
*/
#ifndef TASKPTR
typedef void (*TASKPTR)(UINT32);
#endif


#define RVF_INVALID_TASK (0xFF)
#define RVF_INVALID_MB_ID		(0xFFFF)
#define RVF_INVALID_MAILBOX (0xFF)
#define RVF_INVALID_ADDR_ID	(0xFF)

/* Define the value that create pool will return if it fails
*/


/************************************************************************/
/* Function prototypes													*/


/* Task management*/
void			rvf_init(void);
T_RVF_RET		rvf_create_legacy_task (TASKPTR task_entry, UINT8 task_id, char *taskname, UINT8 *stack, UINT16 stacksize, UINT8 priority, UINT8 time_slicing, T_RVF_TASK_STATE is_suspend) ;
T_RVF_RET		rvf_create_task (TASKPTR task_entry, T_RVF_G_ADDR_ID task_id, char *taskname, UINT8 *stack, UINT16 stacksize, UINT8 priority, UINT8 tcode, UINT8 time_slicing, T_RVF_TASK_STATE is_suspend);
T_RVF_RET		rvf_resume_task( T_RVF_G_ADDR_ID taskid);
void			rvf_exit_task(T_RVF_G_ADDR_ID vtask);
T_RVF_RET		rvf_suspend_task(T_RVF_G_ADDR_ID vtask);
T_RVF_G_ADDR_ID	rvf_get_taskid(void);
char *			rvf_get_taskname(void);
T_RVF_G_ADDR_ID rvf_get_context();

/* Message Handling */
typedef T_RV_HDR T_RVF_MSG ;
typedef UINT32	  T_RVF_MSG_ID;

T_RVF_RET		rvf_send_msg			(T_RVF_G_ADDR_ID addr_id, void * p_msg);
T_RVF_MB_STATUS rvf_get_msg_buf			(T_RVF_MB_ID mb_id,
											UINT32 message_size,
											T_RVF_MSG_ID msg_id,
											T_RVF_MSG ** pp_msg);
T_RVF_RET		rvf_free_msg			(T_RVF_MSG * p_msg);
T_RVF_RET		rvf_free_timer_msg		(T_RVF_MSG *p_msg);



/* To send buffers and events between tasks*/
void *			rvf_read_mbox  (UINT8 mbox);
void *			rvf_read_addr_mbox (T_RVF_G_ADDR_ID  task_id, UINT8 mbox);
UINT8			rvf_send_event (T_RVF_G_ADDR_ID task_id, UINT16 event);

/* task synchronization */
T_RVF_RET		rvf_initialize_mutex( T_RVF_MUTEX * mutex);
T_RVF_RET		rvf_lock_mutex( T_RVF_MUTEX * mutex);
T_RVF_RET		rvf_unlock_mutex( T_RVF_MUTEX * mutex);
T_RVF_RET		rvf_delete_mutex( T_RVF_MUTEX * mutex);


/* To manage memory */
T_RVF_RET		rvf_create_mb(T_RVF_MB_NAME mb_name, T_RVF_MB_PARAM mb_param, T_RVF_MB_ID *mb_id);
T_RVF_RET		rvf_delete_mb(T_RVF_MB_NAME mb_name);
T_RVF_RET		rvf_get_mb_id(T_RVF_MB_NAME mb_name, T_RVF_MB_ID *mb_id);
T_RVF_MB_STATUS	rvf_get_mb_status(T_RVF_MB_ID mb_id);
T_RVF_RET		rvf_mb_is_used(T_RVF_MB_NAME mb_name, UINT8* isUsed) ;

T_RVF_MB_STATUS	rvf_get_buf(T_RVF_MB_ID mb_id, UINT32 buffer_size, T_RVF_BUFFER** p_buffer);
T_RVF_MB_STATUS	rvf_count_buf(T_RVF_MB_ID mb_id, T_RVF_BUFFER * p_buffer);
T_RVF_RET		rvf_free_buf( T_RVF_BUFFER * p_buffer);
T_RVF_RET		rvf_set_callback_func(T_RVF_MB_ID mb_id, MB_CALLBACK_FUNC func);
T_RVF_RET		rvf_change_callback_func(T_RVF_MB_ID mb_id, MB_CALLBACK_FUNC func);

void rvf_get_protected_buf(T_RVF_MB_ID mb_id, UINT32 buffer_size, T_RVF_BUFFER** p_buffer);
  

UINT32			rvf_get_buf_size (void *bptr);
				/* get the parameters of a specific memory bank */
T_RVF_RET		rvf_get_mb_param( T_RVF_MB_NAME mb_name, T_RVF_MB_PARAM * param);
				/* change the parameters of a specific memory bank */
T_RVF_RET		rvf_set_mb_param( T_RVF_MB_NAME mb_name, T_RVF_MB_PARAM * param);


/* User buffer queue management*/
T_RVF_RET		rvf_enqueue (T_RVF_BUFFER_Q *p_q, void *p_buf);
T_RVF_RET		rvf_enqueue_head (T_RVF_BUFFER_Q *p_q, void *p_buf);
void *			rvf_dequeue  (T_RVF_BUFFER_Q *p_q);
T_RVF_BUFFER *	rvf_scan_next (T_RVF_BUFFER_Q * p_q, T_RVF_BUFFER * p_buf);
T_RVF_RET		rvf_remove_from_queue (T_RVF_BUFFER_Q * p_q, T_RVF_BUFFER * p_buf);

/* Timer management*/

UINT16			rvf_wait(UINT16 flag, UINT32 ticks);
UINT16			rvf_evt_wait(T_RVF_G_ADDR_ID rtask, UINT16 flag, UINT32 timeout) ;
void			rvf_delay(UINT32 ticks);

void			rvf_start_timer(UINT8 tnum, UINT32 ticks, BOOLEAN is_continuous);
void			rvf_stop_timer (UINT8 tnum);
char *			rvf_get_time_stamp(char *tbuf);
UINT32			rvf_get_tick_count(void);
void			rvf_init_timer_list (T_RVF_TIMER_LIST_Q *p_timer_listq);
void			rvf_init_timer_list_entry (T_RVF_TIMER_LIST_ENT  *p_tle);
UINT16			rvf_update_timer_list (T_RVF_TIMER_LIST_Q *p_timer_listq);
void			rvf_add_to_timer_list (T_RVF_TIMER_LIST_Q *p_timer_listq, T_RVF_TIMER_LIST_ENT  *p_tle);
void			rvf_remove_from_timer_list (T_RVF_TIMER_LIST_Q *p_timer_listq, T_RVF_TIMER_LIST_ENT  *p_tle);
T_RVF_TIMER_LIST_ENT*	rvf_get_expired_entry (T_RVF_TIMER_LIST_Q *p_timer_listq);


#define T_RV_TM_ID	UINT32

/*************************************************************************/
/* :)  USER FRIENDLY TIMER MANAGEMENT API consisting of add, delete and modify timer */
/*************************************************************************/
/* Returns 0 on failure */
T_RVF_TIMER_ID	rvf_create_timer(	T_RVF_G_ADDR_ID			g_addrId,
									UINT32					timerDuration,
									BOOLEAN					isContinuous,
									void*					p_action);

void			rvf_del_timer	(	T_RV_TM_ID tm_id) ;

void			rvf_reset_timer	(	T_RV_TM_ID tm_id, UINT32 new_duration,
									BOOLEAN isContinuous);


/* Disable Interrupts, Enable Interrupts*/
void			rvf_enable(void);
void			rvf_disable(UINT8 who);

/* Trace for debug purposes*/
void rvf_dump_mem();
void rvf_dump_tasks();
void rvf_dump_pool();

/* specific function */
T_RVF_BUFFER * rvf_wait_for_specific_msg(UINT16 msg_code, UINT8 mbox, UINT32 timeout);

/* Trace definitions */
#define NULL_PARAM 0xFFFFFFFFU /* this value will not be displayed in rvf_send_trace function */
#define TRACE_MB_ID		(RVF_MAX_REAL_MB -1)

#ifdef __cplusplus
}
#endif

#endif	/* _RVF_API_H */

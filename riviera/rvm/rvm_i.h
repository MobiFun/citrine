
/****************************************************************************/
/*                                                                          */
/*  Name        rvm_i.h														*/
/*                                                                          */
/*  Function    this file contains the definitions of internal data			*/
/*				structures for the RVM										*/
/*																			*/
/*                                                                          */
/*  Version		0.1															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  20/01/2000	Create														*/
/*																			*/
/*	Author		David Lamy-Charrier (dlamy@tif.ti.com)						*/
/*																			*/
/* (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#ifndef __RVM_I_H_
#define __RVM_I_H_

#if 0 //#ifndef _WINDOWS
  #include "config/rv.cfg"
  #include "config/debug.cfg"
#endif

#include "../rvf/rvf_api.h"
#include "rvm_api.h"
#include "rvm_gen.h"
//#include "rvf/rvf_pool_size.h"
#include "rvm_pool_size.h"
//#include "rv_general.h"  //??


/* MSG MUST BE VERIFIED */
#define RVM_MSG_OFFSET		BUILD_MESSAGE_OFFSET(RVM_USE_ID)

#define RVM_START_T2_MSG			(RVM_MSG_OFFSET	| 0x001)
#define RVM_RT_MOD_YIELD_T2_MSG		(RVM_MSG_OFFSET	| 0x002)
#define RVM_STOP_MSG				(RVM_MSG_OFFSET	| 0x003)
#define RVM_STOP_REQ_MSG			(RVM_MSG_OFFSET	| 0x004)
		

#define RVM_YIELD_T2_PS_TM			10
#define RVM_YIELD_T2_PRI_MSG_CNT	10
#define T_RVM_TASK_ID				T_RVF_G_ADDR_ID  //UINT8
#define SYSTEM_TASK_MEM				204

#define MAX_GRPS		MAX_HOSTING_TASKS
#define MAX_COMPOSITES	MAX_PARASITES

/* definitions related to the RVM as a RVF task */
#define RVM_EXPECTED_EVENT	RVF_TASK_MBOX_0_EVT_MASK
#define RVM_MAILBOX			RVF_TASK_MBOX_0

#if (defined BLUETOOTH) || (TEST==1)
	#define RVM_MAX_SWE_USING	(5)		/* max number of SW Entities using another one within a given appli */
#else
	#define RVM_MAX_SWE_USING	(3)		/* max number of SW Entities using another one within a given appli */
#endif


#define RVM_INVALID_SWE_INDEX		(0xff)

extern T_RVF_MB_ID rvm_mem_bank;
extern T_RVF_MB_ID rvm_sys_mem_bank;
extern T_RVF_MB_ID rvm_timer_mem_bank;
extern T_RVF_MB_ID rvm_tm_notify_mem_bank;
extern T_RVF_MB_ID rvm_stack_mem_bank;

/* parameters of mb in use */
typedef struct
{	T_RVF_MB_NAME		mb_name;
	T_RVF_MB_PARAM		mb_initial_param;
}T_RVM_APPLI_MB_PARAMS;


#define RVM_INVALID_GROUP_INDEX	(0xFFFF)
#define RVM_OWN_GROUP			(0xFFFE)


/* list of possible event in the RVM */
//#define		RVM_START_APPLI	(60000)
//#define		RVM_STOP_APPLI	(60001)
//#define		RVM_ERROR		(60002)


typedef enum {
    SWE_NOT_STARTED			= 0,
    SWE_RUNNING				= 1,
    SWE_STOPPING			= 2,
    SWE_KILLED				= 3
}T_RVM_SWE_STATE;


/* structure of message  */
typedef struct {
	T_RV_HDR		header;
	T_RV_RETURN_PATH	rp;
	UINT8			swe_num;
}T_RVM_MSG;


/* Information about each known SWE
   This is static information and is built at RVM initialization */
typedef struct
{	
	T_RVM_NAME			swe_name;
	T_RVM_USE_ID			swe_use_id;
	T_RVM_GET_INFO_FUNC		swe_get_info;			/* pointer to its get_info function */
	T_RVM_SWE_TYPE			swe_type;				/* type of the SWE */
	T_RVF_G_ADDR_ID			swe_addr_id;			/* unique address id of the SWE */
	UINT16				group_index;			/* for type 2 SWE, index of the hosting group in the group array. */
	UINT16				swe_stack_size;			/* size of the task stack */
	UINT8				swe_priority;			/* priority of the SWE */
	T_RVM_SWE_STATE			swe_state;				/* state of the SWE */
	T_RVF_BUFFER*			stack_ptr;			/* pointer to the stack of the task */
	UINT8				nb_using_appli;			/* nb of application which uses this SWE */
	UINT8				using_appli[RVM_MAX_SWE_USING]; /* pointer to each application which use this SWE */
	T_RV_RETURN_PATH		swe_return_path;
	T_RV_RETURN_PATH		mmi_return_path;
}T_RVM_KNOWN_SWE;

typedef struct
{	T_RVM_SWE_STATE		host_state;
	UINT8			task_id;
	UINT16			nb_hosted_swe;
	UINT16			stack_size;
	UINT8			task_priority;
}	T_RVM_GROUP_INFO; /* DEPRECATED! */


/* structure containing the SWE generic functions pointers */
typedef struct
{	
T_RVM_RETURN  (* set_info)	(	T_RVF_ADDR_ID	addr_id,
								T_RV_RETURN_PATH return_path[],
								T_RVF_MB_ID		bk_id_table[],
								T_RVM_CB_FUNC	call_back_error_ft);
T_RVM_RETURN  (* init)  (void);
T_RVM_RETURN  (* start) (void);
T_RVM_RETURN  (* handle_message)	(T_RV_HDR * msg);
T_RVM_RETURN  (* handle_timer)		(T_RV_HDR * msg);
T_RVM_RETURN  (* core)	(void);
T_RVM_RETURN  (* stop)	(T_RV_HDR* hdr);
T_RVM_RETURN  (* stop1) (void);
T_RVM_RETURN  (* kill)	(void);
}	T_RVM_SWE_FUNCTIONS;

/*	Information about each processing SWE
	This information is dynamic and lives during start or stop process only*/
typedef struct _t_rvm_processing_swe
{
	UINT8				swe_id; // From known SWE array index
	T_RVM_SWE_TYPE			swe_type;
	UINT16				stack_size;
	UINT8				priority;
	UINT8				nb_requested_mb;
	UINT8				nb_created_mb;
	T_RVM_APPLI_MB_PARAMS		swe_mem_bank[RVM_MAX_NB_MEM_BK];
	T_RVM_SWE_FUNCTIONS		rvm_functions;
	T_RVF_MB_ID			bk_id_table[RVM_MAX_NB_MEM_BK];
	struct _t_rvm_processing_swe	*next_swe;
} T_RVM_PROCESSING_SWE;

typedef struct {
	T_RV_HDR	hdr;
	UINT8		grp[MAX_PARASITES];					// to do ..... constant!
} T_RVM_START_T2_MSG;

typedef struct {
	T_RV_HDR		header;
	UINT8			swe_num;
	T_RV_RETURN_PATH	rp;
	T_RVM_SWE_STATE status;
} T_RVM_STOP_MSG;

/*typedef struct {
	T_RV_HDR		header;
	UINT8			swe_num;
//	T_RVF_G_ADDR_ID	target;
//	UINT8			attempts;
//	T_RVM_SWE_STATE status;
} T_RVM_STOP_REQ_MSG; */

typedef struct {
	T_RV_HDR	hdr;
	UINT16		val;					
} T_RVM_RT_MOD_YIELD_T2_MSG;


/* information about groups of type2 SWE */
#define	T_GD_ID	UINT8

typedef struct {
	T_RVM_SWE_STATE		host_state;
	T_RVF_GD_ID		group_directive;
	T_RVF_G_ADDR_ID		host_task_addr;
//	UINT16			hosted_swe_addr[10];
	UINT8			hosted_swe_db_index[MAX_PARASITES];  // to do ..... constant!
	UINT16			stack_size;
	UINT8			task_priority;
}	T_RVM_GROUP_DIRECTIVE;

T_RV_RET rvm_t2_proxy(void);
T_RV_RET rvm_t3_proxy(void);
//typedef TASKPTR rvm_proxy;

/* definitions related to the RVM as a RVF task */
#define RVM_TASK_ID		(0)
#define RVM_PRIORITY		(220)

/* To do ... should correspond to RVF definitions   */
#define RVM_TASK		0			/* A-M-E-N-D-E-D !	*/
#define ET2_HOST_TASK		1
#define ET3_TASK		3
#define ET4_TASK		4
#define ET2_VTASK		5
#define PRE_RV_INIT_TASK	10


#define RVM_ERROR_MSG_MAX_LENGTH	(100)


#define DEFAULT_TIME_SLICING	0	/* default is no time-slicing */

#define RVM_STACK_SIZE			RVF_RVM_STACK_SIZE
/*#define RVM_STACK_SIZE			(1500) */

/* memory bank related definitions */
#define RVM_PRIM_MB				"RVM_PRIM"
#define RVM_PRIM_MB_SIZE		 RVF_RVM_MB1_SIZE  //A-M-E-N-D-E-D
#define RVM_SYS_MB				"RVM_SYS"
#define RVM_SYS_MB_SIZE			 RVF_RVM_MB3_SIZE
#define RVM_TIMER_MB			"RVM_TIMER"
#define RVM_TIMER_MB_SIZE		 RVF_RVM_MB4_SIZE
#define RVM_NOTIFY_MB			"RVM_NOTIFY"
#define RVM_NOTIFY_MB_SIZE		 RVF_RVM_MB5_SIZE

#define RVM_TIMER_MB_WATERMARK	(RVM_TIMER_MB_SIZE - 500)
#define RVM_NOTIFY_MB_WATERMARK	(RVM_NOTIFY_MB_SIZE - 500)

#if (defined BLUETOOTH) || (TEST==1) || (TI_PROFILER == 1)	|| (TI_NUC_MONITOR == 1)
 		#define RVM_PRIM_MB_WATERMARK	(RVM_PRIM_MB_SIZE - 1000)
		#define RVM_SYS_MB_WATERMARK	(RVM_SYS_MB_SIZE - 1000)
#else
 		#define RVM_PRIM_MB_WATERMARK	(RVM_PRIM_MB_SIZE - 100)
		#define RVM_SYS_MB_WATERMARK	(RVM_SYS_MB_SIZE - 100)
#endif
  
#define RVM_STACK_MB			"RVM_STACKS"
#define RVM_STACK_MB_SIZE		(RVM_STACK_SIZE + RVF_RVM_MB2_SIZE)
#define RVM_STACK_MB_WATERMARK	RVM_STACK_MB_SIZE



/***************************************/
/* RVM Internal functions declarations */
/***************************************/
/* function called from rvm_task.c in order to check if an application is startable or stoppable*/


/* functions declarations */
T_RVM_RETURN	rvm_check_application	(T_RVM_USE_ID swe_use_id, UINT8* num_swe, T_RVM_APPLI_ACTION appli_action);

T_RVM_RETURN	rvm_error ( T_RVM_NAME swe_name, T_RVM_RETURN error_cause, 
							T_RVM_ERROR_TYPE error_type, T_RVM_STRING error_msg);


void		rvm_task (UINT32 param);

void		rvm_launch_appli( T_RVM_MSG * msg_Ptr);

void		rvm_stop_appli( T_RVM_STOP_MSG * msg_Ptr);

void		rvm_swe_has_stopped(T_RVM_STOP_MSG* p_msg);
T_RVM_RETURN	rvm_dup_stop_msg(T_RVM_STOP_MSG* i_msg, T_RVM_STOP_MSG* o_msg) ;
void		rvm_suspend_swe(UINT8 swe_index);
T_RVM_RETURN	rvm_kill_swe(UINT8 swe_index);
UINT8		rvm_get_mb_level(UINT8 swe_index) ;

void		rvm_shut_down_appli( T_RVM_MSG * msg_Ptr);
void		rvm_shutdown_swe(UINT8 index) ;

T_RVM_TASK_ID rvm_allocate_task_id(UINT8 isRealTask);

T_RVM_RETURN rvm_delete_used_memory ( T_RVM_PROCESSING_SWE * appli);

T_RVM_RETURN rvm_build_swe_list(T_RVM_PROCESSING_SWE ** list, UINT8 swe_num, UINT8 mode);

T_RVM_RETURN rvm_check_memory_requirement( T_RVM_PROCESSING_SWE * appli); // deprected !
T_RVM_RETURN rvm_verify_memory_requirement( T_RVM_PROCESSING_SWE * appli,\
										    T_RVM_GROUP_DIRECTIVE* gd, UINT8 cnt);

T_RVM_RETURN rvm_allocate_mb( T_RVM_PROCESSING_SWE * appli);

T_RVM_RETURN rvm_initialize_swe( T_RVM_PROCESSING_SWE * appli,
										T_RVM_GROUP_DIRECTIVE* gd, 
										UINT8 cnt);

T_RVM_RETURN rvm_allocate_stack_buffer( UINT32 stack_size, T_RVF_BUFFER** stack_ptr );

T_RVM_RETURN rvm_delete_created_mb (T_RVM_PROCESSING_SWE * appli);

T_RVM_RETURN rvm_clean_env( T_RVM_PROCESSING_SWE * appli);

T_RVM_RETURN rvm_generic_swe_core(void);

T_RVM_RETURN	rvm_init_swe_db (void);

T_RVM_RETURN	rvm_snd_msg_to_upper	(T_RVM_APPLI_ACTION action,
										 T_RVM_RETURN result,
										 UINT8 swe_num,
										 T_RV_RETURN_PATH return_path);

T_RVM_RETURN	rvm_get_swe_index( UINT8 * index, T_RVM_USE_ID swe_use_id);

T_RVM_RETURN	rvm_start_group_req(T_RVF_G_ADDR_ID addrId, UINT8* addrGrp) ;

/*
** Trace Macros
*/
#define RVM_TRACE_ERROR(string)\
	rvf_send_trace (string,(sizeof(string)-1),NULL_PARAM,RV_TRACE_LEVEL_ERROR,RVM_USE_ID )

#define RVM_TRACE_ERROR_PARAM(string, param)\
	rvf_send_trace (string,(sizeof(string)-1),(UINT32)param,RV_TRACE_LEVEL_ERROR,RVM_USE_ID )

#define RVM_TRACE_WARNING(string)\
	rvf_send_trace (string,(sizeof(string)-1),NULL_PARAM,RV_TRACE_LEVEL_WARNING,RVM_USE_ID )

#define RVM_TRACE_WARNING_PARAM(string, param)\
	rvf_send_trace (string,(sizeof(string)-1),(UINT32)param,RV_TRACE_LEVEL_WARNING,RVM_USE_ID )

#define RVM_TRACE_DEBUG_HIGH(string)\
	rvf_send_trace (string,(sizeof(string)-1),NULL_PARAM,RV_TRACE_LEVEL_DEBUG_HIGH,RVM_USE_ID )

#define RVM_TRACE_DEBUG_HIGH_PARAM(string, param)\
	rvf_send_trace (string,(sizeof(string)-1),(UINT32)param,RV_TRACE_LEVEL_DEBUG_HIGH,RVM_USE_ID )

#define RVM_TRACE_DEBUG_LOW(string)\
	rvf_send_trace (string,(sizeof(string)-1),NULL_PARAM,RV_TRACE_LEVEL_DEBUG_LOW,RVM_USE_ID )

#define RVM_TRACE_DEBUG_LOW_PARAM(string, param)\
	rvf_send_trace (string,(sizeof(string)-1),(UINT32)param,RV_TRACE_LEVEL_DEBUG_LOW,RVM_USE_ID )


#endif /* __RVM_I_H_ */

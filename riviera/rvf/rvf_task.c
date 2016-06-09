/****************************************************************************/
/*                                                                          */
/*  Name        rvf_task.c                                                  */
/*                                                                          */
/*  Function    this file contains \rvf task related functions               */
/*                                                                          */
/*  Version		0.1															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  3/12/99		Create														*/
/*	10/27/99	remove all non-nucleus sections (#ifdef)					*/
/*				change tasks priority and time_slicing						*/	
/*  11/17/1999	change RVF_create_task and RVF_get_taskid functions         */
/*	30/11/99	compliant to RV coding guidelines							*/
/*  28/08/2000	add mutex related functions.								*/
/*																			*/
/*	Author		David Lamy-Charrier (dlamy@tif.ti.com)						*/
/*																			*/
/* (C) Copyright 1999 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../include/config.h"

#include "../rv/rv_general.h"
#include "rvf_api.h"
#include "rvf_i.h"			
#include "../rvm/rvm_i.h"	 /* ONLY for Task Codes */
#include "../rvm/rvm_use_id_list.h"

#include "../../nucleus/nucleus.h"

#include <stdio.h>
#include <string.h>

/* include the rvtool_trace.h file only in the RivieraTool */
#ifdef	_WINDOWS
#ifndef	_CONSOLE
//		#include "rvtool_trace.h"
	#endif
	#include <windows.h>
#endif

#define RVF_STATIC_ALLOC_NB	1

/**********************************************************************
** Nucleus specific definitions
*/

typedef void (*NU_TASK_ENTRY)(UNSIGNED, VOID *);  //

/* array of tasks */
/**********************************************************************
* Note: if dynamic mem alloc of "pOSTCB" creates too much fragentation
* The origianl way me be utilised and pointed to by a addr_id table 
* structure, which is proposed for type 2 support.
***********************************************************************/

#define _RALLOC_TASK_CNTRL_BLK(tb)		rvf_get_buf(rvm_sys_mem_bank, sizeof(NU_TASK), tb)
#define	_RALLOC_TASK_EVT_GRP(tb)		rvf_get_buf(rvm_sys_mem_bank, sizeof(NU_EVENT_GROUP), tb)
#define _RALLOC_TASK_RT_ADDR_DATA(tb)	rvf_get_buf(rvm_sys_mem_bank, sizeof(T_RVF_RT_ADDR_ID_DATA), tb)
//#define _RALLOC_TASK_RT_TM(tb)			rvf_get_buf(rvm_sys_mem_bank, sizeof(T_RVF_TIMER_LIST_Q), tb)

/*static NU_TASK			bOSTCB[RVF_STATIC_ALLOC_NB];			 
static NU_EVENT_GROUP	bOSEvtGrp[RVF_STATIC_ALLOC_NB];			 
static UINT8			*bOSStack;
static UINT16			bOSStackSize;
static char				bOSTName[RVF_STATIC_ALLOC_NB][RVF_STATIC_ALLOC_NB];
*/

static UINT8	task_counter = 0;	
static INT16	OSDisableNesting = 0;
static BOOL		OSInterruptAlreadyMasked = FALSE;
static INT32    OSLastIntLevel;
static T_RVF_G_ADDR_ID _RDV=RVF_INVALID_ADDR_ID;

T_RVF_RET		_initSysRtData(void);


/*******************************************************************************
**
** Function         rvf_AllocTaskXyz()
**
** Description      
**                  
**
** Returns          T_RVF_MB_STATUS
**
*******************************************************************************/
T_RVF_RET _initSysRtData(void) {    /*A-M-E-N-D-E-D!*/
	int i=0;

	for (i=0; i < MAX_RVF_G_ADDR_ID; i++) pRtAddrIdTable[i]=NULL;

	//	pOSTCB[0]=&OSTCB[0];
	return RVF_OK;
}

/*******************************************************************************
**
** Function         rvf_init
**
** Description      This function is called once at startup to initialize
**                  all the rvf (timer, buffer...).
**
** Returns          void
**
*******************************************************************************/
void rvf_init(void) {
	/* UINT8   i;	*/

    /* Initialize RVF variables */
    /*for (i = 0; i < MAX_RVF_TASKS; i++) {
        OSStack[i] = 0;
        OSStackSize[i] = 0;
        OSTName[i][0] = 0;
		memset( &OSTCB[i],		0, sizeof(NU_TASK) );
		memset( &OSEvtGrp[i],	0, sizeof(NU_EVENT_GROUP) );
    }
	//	for (i = 0; i < 1; i++) memset( &OSTCB[i], 0, sizeof(NU_TASK) );
	*/
	_initSysRtData();
	_rvf_buffer_init();
	_rvf_timers_init();
}

/*******************************************************************************
**
** Function         _rvf_name_cpy
**
** Description      Internal function which copy a string in a buffer.
**                  The string may be null-terminated or length bytes long.
**
** Returns          void
**
*******************************************************************************/
void _rvf_name_cpy(char * dest, char * source, UINT8 length) {	
	UINT8 cpt;
	for( cpt = 0; cpt < length ; cpt++) {
		dest[cpt] = source[cpt];
		if (source[cpt] == 0) {	return;}
	}
}

T_RVF_G_ADDR_ID rvf_get_context() {
	T_RVF_G_ADDR_ID gid=rvf_get_taskid();
	if(pRtAddrIdTable[gid]->type_code==ET2_HOST_TASK) {
		return pRtAddrIdTable[gid]->virtualContext;
	} else {
		return pRtAddrIdTable[gid]->host_addr_id;
	}
}

void rvf_setRDV(T_RVF_G_ADDR_ID tid,T_RVF_G_ADDR_ID vid) { 	
	if(pRtAddrIdTable[tid])	pRtAddrIdTable[tid]->virtualContext=vid;
}


/**************************************************************
*	Function         _rvf_name_cpy
*
***************************************************************/
T_RVF_G_ADDR_ID rvf_allocate_task_id(UINT8 isRealTask) {
	T_RVF_G_ADDR_ID i=0;
	UINT8 isTask=1;

	/* Note: differentiation is made between REAL and VIRTUAL IDs
	 * to provide for backwards compatabible LEGACY timer implementations
	 * The Real task ids should be in sync with:
	 * "static rvf_Timer[MAX_RVF_TASKS][RVF_NUM_TASK_TIMERS]",
	 * defined in "rvf_time.c". In the future, one NU_Timer blk will
	 * be pointed to in the rt global addr table. Hence, eliminating 
	 * the large resource of "rvf_Timer[][]"
	 */
	if(!pRtAddrIdTable[i]) return RVF_INVALID_ADDR_ID;

	if(isRealTask) {
		/* REAL TASK									*/
		for(i=0; pRtAddrIdTable[i]!=NULL && i<MAX_RVF_TASKS; i++);

		if (i < MAX_RVF_TASKS)	{ /* alloc. and init. */ 
			if(_RALLOC_TASK_RT_ADDR_DATA((T_RVF_BUFFER**)&pRtAddrIdTable[i])) return RVF_INVALID_TASK;
			memset( pRtAddrIdTable[i], 0, sizeof(T_RVF_RT_ADDR_ID_DATA) );
			rvf_mbox_buffer_init(pRtAddrIdTable[i]);

		} else	return RVF_INVALID_ADDR_ID;
	} else {
		/* VIRTUAL TASK				*/
		for(i=MAX_RVF_TASKS; pRtAddrIdTable[i]!=NULL && i<MAX_RVF_G_ADDR_ID; i++);

		if (i < MAX_RVF_G_ADDR_ID)	{ /* alloc. and init. */ 
			if(_RALLOC_TASK_RT_ADDR_DATA((T_RVF_BUFFER**)&pRtAddrIdTable[i])) return RVF_INVALID_TASK;
			memset( pRtAddrIdTable[i], 0, sizeof(T_RVF_RT_ADDR_ID_DATA) );
			rvf_mbox_buffer_init(pRtAddrIdTable[i]);

		} else	return RVF_INVALID_ADDR_ID;
	}
	
	return  i;
}

/* MUST RESOLVE ERROR CODES '1' is just for P of C */
T_RVF_RET rvf_setRtAddrSweIndex(T_RVF_G_ADDR_ID id, UINT8 sweIndex) {
	if(id>=MAX_RVF_G_ADDR_ID) return 1;
	pRtAddrIdTable[id]->swe_db_index=sweIndex;
//	pRtAddrIdTable[id]->type_code=tcode; /* allows type to be set if not in swe */
	return RVF_OK;
}

T_RVF_RET rvf_setHostTaskStackPtr(T_RVF_G_ADDR_ID id, UINT8* pStack) { /* deprecated ! */
	if(id>=MAX_RVF_TASKS) return 1;
	pRtAddrIdTable[id]->p_os_stack=pStack;

	return RVF_OK;
}

T_RVF_RET rvf_isHostingTaskIdle(T_RVF_G_ADDR_ID id, UINT8* status) { /* deprecated ! */
	if(id>=MAX_RVF_TASKS)	return RVF_INVALID_PARAMETER;
	if(!pRtAddrIdTable[id])	return RVF_INVALID_PARAMETER;
	if(pRtAddrIdTable[id]->type_code!=ET2_HOST_TASK) return RVF_INVALID_PARAMETER;

	if(pRtAddrIdTable[id]->hosting_count==0) *status=1;
	return RVF_OK;
}

/* convenience/helper fnc. */
T_RVF_G_ADDR_ID resolveHostAddrId(T_RVF_G_ADDR_ID id) { 
	if(!pRtAddrIdTable[id]) return RVF_INVALID_ADDR_ID;
	return pRtAddrIdTable[id]->host_addr_id;
}

/* HostingCounter enables one to deduce if task can be terminated	*/
/* ERROR return val must be revised									*/
T_RVF_RET rvf_registerToHost(T_RVF_G_ADDR_ID host_id, T_RVF_G_ADDR_ID eid) {
	UINT8 i;

	if (host_id >= MAX_RVF_TASKS || eid >= MAX_RVF_G_ADDR_ID )  return RV_INVALID_PARAMETER;; 

	for(i=0;pRtAddrIdTable[host_id]->parasites[i]!=0 && i < MAX_PARASITES; i++);

		pRtAddrIdTable[host_id]->parasites[i]=eid;
		pRtAddrIdTable[host_id]->hosting_count++;

	return RVF_OK;
}

T_RVF_RET rvf_unregisterFromHost(T_RVF_G_ADDR_ID host_id, T_RVF_G_ADDR_ID pid) {
	UINT8 i=0;
	if (pRtAddrIdTable[host_id]->hosting_count !=0) {
		for(i=0; i<MAX_PARASITES || pRtAddrIdTable[host_id]->parasites[i]==pid; i++);
			pRtAddrIdTable[host_id]->parasites[i]=0;

		pRtAddrIdTable[host_id]->hosting_count--;
	} else return RV_INVALID_PARAMETER;

	return RVF_OK;
}

T_RVF_RET rvf_associateGrpToHost(T_RVF_G_ADDR_ID host_id, T_RVF_GD_ID gd_id) {

	if (host_id >= MAX_RVF_TASKS )  return RV_INVALID_PARAMETER; 

	pRtAddrIdTable[host_id]->gdHost=gd_id;
		
	return RVF_OK;
}

/*T_RVF_RET rvf_unregisterGrpFromHost(T_RVF_G_ADDR_ID host_id, T_RVF_G_ADDR_ID pid) {
	if (pRtAddrIdTable[host_id]->hosting_count !=0) {
		pRtAddrIdTable[host_id]->parasites[pid]=0;
		pRtAddrIdTable[host_id]->hosting_count--;
	} else return RV_INVALID_PARAMETER;

	return RVF_OK;
}*/

T_RVF_G_ADDR_ID rvf_resolveHostingAddrId(T_RVM_GROUP_DIRECTIVE gd) {
	int i=0;

	for(i=1; i<MAX_RVF_TASKS; i++) {		// i=1 by-pass RVM task 
		if(pRtAddrIdTable[i]!=NULL) {
			if( pRtAddrIdTable[i]->type_code==ET2_HOST_TASK &&
//				pRtAddrIdTable[i]->priority==priority &&	// to do ...
//				pRtAddrIdTable[i]->os_stack_size>=stack_size
				pRtAddrIdTable[i]->gdHost==gd.group_directive &&
				pRtAddrIdTable[i]->hosting_count<MAX_PARASITES ) return i; // def. 10 A+ .
		}
	}

	return RVF_INVALID_ADDR_ID; /* nothing found ret. param must be invalid*/
}

/*********************************************************************
 * start() and stop() should be added to params, due to defered calling 
 * Allow for parasites to be added at RT and their start to be called.
 *********************************************************************/
T_RVF_RET rvf_create_virtual_task (T_RV_RET (* handle_message)(T_RV_HDR * msg),
								   T_RV_RET (* handle_timer)(T_RV_HDR * msg),
								   T_RVF_G_ADDR_ID task_id, T_RVF_G_ADDR_ID host_task_id, char *taskname,
								   UINT8 priority, UINT8 tcode) {

	if(!pRtAddrIdTable[task_id]) {  /* allow for static init. or previous dyn init, eg. idle*/	
		if(_RALLOC_TASK_RT_ADDR_DATA((T_RVF_BUFFER**)&pRtAddrIdTable[task_id])) return RVF_INTERNAL_ERR;
		memset( pRtAddrIdTable[task_id], 0, sizeof(T_RVF_RT_ADDR_ID_DATA) );

		rvf_mbox_buffer_init(pRtAddrIdTable[task_id]);
	} /*else printf("RVF: task rt addr %d already alloc'ed\n", task_id); */

	pRtAddrIdTable[task_id]->host_addr_id=host_task_id;
	pRtAddrIdTable[task_id]->symbolic_name=taskname;
	pRtAddrIdTable[task_id]->handle_message=handle_message;
	pRtAddrIdTable[task_id]->handle_timer=handle_timer;
	pRtAddrIdTable[task_id]->type_code=tcode;

	pRtAddrIdTable[task_id]->pOSTCB=NULL;		/* init to NULL for res.'free' */ 
	pRtAddrIdTable[task_id]->pOSEvtGrp=NULL;
	pRtAddrIdTable[task_id]->p_os_stack=NULL;
//	pRtAddrIdTable[task_id]->p_tm_q=NULL;

//	rvf_registerToHost(host_task_id, task_id);
	
	return RVF_OK;
}

T_RVF_RET rvf_register_t3_handlers (T_RVF_G_ADDR_ID task_id,
								   T_RV_RET (* handle_message)(T_RV_HDR * msg),
								   T_RV_RET (* handle_timer)(T_RV_HDR * msg) ) {

	pRtAddrIdTable[task_id]->handle_message=handle_message;
	pRtAddrIdTable[task_id]->handle_timer=handle_timer;

	return RVF_OK;
}

/* RVM must create a stack with its MB and pass as params to task create */
/*T_RVF_RET rvf_create_host_task (T_RV_RET (* proxy)(void), T_RVF_G_ADDR_ID task_id,
								   char *taskname, UINT8 *stack, UINT16 stacksize, 
								   UINT8 priority, UINT8 tcode, UINT8 time_slicing, T_RVF_TASK_STATE suspend) {

	return rvf_create_task(	(TASKPTR)proxy, task_id, taskname, stack, stacksize,\
							priority, tcode, time_slicing,	suspend ) ;
									
}*/

/*******************************************************************************
**
** Function         rvf_create_task
**
** Description      This function is called to create a new rvf task.
**                  time_slice represents the number of Nucleus ticks before a task is interrupted.
**					0 for no time-slicing.
**
** Returns          RVF_OK if successful, else an error code
**
*******************************************************************************/
T_RVF_RET rvf_create_legacy_task (TASKPTR task_entry, UINT8 task_id, char *taskname, UINT8 *stack, UINT16 stacksize, UINT8 priority, UINT8 time_slicing, T_RVF_TASK_STATE is_suspend) {

	return rvf_create_task(task_entry,task_id,taskname,stack,stacksize,priority,ET4_TASK,time_slicing,is_suspend);
}

T_RVF_RET rvf_create_task (TASKPTR task_entry, T_RVF_G_ADDR_ID task_id, char *taskname, UINT8 *stack, UINT16 stacksize,
						   UINT8 priority, UINT8 tcode, UINT8 time_slicing, T_RVF_TASK_STATE suspend) {

	/*if (task_id >= MAX_RVF_TASKS) { return 1; } */
	if (task_counter >= MAX_RVF_TASKS) { return 1; }

	/* fill the task stack with a 0xFE pattern to allow use with stack monitoring tool */
	memset( stack, 0xFE, stacksize );

	/* allow for immediate task creation, eg. no alloc taskId(), but #defined task No. Dangerous!*/	
	if(!pRtAddrIdTable[task_id]) {  
		if(_RALLOC_TASK_RT_ADDR_DATA((T_RVF_BUFFER**)&pRtAddrIdTable[task_id])) return RVF_INTERNAL_ERR;
		memset( pRtAddrIdTable[task_id], 0, sizeof(T_RVF_RT_ADDR_ID_DATA) );

		rvf_mbox_buffer_init(pRtAddrIdTable[task_id]);
	} /*else printf("RVF: task rt addr %d already alloc'ed\n", task_id); */

	/*if(_RALLOC_TASK_RT_TM((T_RVF_BUFFER**)&pRtAddrIdTable[task_id]->p_tm_q)) {
		rvf_free_buf(pRtAddrIdTable[task_id]);
		pRtAddrIdTable[task_id]=NULL;
		return RVF_INTERNAL_ERR;
	} else pRtAddrIdTable[task_id]->p_tm_q->timerCnt=0;*/
	
	pRtAddrIdTable[task_id]->hosting_count=0;
	pRtAddrIdTable[task_id]->host_addr_id=task_id;
	pRtAddrIdTable[task_id]->symbolic_name=taskname;
	pRtAddrIdTable[task_id]->priority=priority;
	pRtAddrIdTable[task_id]->type_code=tcode;

	memset(&pRtAddrIdTable[task_id]->parasites, 0, (sizeof(T_RVF_G_ADDR_ID)*MAX_PARASITES));

	if (stack) {
	    pRtAddrIdTable[task_id]->p_os_stack     = (UINT8 *)stack;// - stacksize;
	    pRtAddrIdTable[task_id]->os_stack_size	= stacksize;
    } else {
//		rvf_free_buf(pRtAddrIdTable[task_id]->p_tm_q);
		rvf_free_buf(pRtAddrIdTable[task_id]);
		pRtAddrIdTable[task_id]=NULL;
		return RVF_INTERNAL_ERR;
	}

	if(_RALLOC_TASK_CNTRL_BLK((T_RVF_BUFFER**)&pRtAddrIdTable[task_id]->pOSTCB)) {
		rvf_free_buf(stack);
//		rvf_free_buf(pRtAddrIdTable[task_id]->p_tm_q);
		rvf_free_buf(pRtAddrIdTable[task_id]);
		pRtAddrIdTable[task_id]=NULL;
		return RVF_INTERNAL_ERR;
	}
	memset( pRtAddrIdTable[task_id]->pOSTCB, 0, sizeof(NU_TASK) );

	if(_RALLOC_TASK_EVT_GRP((T_RVF_BUFFER**)&pRtAddrIdTable[task_id]->pOSEvtGrp)) {
		rvf_free_buf(pRtAddrIdTable[task_id]->pOSTCB);
		rvf_free_buf(stack);
//		rvf_free_buf(pRtAddrIdTable[task_id]->p_tm_q);
		rvf_free_buf(pRtAddrIdTable[task_id]);
		pRtAddrIdTable[task_id]=NULL;
		return RVF_INTERNAL_ERR;
	}
	memset( pRtAddrIdTable[task_id]->pOSEvtGrp, 0, sizeof(NU_EVENT_GROUP) );

	/* Create one Event Group for this task */
	if( NU_SUCCESS != \
		NU_Create_Event_Group (pRtAddrIdTable[task_id]->pOSEvtGrp,\
		taskname) ) {
		return RVF_INTERNAL_ERR;
	}

	//}
    /* Create Task */
	if ( NU_SUCCESS != 
	NU_Create_Task (pRtAddrIdTable[task_id]->pOSTCB,            /* Task Control Block */
		            taskname,     /*taskname,*/					/* Task Name */
		            (NU_TASK_ENTRY )task_entry,					/* Task Entry Function */
		            0,            /* why prev. task_id ???  */	/* ARGC  A-M-E-N-D-E-D! */
		            NULL,										/* ARGV */
		            pRtAddrIdTable[task_id]->p_os_stack,        /* Begining of Stack */
		            pRtAddrIdTable[task_id]->os_stack_size,		/* Stack size */
		            priority,									/* Priority */
		            time_slicing,								/* Time Slicing Period*/
		            NU_PREEMPT,									/* Preemption allowed */
					(OPTION)(suspend == RUNNING ? NU_START : NU_NO_START) )/* Start the task or suspend it */
		)	{	
		return RVF_INTERNAL_ERR;
	}
	task_counter++;   /* MUST 'DEC' ON TASK TERMINATION	*/
	
	return RVF_OK;
}

/* to be called from func. 'create_tasks()' of module "create_RVtasks.c" 
 * utilises static alloated system variables								*/
/*T_RVF_RET rvf_create_boot_task (TASKPTR task_entry, UINT8 task_id, char *taskname, UINT8 *stack, UINT16 stacksize, UINT8 priority, UINT8 time_slicing, T_RVF_TASK_STATE suspend) {
	if (task_id >= MAX_RVF_TASKS)	{		return 1;}

    if (stack)    {
	    bOSStack     = (UINT8 *)stack;// - stacksize;
	    bOSStackSize = stacksize;
    }
	// copy the task name into an internal buffer 
	_rvf_name_cpy( &(bOSTName[0][0]), taskname, RVF_MAX_TASK_LEN);

	// fill the task stack with a 0xFE pattern to allow use with stack monitoring tool 
	memset( stack, 0xFE, stacksize );

	// Create one Event Group for this task 
	if( NU_SUCCESS != NU_Create_Event_Group (&bOSEvtGrp[0], taskname) ){
		return RVF_INTERNAL_ERR;
	}

    // Create Task 
	if ( NU_SUCCESS != 
	NU_Create_Task (&bOSTCB[0],                 // (reserved)Task Control Block 
		            taskname,	                // Task Name 
		            (NU_TASK_ENTRY )task_entry,	// Task Entry Function 
		            task_id,		            // ARGC 
		            NULL,		                // ARGV 
		            bOSStack,					// Begining of Stack 
		            stacksize,	                // Stack size 
		            priority,					// Priority 
		            time_slicing,			    // Time Slicing Period
		            NU_PREEMPT,                 // Preemption allowed 
					(OPTION)(suspend == RUNNING ? NU_START : NU_NO_START) )// Start the task or suspend it 
		)
	{	return RVF_INTERNAL_ERR;
	}
	task_counter++;
	return RVF_OK;
}*/

/* Later timer blk must be added, once made dynamic   */ 
T_RVF_RET rvf_free_sys_resources(T_RVF_G_ADDR_ID gid, UINT8 rm) {

	if(!pRtAddrIdTable[gid]) return RVF_INTERNAL_ERR;

	if(rm==1 || rm==2) {
		if(pRtAddrIdTable[gid]->pOSTCB)rvf_free_buf(pRtAddrIdTable[gid]->pOSTCB);
		if(pRtAddrIdTable[gid]->pOSEvtGrp)rvf_free_buf(pRtAddrIdTable[gid]->pOSEvtGrp);
		if(pRtAddrIdTable[gid]->p_os_stack)rvf_free_buf(pRtAddrIdTable[gid]->p_os_stack);

		task_counter--;
	}
	if(rm==0 || rm==2) {
		if(pRtAddrIdTable[gid])rvf_free_buf(pRtAddrIdTable[gid]);
		pRtAddrIdTable[gid]=NULL;
	}

	return RVF_OK;
}

/*******************************************************************************
**
** Function         rvf_exit_task
**
** Description      This function is called to stop a rvf task.
**					A task can kill another task or itself.
**
** Returns          void
**
*******************************************************************************/
void rvf_exit_task (T_RVF_G_ADDR_ID task_id)
{
	if(!pRtAddrIdTable[task_id]) return;
	/*
	** Empty task's mail box
	*/
	_rvf_empty_mailboxes(task_id);

	/*
	** Terminate task
	*/
	NU_Terminate_Task(pRtAddrIdTable[task_id]->pOSTCB); /*&OSTCB[task_id]);*/
	NU_Delete_Task(pRtAddrIdTable[task_id]->pOSTCB);

	/*
	** Delete related event group
	*/
	NU_Delete_Event_Group (pRtAddrIdTable[task_id]->pOSEvtGrp);

	pRtAddrIdTable[task_id]->p_os_stack=0;
    //OSStack[task_id]     = 0;
}


/*******************************************************************************
**
** Function         rvf_suspend_task
**
** Description      This function is called to suspend a rvf task.
**					A task can suspend another task or itself.
**
** Returns          void
**
*******************************************************************************/
T_RVF_RET rvf_suspend_task (T_RVF_G_ADDR_ID task_id)
{
	if(!pRtAddrIdTable[task_id]) return RVF_INVALID_PARAMETER;
	NU_Suspend_Task(pRtAddrIdTable[task_id]->pOSTCB); // A-M-E-N-D-E-D! 

	return RVF_OK;
}


/*******************************************************************************
**
** Function         rvf_wait
**
** Description      This function is called by tasks to wait for a specific
**                  event or set of events. The task may specify the duration
**                  that it wants to wait for, or 0 if infinite.
**
** Returns          the event mask of received events or zero if timeout
**
*******************************************************************************/
UINT16 rvf_wait (UINT16 flag, UINT32 timeout) {
	T_RVF_G_ADDR_ID     rtask = rvf_get_taskid();

	if (!timeout) timeout = 0xFFFFFFFFL;
	
	return rvf_evt_wait(rtask, flag, timeout);
}

UINT16 rvf_evt_wait(T_RVF_G_ADDR_ID rtask, UINT16 flag, UINT32 timeout) {

#define RVF_RET_TIME_OUT	0

	UINT16		mbxEvt = 0;
	UNSIGNED    evt   = 0;
	UNSIGNED	clear = 0;
	STATUS		status_ret; 

    /* Check if anything in any of the mailboxes. Possible race condition. */
   
	if (rtask>=MAX_RVF_TASKS || !pRtAddrIdTable[rtask]) {
		RVM_TRACE_WARNING_PARAM("RVF: Illegal MBOX or MBOX not ready!", rtask);
		return (UINT16) RVF_RET_TIME_OUT; 
	}

	if (pRtAddrIdTable[rtask] && pRtAddrIdTable[rtask]->OSTaskQFirst[0])
        mbxEvt |= RVF_TASK_MBOX_0_EVT_MASK;
    if (pRtAddrIdTable[rtask] && pRtAddrIdTable[rtask]->OSTaskQFirst[1])
        mbxEvt |= RVF_TASK_MBOX_1_EVT_MASK;
    if (pRtAddrIdTable[rtask] && pRtAddrIdTable[rtask]->OSTaskQFirst[2])
        mbxEvt |= RVF_TASK_MBOX_2_EVT_MASK;
    if (pRtAddrIdTable[rtask] && pRtAddrIdTable[rtask]->OSTaskQFirst[3])
        mbxEvt |= RVF_TASK_MBOX_3_EVT_MASK;

    /* If any valid event if pending, return immediately */
    if (mbxEvt & flag)
    {
       /* Return only those bits which user wants... */
       evt = (UINT16) (mbxEvt & flag);

	   /* clear the nucleus event(s) for mailboxes */
	   if ( mbxEvt & 0x000F ) /* a mailbox event is signaled*/
	   {   NU_Retrieve_Events (pRtAddrIdTable[rtask]->pOSEvtGrp, (UNSIGNED) mbxEvt & 0x000F, NU_AND_CONSUME,
                        (UNSIGNED *)&clear, NU_NO_SUSPEND);
	   }

       	   
        return ((UINT16) evt);
    }


    if(pRtAddrIdTable[rtask]) {
		status_ret = NU_Retrieve_Events (pRtAddrIdTable[rtask]->pOSEvtGrp,
										 (UNSIGNED) flag, NU_OR_CONSUME,
										 (UNSIGNED *)&evt, timeout );
		if ( status_ret == NU_SUCCESS) {
			return (UINT16) evt;
		} else { /* timeout or error case */
			return (UINT16) RVF_RET_TIME_OUT;
		}
	} return (UINT16) RVF_RET_TIME_OUT;
}

/*******************************************************************************
**
** Function         rvf_wait_for_specific_msg
**
** Description      This function is called by tasks to wait for a specific
**                  message in the specified mailbox. The task may specify the duration
**                  that it wants to wait for, or 0 if infinite.
**
** Returns          A pointer to the message, NULL in case of time-out.
**
*******************************************************************************/
T_RVF_BUFFER * rvf_wait_for_specific_msg(UINT16 msg_code, UINT8 mbox, UINT32 timeout)
{
	T_RVF_G_ADDR_ID task_id = rvf_get_taskid();
	T_RVF_BUFFER * p_buf = NULL;
	T_RVF_INTERNAL_BUF * p_hdr;
	UNSIGNED clear = 0;
	STATUS status_ret;
	UINT32 wait_time;
	UINT32 init_time = rvf_get_tick_count();

	
	/* check input parameter */
	if ( mbox >= RVF_NUM_TASK_MBOX)  /* NOTE: must be def to 2 max */
	{	rvf_send_trace( "RVF: rvf_wait_for_specific_msg(): invalid mailbox id", 52, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVM_USE_ID);
		return p_buf;
	}

	if(!timeout)
		timeout = 0xFFFFFFFFL;
	wait_time = timeout;

	while( (rvf_get_tick_count() - init_time) < timeout  )
	{
		/* test all messages in the mailbox */
		if( pRtAddrIdTable[task_id]->OSTaskQFirst[mbox] )
		{
			rvf_disable(9);
			p_hdr = pRtAddrIdTable[task_id]->OSTaskQFirst[mbox];
			p_buf = MEM2USER(p_hdr);
			/* test the first one */
			if ( ((T_RV_HDR *)p_buf)->msg_id == msg_code )
			{	/* message found, return it */
				pRtAddrIdTable[task_id]->OSTaskQFirst[mbox] = p_hdr->p_next;
				p_hdr->p_next				= NULL;

#if RVF_ENABLE_BUF_LINKAGE_CHECK
				RVF_SET_BUF_UNLINKED(p_hdr);
#endif
				/* clear the Nucleus Event for this mailbox */
				NU_Retrieve_Events( pRtAddrIdTable[task_id]->pOSEvtGrp, EVENT_MASK(mbox), NU_AND_CONSUME, 
								(UNSIGNED *)&clear, NU_NO_SUSPEND);
				rvf_enable();
				return p_buf;
			}

			while(p_hdr->p_next != NULL)
			{
				p_buf = MEM2USER(p_hdr->p_next);
				if ( ((T_RV_HDR *)p_buf)->msg_id == msg_code )
				{	/* remove it from the list */
					p_hdr->p_next = ( (T_RVF_INTERNAL_BUF *) USER2MEM(p_buf))->p_next;
					/* check if it the last one */
					if ( pRtAddrIdTable[task_id]->OSTaskQLast[mbox]  == USER2MEM(p_buf) )
					{	pRtAddrIdTable[task_id]->OSTaskQLast[mbox]  = p_hdr;
					}
					((T_RVF_INTERNAL_BUF *) USER2MEM(p_buf))->p_next = NULL;

	#if RVF_ENABLE_BUF_LINKAGE_CHECK
					RVF_SET_BUF_UNLINKED(USER2MEM(p_buf));
	#endif
					rvf_enable();
					return p_buf;
				}
				p_hdr = p_hdr->p_next;
			}

			rvf_enable();
		}
		
		/* here, the message has not been found, so wait for a new message */
		
		if ((wait_time != 0xFFFFFFFFL) &&
			(timeout > rvf_get_tick_count() - init_time))
		{
			/* NU_Retrieve_Events bug: cannot call function with a parameter with F in MSB */
			wait_time = (timeout - (rvf_get_tick_count() - init_time)) & 0x0FFFFFFFL;
		}

		status_ret = NU_Retrieve_Events( pRtAddrIdTable[task_id]->pOSEvtGrp, EVENT_MASK(mbox), NU_OR_CONSUME,
										(UNSIGNED *)&clear, wait_time);

		if( status_ret != NU_SUCCESS) /* time out */
		{	return NULL;
		}
		
	}
	return NULL;
}


/*******************************************************************************
**
** Function         rvf_delay
**
** Description      This function is called by tasks to sleep unconditionally
**                  for a specified amount of time.
**
** Returns          void
**
*******************************************************************************/
void rvf_delay (UINT32 timeout)
{
	if (timeout == 0)
	{	timeout = 1;
	}

	NU_Sleep(timeout);

}


/*******************************************************************************
**
** Function         rvf_send_event
**
** Description      This function is called by tasks to send events to other
**                  tasks. Tasks can also send events to themselves.
**
** Returns          0 if all OK, else 1
**
*******************************************************************************/
UINT8 rvf_send_event (T_RVF_G_ADDR_ID task_id, UINT16 event) {	

	if (task_id >= MAX_RVF_TASKS || !pRtAddrIdTable[task_id] ) return 1;

	NU_Set_Events (pRtAddrIdTable[task_id]->pOSEvtGrp, (UNSIGNED)event, NU_OR);

	return 0;
}

/*******************************************************************************
**
** Function         rvf_get_taskid
**
** Description      This function gets the currently running task ID.
**
** Returns          task ID
**
*******************************************************************************/
T_RVF_G_ADDR_ID rvf_get_taskid(void) {	/* Retrieve the taskid using index of the task pointer in the OSTCB array */
	NU_TASK * currTask=0;
	T_RVF_G_ADDR_ID taskId=RVF_INVALID_ADDR_ID;

	currTask = NU_Current_Task_Pointer();

	if( currTask != NU_NULL) {
		/* find the task pointer in the OSTCB array */
		for ( taskId = 0; taskId < MAX_RVF_TASKS; taskId++) {
			if( (pRtAddrIdTable[taskId]!= NULL) && 
				((pRtAddrIdTable[taskId]->pOSTCB)) == currTask) return taskId;
		}
		return RVF_INVALID_ADDR_ID;
	} else {
		return RVF_INVALID_ADDR_ID; /* error case, must return an error code */
	}
}


/*******************************************************************************
**
** Function         rvf_get_taskname
**
** Description      This function gets the currently running task name.
**
** Returns          pointer to task name or NULL if error
**
*******************************************************************************/
char* rvf_get_taskname(void)
{	T_RVF_G_ADDR_ID id = rvf_get_taskid();
	
	if (id == 0xFF) return NULL; /* return NULL if rvf_get_taskid returns 0xFF */

    return pRtAddrIdTable[id]->symbolic_name; /*(OSTName[ id ]);*/
}

/*******************************************************************************
**
** Function         rvf_enable
**
** Description      This function enables interrupts.
**
** Returns          void
**
*******************************************************************************/
void rvf_enable(void)
{

	if( --OSDisableNesting == 0)	/* Control nesting interrupt */
	{	
		if( OSInterruptAlreadyMasked == TRUE) /*	check if interrupts have been disabled outside RVF, 
													in that case, do not enable interrupts */
		{	OSInterruptAlreadyMasked = FALSE;
		}
		else
		{	NU_Control_Interrupts(OSLastIntLevel);
		}
	}

}

#ifdef _WINDOWS

/*******************************************************************************
**
** Function         INT_Check_IRQ_Mask()
**
** Description      This function checks if the IRQ are disabled (outside RVF).
**
** Returns          IRQ mask
**
*******************************************************************************/
UINT32 INT_Check_IRQ_Mask(void)
{
	return 0;
}
#else

/*-------------------------------------------------------*/ 
/* INT_Check_IRQ_Mask()                                  */
/*-------------------------------------------------------*/
/*                                                       */
/* Description: check in the CPSR register if the IRQ	 */
/*              are masked out or not.                   */
/* ------------                                          */
/*												         */
/*-------------------------------------------------------*/
/* Declaration of ASM INT_Check_IRQ_Mask function */
UINT32 INT_Check_IRQ_Mask(void);

/*
 * FreeCalypso: this assembly function will be moved out into
 * its own source file, or maybe added to nucleus/tct.S.
 */
#if 0
asm("           .def	$INT_Check_IRQ_Mask");
asm("$INT_Check_IRQ_Mask  ");			    
asm("           .ref _INT_32_Check_IRQ_Mask");
asm(".state16");
asm("	ADR	r0,_INT_32_Check_IRQ_Mask  ");
asm("	BX r0  ");

asm("	.align");
asm("	.state32");
asm("           .def _INT_32_Check_IRQ_Mask");
asm("_INT_32_Check_IRQ_Mask  ");

asm("	MRS	r0,CPSR  ");   // pick up CPSR 
asm("	BX	lr  ");        // return to caller
#endif
#endif


/*******************************************************************************
**
** Function         rvf_disable
**
** Description      This function disables interrupts.
**
** Returns          void
**
*******************************************************************************/

#define RVF_IRQ_DISABLED_MASK 0x00000080

void rvf_disable(UINT8 who)
{

	/* Control interrupt nesting ourselves */
	if (OSDisableNesting == 0)
	{
		if ( INT_Check_IRQ_Mask() & RVF_IRQ_DISABLED_MASK)	/* if IRQ are disabled (outside RVF) */
		{	OSInterruptAlreadyMasked = TRUE;
		}
		else
		{	OSLastIntLevel = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);
		}
	}
	OSDisableNesting++;

}


/*******************************************************************************
**
** Function         rvf_used_stack
**
** Description      This function tries to calculate the amount of
**                  stack used by looking for a zero.
**
** Returns          the number of non-zero bytes on the stack
**
*******************************************************************************/
UINT16 rvf_used_stack(T_RVF_G_ADDR_ID task)
{
	UINT16 j, stacksize;
	UINT8 *p;

	if(!pRtAddrIdTable[task]) return 0;

	stacksize = pRtAddrIdTable[task]->os_stack_size; /*OSStackSize[task];*/
	p = pRtAddrIdTable[task]->p_os_stack; /*OSStack[task];*/
	for(j = 0; (j < stacksize) && (*p++ == 0xFE); j++);

	return ((UINT16)(stacksize - j));
}


/*******************************************************************************
**
** Function         rvf_dump_tasks
**
** Description      This function dump all the rvf tasks.
**
** Returns          void
**
*******************************************************************************/
void rvf_dump_tasks()
{	UINT8 num_task;
	char task_info[100];

	rvf_send_trace("*** START DUMPING TASKS ***", 27, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);
	
	/* for each task, display its name, its id, its stack size*/
	rvf_send_trace("*TASK_NAME Id Stack_size Used_stack", 35, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);
		
	for ( num_task = 0; num_task < MAX_RVF_G_ADDR_ID; num_task++ )
	{	
		/* trace the task if it has been created*/
		if (pRtAddrIdTable[num_task] != 0 ) {	
			sprintf( task_info, "%10.10s %2d      %5d      %5d",
				pRtAddrIdTable[num_task]->symbolic_name /*OSTName[num_task]*/, 
				num_task, 
				pRtAddrIdTable[num_task]->os_stack_size, 
				rvf_used_stack( num_task) );
			rvf_send_trace( task_info, 35, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_HIGH, RVM_USE_ID);		
		}
	}
	/* find a way to track task stack usage and display it */
	/* using NU_Check_Stack, NU_Task_Information or finding the first non-zero value in the stack */
}


/****************************************************************************/
/* For Big Endian Processors swap the bytes                                 */
#if defined(__BIG_ENDIAN)
UINT16 ntohs(UINT16 n)
{
	register UINT8  tmp;
	register UINT8  *p=(UINT8 *)&n;

	tmp  = p[0];
	p[0] = p[1];
	p[1] = tmp;

	return n;
}

UINT32 ntohl(UINT32 n)
{
	register UINT8 tmp;
	register UINT8 *p=(UINT8 *)&n;

	tmp  = p[0];
	p[0] = p[3];
	p[3] = tmp;

	tmp  = p[1];
	p[1] = p[2];
	p[2] = tmp;

	return n;
}

#endif /* __BIG_ENDIAN*/




/******************************************************************************
**
** Function			rvf_send_trace
**
** Description		This function displays a message essentially for debug purposes.
**					It displays the msg_length characters of the string pointed by msg
**					and the value of val.		
**
** Returns			void
**
******************************************************************************/
#ifdef  _WINDOWS 
#ifndef _CONSOLE
void rvf_send_trace1( INT8 * msg, UINT8 msg_length, UINT32 val, UINT8 TRACE_LEVEL, UINT32 swe_use_id)
{
/* Function to display trace message for Tool */

	UINT32 trace_type = swe_use_id;
	
	Trace( msg, msg_length, val, TRACE_LEVEL, trace_type);
}
#endif	
#endif	

#ifdef _CONSOLE  /* CONSOLE */
void rvf_send_trace1( INT8 * msg, UINT8 msg_length, UINT32 val, UINT8 TRACE_LEVEL, UINT32 swe_use_id) {
	const	int MAX		= 1000; 
	static	int l		= 0;
	char	buf[100];
	HANDLE	out			= 0;
    int nb;

	if(!out)out=GetStdHandle(STD_OUTPUT_HANDLE); 

	rvf_disable(25);
	
	sprintf(buf,"%s %d\n", msg, val);
	WriteConsole(out, buf, strlen(buf), &nb, NULL);
	
	if(l>=MAX) {
		system("cls");
		l=0;
	} else l++;

	rvf_enable();
}
#endif			/* CONSOLE */

/*******************************************************************************
**
** Function         rvf_resume_task
**
** Description      This function is called to resume a rvf task which is in a suspend state.
**
** Returns          RVF_OK if successful, else an error code
**
*******************************************************************************/
T_RVF_RET rvf_resume_task( T_RVF_G_ADDR_ID taskid)
{
	if(!pRtAddrIdTable[taskid]) return RVF_INVALID_PARAMETER;
	if(!pRtAddrIdTable[taskid]->pOSTCB) return RVF_INVALID_PARAMETER;
	/* resume the task */
	if ( NU_INVALID_TASK == NU_Resume_Task( pRtAddrIdTable[taskid]->pOSTCB) ) {// A-M-E-N-D-E-D!
		return RVF_INTERNAL_ERR; //RVF_INVALID_PARAMETER;
	}
	return RVF_OK;
}

/*******************************************************************************
**
** Function         rvf_initialize_mutex
**
** Description      This function initialize a mutex structure, which will be used 
**					to protect shared variables against simultaneous access.
**
** Returns          RVF_OK if successful, else an error code
**
*******************************************************************************/
T_RVF_RET		rvf_initialize_mutex( T_RVF_MUTEX * mutex)
{

	/* initializes the mutex structure */
	
	if( NU_Create_Semaphore( (NU_SEMAPHORE *)mutex, "RVF", 1, NU_PRIORITY ) != NU_SUCCESS)
	{	return RVF_INTERNAL_ERR;
	}
	
	return RVF_OK;
}

/*******************************************************************************
**
** Function         rvf_lock_mutex
**
** Description      This function locks a mutex to avoid simultaneous access.
**					If the mutex is already locked, the task is suspended 
**					until the mutex is unlocked.
**
** Returns          RVF_OK if successful, else an error code
**
*******************************************************************************/
T_RVF_RET		rvf_lock_mutex( T_RVF_MUTEX * mutex)
{
	if( NU_Obtain_Semaphore( (NU_SEMAPHORE *)mutex, NU_SUSPEND ) != NU_SUCCESS)
	{	return RVF_INTERNAL_ERR;
	}

	return RVF_OK;
}

/*******************************************************************************
**
** Function         rvf_unlock_mutex
**
** Description      This function unlocks a mutex to avoid simultaneous access.
**
** Returns          RVF_OK if successful, else an error code
**
*******************************************************************************/
T_RVF_RET		rvf_unlock_mutex( T_RVF_MUTEX * mutex)
{
	if( NU_Release_Semaphore( (NU_SEMAPHORE *)mutex ) != NU_SUCCESS)
	{	return RVF_INTERNAL_ERR;
	}

	return RVF_OK;
}

/*******************************************************************************
**
** Function         rvf_delete_mutex
**
** Description      This function deletes a previously created mutex.
**
** Returns          RVF_OK if successful, else an error code
**
*******************************************************************************/
T_RVF_RET		rvf_delete_mutex( T_RVF_MUTEX * mutex)
{
	if( NU_Delete_Semaphore( (NU_SEMAPHORE *)mutex ) != NU_SUCCESS)
	{	return RVF_INTERNAL_ERR;
	}

	return RVF_OK;
}

void rvf_yield() { NU_Relinquish(); }

/* convenience function    */ 
UINT8 rvf_isType2() {
	if(pRtAddrIdTable[rvf_get_taskid()]->type_code==ET2_HOST_TASK) return 1;
	else return 0;
}

/**
 * @file	rv_general.h
 * 
 * Definitions that are shared between S/W entities 
 * in the Riviera Environment.
 *
 * @author	David Lamy-Charrier (d-lamy@ti.com)
 * @version	0.1
 */

/*
 * Revision History:																			
 *
 *	Date       	Author					Modification
 *	-------------------------------------------------------------------
 *	03/12/1999							Create.
 *	12/03/1999	Christian Livadiotti	Replaced:
 *										#define ARRAY_TO_STREAM(p, a, l) {register int i; for
 *										(i = 0; i < l; i++) *p++ = (UINT8) a[i];}
 *										by the following to change convention of array writing.
 *	12/08/1999	Pascal Pompei			Add STREAM_TO_ARRAY
 *	11/20/2001	Vincent Oberle			- Changed T_RV_RETURN to T_RV_RETURN_PATH
 *										  Keep compatibility with a define
 *										- Documentation cleaning
 *	06/03/2002	Vincent Oberle			Added __RV_CRITICAL
 *																			
 * (C) Copyright 2002 by Texas Instruments Incorporated, All Rights Reserved
 */
#ifndef _RV_GENERAL_H_
#define _RV_GENERAL_H_

#include "general.h"
#include "rv_trace.h"

/**
 * Task IDentifiers: A-M-E-N-D-E-D!
 *    0:                   Reserved for RVM,
 *    RVTEST_MENU_TASK_ID: 'Test Selection Menu',
 *    DUMMY_TASK_ID:       'Dummy' task.
 *    MAX - 1:             Trace task (refer to rvf_target.h).
 */
#define RVTEST_MENU_TASK_ID			(0x0A)
#define DUMMY_TASK_ID               (0x0B)



/**
 * Returned parameter values.  [Perhaps, a memory level WARNING could be added]  */
typedef enum {
	RV_OK						= 0,
	RV_NOT_SUPPORTED				= -2,
	RV_NOT_READY					= -3,
	RV_MEMORY_WARNING				= -4,
	RV_MEMORY_ERR					= -5,
	RV_MEMORY_REMAINING				= -6,
	RV_INTERNAL_ERR					= -9,
	RV_INVALID_PARAMETER			        = -10
} T_RV_RET;


/**
 * Unique ADDRess IDentifier of any SWE. (T_RVF_ADDR_ID is deprecated)
 */
typedef UINT8 T_RVF_G_ADDR_ID;
#define T_RVF_ADDR_ID T_RVF_G_ADDR_ID


/**
 * Return path type.
 *
 * T_RV_RETURN_PATH is the new name for the return path type.
 * It is introduced to avoid the confusion with the return value
 * type. Use this one.
 */
typedef struct
{
	T_RVF_ADDR_ID	addr_id;
	void			(*callback_func)(void *);
} T_RV_RETURN_PATH;

// Deprecated. For backward compatibility only.
#define T_RV_RETURN		T_RV_RETURN_PATH


/**
 * Mark used to indicate that a function should be loadable.
 * For instance:
 *    char __RV_CRITICAL xxx_do_something (char toto, int bill) {
 *        ..
 */
#ifndef __RV_CRITICAL
#define __RV_CRITICAL
#endif


/**
 * Generic header of messages used in Riviera.
 */
typedef struct {
	UINT32		msg_id;
	void		(*callback_func)(void *);
	T_RVF_ADDR_ID	src_addr_id;
	T_RVF_ADDR_ID	dest_addr_id;
} T_RV_HDR;

#define RV_HDR_SIZE (sizeof (T_RV_HDR))

/**
 * Macros to get minimum and maximum between 2 numbers.
 */
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)<(b)?(b):(a))


/**
 * Macro to get minimum between 3 numbers.
 */
#define Min3(a,b,c) (Min(Min(a,b),c))

// Pointer type used to handle received data that L2CAP set in a chained buffer list.
typedef UINT8 T_RV_BUFFER;

#endif /* _RV_GENERAL_H_ */

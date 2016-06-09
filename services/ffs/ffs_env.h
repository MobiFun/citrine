/****************************************************************************/
/*                                                                          */
/*  File Name:	ffs_env.h						    */
/*                                                                          */
/*  Purpose:	This file contains prototypes for RV manager related	    */
/*		functions used to get info, start and stop the ffs block.   */
/*                                                                          */
/*  Version	0.1							    */
/*									    */
/*  Date       	Modification						    */
/*  ------------------------------------				    */
/*  10/24/2000	Create							    */
/*									    */
/*  Author	Pascal Puel						    */
/*									    */
/*  This code is in the public domain per the order of the Supreme	    */
/*  Revolutionary Command.						    */
/****************************************************************************/
#ifndef __FFS_ENV_H_
#define __FFS_ENV_H_


#include "../../riviera/rvm/rvm_gen.h"
#include "ffs_pool_size.h"	/* Stack & Memory Bank sizes definitions */


/* FFS mailbox */
#define FFS_MAILBOX	RVF_TASK_MBOX_0

/* min mem size: fd_max * fd_buf_size + 1k */
/* memory bank size and watermark */	
#define FFS_MB_PRIM_SIZE      FFS_MB1_SIZE

#define FFS_MB_PRIM_WATERMARK (FFS_MB_PRIM_SIZE - 3000)


typedef struct {
	T_RVF_MB_ID	mbid;
	T_RVF_ADDR_ID   addr_id;
} T_FFS_TASK_INFO;

extern T_FFS_TASK_INFO ffs_task_info;

/* generic functions declarations */
T_RVM_RETURN ffs_get_info (T_RVM_INFO_SWE  *infoSWE);
T_RVM_RETURN ffs_set_info(T_RVF_ADDR_ID  addr_id,
			  T_RV_RETURN		ReturnPath[],
			  T_RVF_MB_ID mbId[],
			  T_RVM_RETURN (*callBackFct)(T_RVM_NAME SWEntName,
						T_RVM_RETURN errorCause,
						T_RVM_ERROR_TYPE errorType,
						T_RVM_STRING errorMsg));
T_RVM_RETURN ffs_init (void);
T_RVM_RETURN ffs_start (void);
T_RVM_RETURN ffs_stop (void);
T_RVM_RETURN ffs_kill (void);

#endif /*__FFS_ENV_H_*/

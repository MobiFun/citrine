/****************************************************************************/
/*                                                                          */
/*  Name        rvf_cust.h                                                  */
/*                                                                          */
/*  Function    this file contains rvf cust definitions                     */
/*                                                                          */
/*  Version		0.1															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  29/11/2002	Create														*/
/*																			*/
/*	Author		Stephanie Gerthoux(s-gerthoux@ti.com)						*/
/*																			*/
/* (C) Copyright 2002 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/


#ifndef _RVF_CUST_H
#define _RVF_CUST_H


#include "../rv/general.h"


/* RVF will managed at most 2 pools of memory for dynamic allocation */
/* buffer pool structure*/
typedef struct _t_rvf_pool
{
	void * start_address;			/* address of the beginnig of the pool */
	UINT32 pool_size;				/* total size of the pool */
} T_RVF_POOL;


/* RVF function prototypes */

void _rvf_init_free_queue (UINT8 id, UINT32 size, void *p_mem);

#endif /* _RVF_I_H */

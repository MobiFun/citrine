/****************************************************************************/
/*                                                                          */
/*  Name        rvf_env.h                                                   */
/*                                                                          */
/*  Function    this file contains rvf definitions specific for the RVM.	*/
/*                                                                          */
/*  Version		0.1															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  28/01/2000	Create														*/
/*																			*/
/*	Author		David Lamy-Charrier (dlamy@tif.ti.com)						*/
/*																			*/
/* (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#ifndef _RVF_ENV_H
#define _RVF_ENV_H

#include "rvf_api.h"




/* return the size of the available memory in bytes */
T_RVF_RET rvf_get_available_mem( UINT32 * total_size, UINT32 * used_size );

/* get the parameters of a specific memory bank */
T_RVF_RET rvf_get_mb_param( T_RVF_MB_NAME mb_name, T_RVF_MB_PARAM * param);

/* change the parameters of a specific memory bank */
T_RVF_RET rvf_set_mb_param( T_RVF_MB_NAME mb_name, T_RVF_MB_PARAM * param);

/* retrieve the amount of memory available before the memory bank size */
UINT32	rvf_get_mb_unused_mem(T_RVF_MB_ID mb_id);

/* retrieve the amount of memory available before the memory bank watermark */
UINT32	rvf_get_mb_unused_green_mem(T_RVF_MB_ID mb_id);





#endif	/* _RVF_ENV_H */

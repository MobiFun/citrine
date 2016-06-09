/**
 * @file	xxx_api.h
 *
 * API Definition for XXX SWE.
 *
 * @author	Your name here (your_email_here)
 * @version 0.1
 */

/*
 * History:
 *
 *	Date       	Author					Modification
 *	-------------------------------------------------------------------
 *	//							Create.
 *
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef __XXX_API_H_
#define __XXX_API_H_


#include "rvm/rvm_gen.h"		/* Generic RVM types and functions. */


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @name XXX Return type and return values.
 *
 * Currently they are the standard RV return types, but they may
 * be customized in the future.
 */
/*@{*/
typedef T_RV_RET T_XXX_RETURN;

#define	XXX_OK					RV_OK				
#define	XXX_NOT_SUPPORTED		RV_NOT_SUPPORTED		
#define	XXX_MEMORY_ERR			RV_MEMORY_ERR			
#define	XXX_INTERNAL_ERR		RV_INTERNAL_ERR		
/*@}*/


/**
 * @name API functions
 *
 * API functions declarations (bridge functions).
 */
/*@{*/

T_XXX_RETURN xxx_send_sample ();

/*@}*/


#ifdef __cplusplus
}
#endif


#endif /*__XXX_API_H_*/


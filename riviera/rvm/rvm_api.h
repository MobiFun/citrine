/**
 *                                                                          
 *  @file	rvm_api.h                                              
 *                                                                          
 *  Public interface of the Riviera Manager.
 *
 * @author	Cristian Livadiotti (c-livadiotti@ti.com)
 * @version	0.2
 *
 */

/*
 * Revision History:																			
 *
 * 06/04/2000	Cristian Livadiotti		Create.
 * 10/18/2001	David Lamy-Charrier		Update for new Riviera 1.6.
 *																			
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */


#ifndef __RVM_API_H_
#define __RVM_API_H_

#include "../rv/rv_general.h"
//#include "rvf_i.h"


#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
* Message type used by rvf: add, del and  Modify timers
*******************************************************/
#define RVM_MSG_OFFSET_PUB		BUILD_MESSAGE_OFFSET(RVM_USE_ID)

#define RVM_TMS_MSG					(RVM_MSG_OFFSET_PUB | 0x004)
#define RVF_NOTIFY_MSG				(RVM_MSG_OFFSET_PUB | 0x006)

/******************************************************************/
/* RVM functions return type and values                           */
/******************************************************************/

/* Return type */
typedef T_RV_RET T_RVM_RETURN;


/* RVM return parameters definition */
#define	RVM_OK			RV_OK				
#define	RVM_NOT_SUPPORTED	RV_NOT_SUPPORTED		
#define	RVM_NOT_READY		RV_NOT_READY			
#define	RVM_MEMORY_ERR		RV_MEMORY_ERR			
#define	RVM_INTERNAL_ERR	RV_INTERNAL_ERR		
#define	RVM_INVALID_PARAMETER	RV_INVALID_PARAMETER	


/******************************************************************/
/* Definition / Types related to both API functions and messages */
/******************************************************************/

/* RVM USEID definition (Unique Software Entity IDentifier)*/
typedef UINT32 T_RVM_USE_ID;

#define RVM_INVALID_USE_ID		(0)

/* RVM name prototype */
#define RVM_NAME_MAX_LEN		(10)
typedef char T_RVM_NAME[RVM_NAME_MAX_LEN];

typedef UINT32 T_RVM_VERSION;


/***********************************************/
/* Definition / Types related to messages only */
/***********************************************/

/* Message used as a result from a rvm_start or rv_stop function call */
/*--------------------------------------------------------------------*/
#define	RVM_EVT_TO_APPLI (60009) // Value of the event field in the message buffer


/* Action field of the results message to application */
typedef enum {
    RVM_START_APPLI		= 0,
    RVM_STOP_APPLI		= 1
}T_RVM_APPLI_ACTION;


/* structure of RVM returned message to appli */
typedef struct
{
	T_RV_HDR		header;		// event field will be RVM_EVT_TO_APPLI
	T_RV_RETURN_PATH	rp;
	T_RVM_APPLI_ACTION	action;		// Indicates if the result is coming from a rvm_start or a rvm_stop function call
	T_RVM_NAME		swe_name;	// Name of the SW entity
	UINT8			swe_index;	// Number of the SW entity
	T_RVM_RETURN		result;
} T_RVM_APPLI_RESULT;




/***************************************/
/* RVM external functions declarations */
/***************************************/

/* enable init at boot up   */
BOOLEAN		rvm_init(void);
/* launch the creation process to start the specified SWE */
T_RVM_RETURN	rvm_start_swe( T_RVM_USE_ID swe_id, T_RV_RETURN_PATH return_path);

/* stop the specified SWE */
T_RVM_RETURN	rvm_stop_swe( T_RVM_USE_ID swe_id, T_RV_RETURN_PATH return_path);
T_RVM_RETURN	rvm_swe_stopped(T_RV_HDR* p_msg);

/* function called to start the RV Environment */
T_RVM_RETURN	rvm_start_environment();

/* function called to get the return_path of a SWE  */
T_RVM_RETURN	rvm_get_swe_information( T_RVM_USE_ID swe_id, 
										 T_RV_RETURN_PATH * return_path);

/* function called to get the user-friendly name of a SWE  */
T_RVM_RETURN	rvm_get_swe_name( T_RVM_USE_ID swe_id, 
								  T_RVM_NAME swe_name);

/* function called to get the version number of a SWE  */
T_RVM_RETURN	rvm_get_swe_version( T_RVM_USE_ID swe_id, 
									 T_RVM_VERSION * swe_version);

#ifdef __cplusplus
}
#endif


#endif /* __RVM_API_H_ */

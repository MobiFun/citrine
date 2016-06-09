/**
 * @file	xxx_message.h
 *
 * Data structures:
 * 1) used to send messages to the XXX SWE,
 * 2) XXX can receive.
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

#ifndef __XXX_MESSAGE_H_
#define __XXX_MESSAGE_H_


#include "rv/rv_general.h"

#include "xxx/xxx_cfg.h"


#ifdef __cplusplus
extern "C"
{
#endif


/** 
 * The message offset must differ for each SWE in order to have 
 * unique msg_id in the system.
 */
#define XXX_MESSAGE_OFFSET	 BUILD_MESSAGE_OFFSET(XXX_USE_ID)



/**
 * @name XXX_SAMPLE_MESSAGE
 *
 * Short description.
 *
 * Detailled description
 */
/*@{*/
/** Message ID. */
#define XXX_SAMPLE_MESSAGE (XXX_MESSAGE_OFFSET | 0x001)

/** Message structure. */
typedef struct 
{
	/** Message header. */
	T_RV_HDR			hdr;

	/** Some parameters. */
	/* ... */

} T_XXX_SAMPLE_MESSAGE;
/*@}*/


#ifdef __cplusplus
}
#endif

#endif /* __XXX_MESSAGE_H_ */

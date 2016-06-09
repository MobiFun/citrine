/*******************************************************************************
 *
 * rvt_def_i.h
 *
 * (C) Texas Instruments, all rights reserved
 *
 * Version number	: 0.1
 *
 * History			: 0.1 (7/5/2000) - Created
 *
 * Date             : 7/5/2000
 *
 * Author           : Guido Pagana  g-pagana@ti.com
 *
 ******************************************************************************/

#ifndef __RVT_DEF_I_H__
#define __RVT_DEF_I_H__

#include "rvt_gen.h"

#include "rvt_pool_size.h"	/* Stack & Memory Bank sizes definitions */

/************************* Internal definitions **************************/


/* Define the maximum number of msg lost before sending an alert */
#define RVT_MAX_LOST_TRACE_MSG (1)

/* Trace user database (must end using RVT_END_OF_TABLE) */
#define RVT_END_OF_TABLE	{"", RVT_INVALID_HEADER, NULL}

#define RVT_USER_ID_MAPPING { \
	{"RV",		RVT_RV_HEADER,		NULL}, \
	{"L1",		RVT_L1_HEADER,		NULL}, \
	{"L23",		RVT_L23_HEADER,		NULL}, \
	{"TM",		RVT_TM_HEADER,		NULL}, \
	{"RNET",	RVT_RNET_HEADER,	NULL}, \
	{"PROF",	RVT_PROF_HEADER,	NULL}, \
	{"GTTBACK",	RVT_GTTBACK_HEADER,	NULL}, \
	{"OTHER",	RVT_OTHER_HEADER,	NULL}, \
	{"AT",		RVT_AT_HEADER,		NULL}, \
	{"EXTUI",	RVT_EXTUI_HEADER,	NULL}, \
	{"TCH",		RVT_TCH_HEADER,		NULL}, \
	{"KEEPALIVE",	RVT_KEEPALIVE_HEADER,	NULL}, \
	RVT_END_OF_TABLE \
}


/* Trace task mailbox */
#define RVT_TRACE_MAILBOX	RVF_TASK_MBOX_0

typedef struct {
	T_RVT_NAME		user_name[RVT_NAME_MAX_LEN];
	T_RVT_USER_ID		user_id;
	RVT_CALLBACK_FUNC	rx_callback_func;
} T_RVT_USER_DB;


/* Define the unique message type used in RVT */
#define RVT_TRACE_RQST_ID ( (RVT_USE_ID & 0xFFFF0000) | ( 0x1) | 0x001)

typedef struct {
	T_RV_HDR	header;
	T_RVT_USER_ID	user_id;
	T_RVT_FORMAT	format;
	T_RVT_MSG_LG	msg_length;
} T_RVT_TRACE_RQST;


/* Trace message header size */
#define RVT_HEADER_SIZE	(sizeof(T_RVT_TRACE_RQST) )

#define TRACE_TASK_STACK_SIZE TRACE_STACK_SIZE


#define TRACE_MB_SIZE		(TRACE_MB1_SIZE)
#define TRACE_MB_WATERMARK	(TRACE_MB_SIZE)

/* Decimal to hexadecimal conversion table */
static const char Num2Char[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

/*
** Number of messages lost. Causes:
**
**    __ 'Format' differs from RVT_ASCII_FORMAT and RVT_BINARY_FORMAT.
**   |  __ 'Message ID' differs from RVT_TRACE_RQST_ID.
**   | |  __ rvf_read_mbox has returned NULL.
**   | | |  __ Failed to send the message.
**   | | | |  __ Insufficient resources.
**   | | | | |  __ Trace Module not started.
**   |_|_|_|_|_|____________________________________________________
**  | ¦ ¦ ¦ ¦ ¦ ¦   |               |               |               |
**  | ¦ ¦ ¦ ¦ ¦ ¦   |               |               |               |
**  |_¦_¦_¦_¦_¦_¦_._|_._._._._._._._|_._._._._._._._|_._._._._._._._|
*/
typedef union
{
	struct
	{
		INT32  count                  : 26;
		INT32  not_started            : 1;
		INT32  insufficient_resources : 1;
		INT32  message_not_sent       : 1;
		INT32  message_empty          : 1;
		INT32  unknown_request        : 1;
		INT32  unknown_format         : 1;
	} bit_mask;
	UINT32     overall_value;
} T_RVT_LOST_MSG;

extern T_RVT_LOST_MSG  rvt_lost_msg_cpt;

/* Prototypes */
BOOLEAN rvt_name_cmp( char * str1, char * str2);
void rvt_activate_RX_HISR (void);                                    
void rvt_RX_process (void);

#endif /* __RVT_DEF_I_H__ */

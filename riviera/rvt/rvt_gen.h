/*******************************************************************************
 *
 * tr_gen.h
 *
 * This file contain general definitions of Trace Module.
 * Definitions are exportable
 *
 * (C) Texas Instruments, all rights reserved
 *
 * Version number	: 0.1
 *
 * History			: 0.1 (7/4/2000) - Created
 *
 * Date             : 7/4/2000
 *
 * Author           : Cristian Livadiotti, c-livadiotti@ti.com
 *
 ******************************************************************************/

#ifndef __RVT_GEN_H__
#define __RVT_GEN_H__

#include "../rv/general.h"

/*
** If TRACE_MODULE is defined, the trace is using the trace task API.
** Otherwise, the trace is using the RVF.  NOW OBSOLETE !!!!!!!!
*/
//#define TRACE_MODULE


/*
** If FRAMING_PROTOCOL is defined, the trace mux should be used because of encapsulation and byte stuffing.
** Otherwise, the hyperterminal should be used.
*/
#define FRAMING_PROTOCOL


/*
** If MIXED_TRACE is defined, all traces are mixed on the same UART and the
** trace task handles the RX commands.
*/
//#define MIXED_TRACE

/*
** Type definitions
*/

/* Define return parameters.                                                     */
typedef enum {
	RVT_OK			= 0,
	RVT_NOT_SUPPORTED	= -2,
	RVT_NOT_READY		= -3,
	RVT_MEMORY_ERR		= -4,
	RVT_INTERNAL_ERR	= -9,
	RVT_INVALID_PARAMETER	= -10
} T_RVT_RET;



typedef UINT8  T_RVT_USER_ID;
typedef UINT32 T_RVT_MSG_LG;

typedef enum { 
	RVT_ASCII_FORMAT,   // 0
	RVT_BINARY_FORMAT   // 1
} T_RVT_FORMAT;


typedef UINT8 * T_RVT_BUFFER;

typedef char T_RVT_NAME;
#define RVT_NAME_MAX_LEN		(10)

#define RVT_RV_HEADER        0x11
#define RVT_L1_HEADER        0x12
#define RVT_L23_HEADER       0x13
#define RVT_TM_HEADER        0x14
#define RVT_RNET_HEADER      0x15
#define RVT_PROF_HEADER      0x16
#define RVT_GTTBACK_HEADER   0x17
#define RVT_OTHER_HEADER     0x18
/* FreeCalypso additions */
#define RVT_AT_HEADER        0x1A
#define RVT_EXTUI_HEADER     0x1B
#define RVT_TCH_HEADER       0x1C
#define RVT_KEEPALIVE_HEADER 0x1D
/* end of FreeCalypso additions */
#define RVT_INVALID_HEADER   0xFF

typedef void (*RVT_CALLBACK_FUNC)(T_RVT_BUFFER, UINT16);

/********* Fonction Prototypes ***********/
void
rvt_set_trace_level(T_RVT_BUFFER p_msg, UINT16 msg_length);

T_RVT_RET
rvt_send_trace_no_cpy(T_RVT_BUFFER p_buffer,
			T_RVT_USER_ID user_id,
			T_RVT_MSG_LG msg_lenght,
			T_RVT_FORMAT msg_format);

T_RVT_RET 
rvt_send_trace_cpy(T_RVT_BUFFER msg,
			T_RVT_USER_ID user_id,
			T_RVT_MSG_LG msg_length,
			T_RVT_FORMAT msg_format);


T_RVT_RET
rvt_register_id(T_RVT_NAME name[], T_RVT_USER_ID *tr_id, RVT_CALLBACK_FUNC callback);

T_RVT_RET
rvt_mem_alloc(T_RVT_USER_ID user_id, T_RVT_MSG_LG buffer_lenght, T_RVT_BUFFER * buff);

T_RVT_RET 
rvt_mem_free(T_RVT_BUFFER msg);


#endif

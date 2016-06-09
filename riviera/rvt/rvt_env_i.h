/*******************************************************************************
 *
 * rvt_env_i.h
 *
 * This file should be included by rvt_env.c ONLY!!!
 * It includes definitions of global variables used for trace.
 *
 * (C) Texas Instruments, all rights reserved
 *
 * Version number	: 0.11
 *
 * History			: 0.1  (7/4/2000) - Created
 *       			: 0.11 (6/13/2002) - Remove useless dependency.
 *
 * Date             : 6/13/2002
 *
 * Author           : Cristian Livadiotti, c-livadiotti@ti.com
 *
 ******************************************************************************/

#ifndef __RVT_ENV_I_H__
#define __RVT_ENV_I_H__


/*
** Environment global variables
*/
extern T_RVF_MB_ID    rvt_mb_id;
extern T_RVF_ADDR_ID  rvt_addr_id;


/*
** Trace Module State
*/
extern T_RVT_STATE    rvt_module_state;

extern char           *p_rvt_lost_msg;
extern char           *p_rvt_sys_time;
extern T_RVT_USER_DB  rvt_user_db [];
extern T_RVT_USER_ID  rv_trace_user_id;

#define RVT_LOST_MSG              ("RVT: Lost Message ")
#define RVT_LOST_MSG_LENGTH       (sizeof (RVT_LOST_MSG) - 1)

#define RVT_SYS_TIME              ("RVT: System Time ")
#define RVT_SYS_TIME_LENGTH       (sizeof (RVT_SYS_TIME) - 1)

#define RVT_HEX_VALUE_LENGTH      (sizeof ("00000000") - 1)

#ifndef FRAMING_PROTOCOL
	#define RVT_HYPERTERM_LENGTH  (sizeof ('\n') + sizeof ('\r'))
#else
	#define RVT_HDR_LENGTH        (6)
#endif

#endif

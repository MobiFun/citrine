/********************************************************************************
 *
 * rvt_env.c
 *
 * This module interfaces the environment and contains all start/init/stop...
 * functions of the trace module.
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
 ********************************************************************************/

#include "../../include/config.h" 

#include <string.h>

#include "../rvf/rvf_api.h"
#include "../rv/rv_general.h"
#include "../rvm/rvm_gen.h"
#include "../rvm/rvm_priorities.h"

#include "rvt_gen.h"
#include "rvt_def_i.h"
#include "rvt_env.h"
#include "rvt_env_i.h"

#include "../rvm/rvm_use_id_list.h"

#include "../../serial/serialswitch.h"

#include "../../nucleus/nucleus.h"

#define TI_RX_HISR_STACK_SIZE     (1024)
static UINT8  TI_RX_HISR_stack[TI_RX_HISR_STACK_SIZE];

char           *p_rvt_lost_msg  = NULL;
char           *p_rvt_sys_time  = NULL;
NU_HISR        TI_rcv_HISR;
T_RVF_MB_ID    rvt_mb_id        = 0;
T_RVT_STATE    rvt_module_state = RVT_NOT_STARTED;
T_RVF_ADDR_ID  rvt_addr_id      = 0;


/********************************************************************************/
/*                                                                              */
/*    Function Name:   rvt_get_info												*/
/*                                                                              */
/*    Purpose:         This function is used to notify the RVM					*/
/*                     of the Trace's Memory Banks requirements.				*/
/*                                                                              */
/*    Input Parameters:                                                         */
/*        &p_info_swe (refer to rvm_gen.h).                                     */
/*                                                                              */
/*    Output Parameters:                                                        */
/*        p_info_swe (refer to rvm_gen.h).                                      */
/*                                                                              */
/*    Global Parameters:                                                        */
/*        None.                                                                 */
/*                                                                              */
/*    Note:                                                                     */
/*        None.                                                                 */
/*                                                                              */
/********************************************************************************/

T_RVM_RETURN
rvt_get_info (T_RVM_INFO_SWE  *p_info_swe)
{
  if (!p_info_swe)
    return RVM_INVALID_PARAMETER;

  p_info_swe->swe_type                                  = RVM_SWE_TYPE_4;

  p_info_swe->type_info.type4.swe_use_id                = RVT_USE_ID;
  p_info_swe->type_info.type4.version                   = 1;
  memcpy (p_info_swe->type_info.type4.swe_name,
          "TRACE",
          sizeof ("TRACE"));

  p_info_swe->type_info.type4.stack_size                = TRACE_TASK_STACK_SIZE;
  p_info_swe->type_info.type4.priority                  = RVM_TRACE_TASK_PRIORITY;

  p_info_swe->type_info.type4.return_path.addr_id       = 0;
  p_info_swe->type_info.type4.return_path.callback_func = NULL;

  // Memory banks definitions. For now, one memory bank is needed
  p_info_swe->type_info.type4.nb_mem_bank                          = 1;
  memcpy (p_info_swe->type_info.type4.mem_bank[0].bank_name,
          "RV_TRACE",
          RVM_NAME_MAX_LEN);
  p_info_swe->type_info.type4.mem_bank[0].initial_params.size      = TRACE_MB_SIZE;
  p_info_swe->type_info.type4.mem_bank[0].initial_params.watermark = TRACE_MB_WATERMARK;

  // Generic functions
  p_info_swe->type_info.type4.init     = rvt_init;
  p_info_swe->type_info.type4.core     = (T_RVM_SWE_CORE_FUNC)rvt_task_core;
  p_info_swe->type_info.type4.stop     = rvt_stop;
  p_info_swe->type_info.type4.kill     = rvt_kill;
  p_info_swe->type_info.type4.set_info = rvt_set_info;

  // Linked SW entities: none, except if Profiler or Nucleus Monitor are defined
  #if (TI_PROFILER == 1) || (TI_NUC_MONITOR == 1)
    p_info_swe->type_info.type4.nb_linked_swe    = 1;
    p_info_swe->type_info.type4.linked_swe_id[0] = TI_PRF_USE_ID;
  #else
    p_info_swe->type_info.type4.nb_linked_swe    = 0;
  #endif

  return RVM_OK;
}


/********************************************************************************/
/*                                                                              */
/*    Function Name:   rvt_set_info												*/
/*                                                                              */
/*    Purpose:         This function is used to notify the Trace module         */
/*                     about mb ids and task id.								*/
/*                                                                              */
/*    Input Parameters:                                                         */
/*        bk_id[]: array of memory bank IDs.									*/
/*                                                                              */
/*    Output Parameters:                                                        */
/*        None.																	*/
/*                                                                              */
/*    Global Parameters:                                                        */
/*        None.                                                                 */
/*                                                                              */
/*    Note:                                                                     */
/*        None.                                                                 */
/*                                                                              */
/********************************************************************************/

T_RVM_RETURN
rvt_set_info (T_RVF_ADDR_ID     addr_id,
              T_RV_RETURN_PATH  return_path[],
              T_RVF_MB_ID       bk_id[],
              T_RVM_CB_FUNC     rvm_error_ft)
{

  // Save parameters given by the environment (Addr ID and MB ID respectively)
  rvt_addr_id = addr_id;
  rvt_mb_id   = bk_id [0];

  return RVM_OK;
}


/********************************************************************************/
/*                                                                              */
/*    Function Name:   rvt_init													*/
/*                                                                              */
/*    Purpose:         This is the initialization function.						*/
/*					   It is called by the RVM.									*/
/*                                                                              */
/*    Input Parameters:                                                         */
/*        None.																	*/
/*                                                                              */
/*    Output Parameters:                                                        */
/*        None.																	*/
/*                                                                              */
/*    Global Parameters:                                                        */
/*        None.                                                                 */
/*                                                                              */
/*    Note:                                                                     */
/*        None.                                                                 */
/*                                                                              */
/********************************************************************************/

T_RVM_RETURN
rvt_init (void)
{

  if (rvt_module_state != RVT_NOT_STARTED)
    return RVM_NOT_READY;

  // The stack is entirely filled with the pattern 0xFE
  memset (TI_RX_HISR_stack,
          0xFE,
          TI_RX_HISR_STACK_SIZE);

  // Create the RX HISR
  if (NU_Create_HISR (&TI_rcv_HISR, \
                      "TI_RCV_HISR", \
                      rvt_RX_process, \
                      2, \
                      TI_RX_HISR_stack, \
                      TI_RX_HISR_STACK_SIZE) != NU_SUCCESS)
    return RVM_INTERNAL_ERR;

  // Initialize the baud rate and the callback function for the RX HISR
  SER_tr_Init (SER_LAYER_1,
               TR_BAUD_CONFIG,
               rvt_activate_RX_HISR); 

  #ifdef FRAMING_PROTOCOL

    // Allocate a buffer that reports the system time (refer to rvf_trace_adapt.c):
    //  - 'User ID' (1 byte),
    //  - 'Trace Type' (4 bytes),
    //  - 'Trace Level' (1 byte),
    //  - 'System Time' (RVT_SYS_TIME_LENGTH bytes),
    //  - Hexadecimal value (RVT_HEX_VALUE_LENGTH bytes).
    if (rvf_get_buf (rvt_mb_id, \
                     (RVT_HDR_LENGTH + RVT_SYS_TIME_LENGTH + RVT_HEX_VALUE_LENGTH), \
                     (T_RVF_BUFFER**)&p_rvt_sys_time) != RVF_RED)
    {
       UINT8  *header_p = NULL;

       header_p    = (UINT8*)p_rvt_sys_time;
       *header_p++ = (UINT8)rv_trace_user_id;
       *header_p++ = 0;
       *header_p++ = 0;
       *header_p++ = 0;
       *header_p++ = 0;
       *header_p++ = RV_TRACE_LEVEL_ERROR;
       memcpy (header_p,
               RVT_SYS_TIME,
               RVT_SYS_TIME_LENGTH);
    }

    // Allocate a buffer that reports a lack of memory (refer to rvf_trace_adapt.c):
    //  - 'User ID' (1 byte),
    //  - 'Trace Type' (4 bytes),
    //  - 'Trace Level' (1 byte),
    //  - 'Lost Message' (RVT_LOST_MSG_LENGTH bytes),
    //  - Hexadecimal value (RVT_HEX_VALUE_LENGTH bytes).
    if (rvf_get_buf (rvt_mb_id,
                     (RVT_HDR_LENGTH + RVT_LOST_MSG_LENGTH + RVT_HEX_VALUE_LENGTH), \
                     (T_RVF_BUFFER**)&p_rvt_lost_msg) != RVF_RED)
    {
       UINT8  *header_p = NULL;

       header_p    = (UINT8*)p_rvt_lost_msg;
       *header_p++ = (UINT8)rv_trace_user_id;
       *header_p++ = 0;
       *header_p++ = 0;
       *header_p++ = 0;
       *header_p++ = 0;
       *header_p++ = RV_TRACE_LEVEL_ERROR;
       memcpy (header_p,
               RVT_LOST_MSG,
               RVT_LOST_MSG_LENGTH);
    }
  #else

    // Allocate a buffer that reports the system time
    if (rvf_get_buf (rvt_mb_id, \
                     (RVT_SYS_TIME_LENGTH + RVT_HEX_VALUE_LENGTH + RVT_HYPERTERM_LENGTH), \
                     (T_RVF_BUFFER**)&p_rvt_sys_time) != RVF_RED)
    {
       memcpy (p_rvt_sys_time,
               RVT_SYS_TIME,
               RVT_SYS_TIME_LENGTH);
    }

    // Allocate a buffer that reports a lack of memory
    if (rvf_get_buf (rvt_mb_id, \
                     (RVT_LOST_MSG_LENGTH + RVT_HEX_VALUE_LENGTH + RVT_HYPERTERM_LENGTH), \
                     (T_RVF_BUFFER**)&p_rvt_lost_msg) != RVF_RED)
    {
        memcpy (p_rvt_lost_msg,
                RVT_LOST_MSG,
                RVT_LOST_MSG_LENGTH);
    }
  #endif

  // State RVT as "STARTED"
  rvt_module_state = RVT_STARTED;

  return RVM_OK;
}


/********************************************************************************/
/*                                                                              */
/*    Function Name:   rvt_stop													*/
/*                                                                              */
/*    Purpose:         This function is defined for compilation only.			*/
/*                                                                              */
/*    Input Parameters:                                                         */
/*        None.																	*/
/*                                                                              */
/*    Output Parameters:                                                        */
/*        None.																	*/
/*                                                                              */
/*    Global Parameters:                                                        */
/*        None.                                                                 */
/*                                                                              */
/*    Note:                                                                     */
/*        None.                                                                 */
/*                                                                              */
/********************************************************************************/

T_RVM_RETURN
rvt_stop (void)
{
  return RVM_OK;
}


/********************************************************************************/
/*                                                                              */
/*    Function Name:   rvt_kill													*/
/*                                                                              */
/*    Purpose:         This function is defined for compilation only.			*/
/*                                                                              */
/*    Input Parameters:                                                         */
/*        None.																	*/
/*                                                                              */
/*    Output Parameters:                                                        */
/*        None.																	*/
/*                                                                              */
/*    Global Parameters:                                                        */
/*        None.                                                                 */
/*                                                                              */
/*    Note:                                                                     */
/*        None.                                                                 */
/*                                                                              */
/********************************************************************************/

T_RVM_RETURN
rvt_kill (void)
{
  return RVM_OK;
}

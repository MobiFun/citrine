/* 
+------------------------------------------------------------------------------
|  File:       rivbridge_api.h
+------------------------------------------------------------------------------
|  Copyright 2003 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  Definitions for the rivbridge api.
+----------------------------------------------------------------------------- 
*/ 

#ifndef REMU_H
#define REMU_H

/*==== CONSTANTS ============================================================*/

#define RIV_MEMORY_POOL 1

#define rvf_add_to_timer_list  gsp_add_to_timer_list  
#define rvf_change_callback_func gsp_change_callback_func 
#define rvf_count_buf gsp_count_buf 
#define rvf_create_legacy_task  gsp_create_legacy_task  
#define rvf_create_mb gsp_create_mb 
#define rvf_create_task  gsp_create_task  
#define rvf_create_timer  gsp_create_timer
#define rvf_disable gsp_disable
#define rvf_delay gsp_delay 
#define rvf_delete_mb gsp_delete_mb 
#define rvf_delete_mutex gsp_delete_mutex 
#define rvf_dequeue   gsp_dequeue   
#define rvf_dump_mem gsp_dump_mem 
#define rvf_dump_pool gsp_dump_pool 
#define rvf_dump_tasks gsp_dump_tasks 
#define rvf_enable gsp_enable
#define rvf_enqueue  gsp_enqueue  
#define rvf_enqueue_head  gsp_enqueue_head  
#define rvf_evt_wait gsp_evt_wait 
#define rvf_exit_task gsp_exit_task 
#define rvf_free_buf gsp_free_buf 
#define rvf_free_msg    gsp_free_msg    
#define rvf_free_timer_msg   gsp_free_timer_msg   
#define rvf_get_buf gsp_get_buf 
#define rvf_get_buf_size  gsp_get_buf_size  
#define rvf_get_context gsp_get_context 
#define rvf_get_expired_entry  gsp_get_expired_entry  
#define rvf_get_mb_id gsp_get_mb_id 
#define rvf_get_mb_param gsp_get_mb_param 
#define rvf_get_mb_status gsp_get_mb_status 
#define rvf_get_mb_unused_mem gsp_get_mb_unused_mem 
#define rvf_get_msg_buf    gsp_get_msg_buf    
#define rvf_get_protected_buf gsp_get_protected_buf 
#define rvf_get_taskid gsp_get_taskid 
#define rvf_get_taskname gsp_get_taskname 
#define rvf_get_tick_count gsp_get_tick_count 
#define rvf_get_time_stamp gsp_get_time_stamp 
#define rvf_init gsp_init 
#define rvf_init_timer_list  gsp_init_timer_list  
#define rvf_init_timer_list_entry  gsp_init_timer_list_entry  
#define rvf_initialize_mutex gsp_initialize_mutex 
//OMAPS72906
#define rvf_initialize_static_mutex gsp_initialize_static_mutex
#define rvf_lock_mutex gsp_lock_mutex 
#define rvf_mb_is_used gsp_mb_is_used 
#define rvf_read_addr_mbox  gsp_read_addr_mbox  
#define rvf_read_mbox   gsp_read_mbox   
#define rvf_remove_from_queue  gsp_remove_from_queue  
#define rvf_remove_from_timer_list  gsp_remove_from_timer_list  
#define rvf_reset_timer gsp_reset_timer
#define rvf_resume_task gsp_resume_task 
#define rvf_scan_next  gsp_scan_next  
#define rvf_send_event  gsp_send_event  
#define rvf_send_msg    gsp_send_msg    
//#define rvf_send_trace1 gsp_send_trace1
#define rvf_set_callback_func gsp_set_callback_func 
#define rvf_set_mb_param gsp_set_mb_param 
#define rvf_start_timer gsp_start_timer 
#define rvf_stop_timer  gsp_stop_timer  
#define rvf_suspend_task gsp_suspend_task 
#define rvf_unlock_mutex gsp_unlock_mutex 
#define rvf_update_timer_list  gsp_update_timer_list  
#define rvf_wait gsp_wait 
#define rvf_wait_for_specific_msg gsp_wait_for_specific_msg 
#define rvm_error gsp_error
/* not really a riviera frame function, but seems to be needed */
//#define rvt_set_trace_level gsp_set_trace_level

/*==== INCLUDES =============================================================*/

#include "rvf/rvf_original_api.h"
#define EXT_MEM_POOL 0x0
#if (LOCOSTO_LITE==1)
#define INT_MEM_POOL EXT_MEM_POOL /* no distinction between internal and external pool. So make it same */
#else
#define INT_MEM_POOL 0x1
#endif

/*==== TYPES ================================================================*/

//typedef UINT8 * T_RVT_BUFFER;

/*==== PROTOTYPES ===========================================================*/

extern UINT8 gsp_trace_level;
extern UINT32 gsp_layer_mask;

/*==== MACROS ===============================================================*/


#endif /* remu_H */        

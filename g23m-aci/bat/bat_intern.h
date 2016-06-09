/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI
+-----------------------------------------------------------------------------
|  Copyright 2005 Texas Instruments Berlin, AG
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
|  Purpose :  This Modul holds the definitions for internal use
|             for the binary AT command library 
+-----------------------------------------------------------------------------
*/
#ifndef BAT_INTERN_H
#define BAT_INTERN_H


/*********************************************************************************
 *
 * defines
 *
 *********************************************************************************/

#define BAT_CONTROL_CHANNEL 0xFE
#define BAT_BROADCAST_CHANNEL 0xFF
#define BAT_INVALID_CLIENT_HANDLE 0xFFFF
#define BAT_INVALID_INSTANCE_HANDLE 0xFF

#define GET_INST_HNDL_FROM_CLNT_HANDLE(clnt_hndl) ((U8)((clnt_hndl & 0xff00) >> 8))
#define GET_CLNT_ID_FROM_CLNT_HANDLE(clnt_hndl) ((U8)(clnt_hndl & 0xff))
#define MAKE_CLNT_HNDL(clnt_id, inst_hndl) ((U16)((clnt_id & 0x00ff)|((0x00ff & inst_hndl)<<8)))
#define MAKE_UNS_CLNT_HNDL(inst_hndl) ((U16)((0x00ff & inst_hndl)<<8|0x00ff))

#ifdef NEW_FRAME
  #include "vsi.h"
  #ifndef _SIMULATION_
    EXTERN  T_HANDLE GDD_DIO_handle;  /* include of gdd_dio.h needs a lot of other includes */
    #undef  TRACE_FUNCTION
    #define TRACE_FUNCTION(a) vsi_o_ttrace(GDD_DIO_handle,TC_EVENT,a)  
    #undef  TRACE_EVENT
    #define TRACE_EVENT(a) vsi_o_ttrace(GDD_DIO_handle,TC_EVENT,a)
    #undef  TRACE_EVENT_P1
    #define TRACE_EVENT_P1(f,a1) vsi_o_ttrace(GDD_DIO_handle,TC_EVENT,f,a1)  
    #undef  TRACE_EVENT_P2
    #define TRACE_EVENT_P2(f,a1,a2) vsi_o_ttrace(GDD_DIO_handle,TC_EVENT,f,a1,a2)  
    #undef  TRACE_BINDUMP
    #define TRACE_BINDUMP(s,m,d,p,l) {vsi_o_ttrace(GDD_DIO_handle,m,d);vsi_o_primsend(GDD_DIO_handle,m,0,FRM_PCO_NAME,BIN_TRACE_OPC,p,l FILE_LINE_MACRO);}
  #else /* _SIMULATION_ */
    EXTERN  T_HANDLE aci_handle;  /* include of aci.h needs a lot of additional includes */
    #undef  TRACE_BINDUMP
    #define TRACE_BINDUMP(s,m,d,p,l) {vsi_o_ttrace(aci_handle,m,d);vsi_o_primsend(aci_handle,m,0,FRM_PCO_NAME,BIN_TRACE_OPC,p,l FILE_LINE_MACRO);}
  #endif /* _SIMULATION_ */
  #define BAT_TRACE_ERROR(a)              TRACE_ERROR(a)
  #define BAT_TRACE_FUNCTION(a)           TRACE_FUNCTION(a)
  #define BAT_TRACE_EVENT(a)              TRACE_EVENT(a)
  #define BAT_TRACE_EVENT_P1(f,a1)        TRACE_EVENT_P1(f,a1)
  #define BAT_TRACE_EVENT_P2(f,a1,a2)     TRACE_EVENT_P2(f,a1,a2)
  #define BAT_TRACE_BINDUMP(s,m,d,p,l)    TRACE_BINDUMP(s,m,d,p,l)
#else /* NEW_FRAME */
  #define BAT_TRACE_ERROR(a)              ((void)(0))
  #define BAT_TRACE_FUNCTION(a)           ((void)(0))
  #define BAT_TRACE_EVENT(a)              ((void)(0))
  #define BAT_TRACE_EVENT_P1(f,a1)        ((void)(0))
  #define BAT_TRACE_EVENT_P2(f,a1,a2)     ((void)(0))
  #define BAT_TRACE_BINDUMP(s,m,d,p,l)    ((void)(0))
  typedef int T_HANDLE;  /* in case of BAT Lib dos not run under GPF */
#endif /* NEW_FRAME */


/*==== GLOBAL VARS =================================================================*/

GLOBAL void bat_change_client_state   (T_BAT_client_maintain   *client,
                                       T_BAT_client_state       new_state);
GLOBAL void bat_change_instance_state (T_BAT_instance_maintain *instance, 
                                       T_BAT_instance_state     new_state);
GLOBAL void bat_change_buffer_state   (T_BAT_instance_maintain *instance, 
                                       T_BAT_buf_state          new_state);

T_BAT_return  bat_init_global_params                (void);
T_BAT_return  bat_deinit_global_params              (void);
T_BAT_return  bat_init_instance_pointer             (T_BAT_instance inst_hndl, 
                                                     T_BAT_instance_maintain *inst_mt);
T_BAT_return  bat_deinit_instance_pointer           (T_BAT_instance inst_hndl);

BOOL          bat_check_init_or_deinit              (BOOL is_init);
BOOL          bat_check_if_all_instances_are_closed (void);
BOOL          bat_check_if_all_clients_are_closed   (T_BAT_instance inst_hndl);

T_BAT_return bat_get_new_instance                   (T_BAT_instance *instance);
T_BAT_return bat_delete_instance                    (T_GDD_CON_HANDLE con_handle);
T_BAT_return bat_get_instance_from_gdd_handle       (T_GDD_CON_HANDLE con_handle,
                                                     T_BAT_instance *instance_handle,
                                                     T_BAT_instance_maintain **instance);
T_BAT_return bat_get_instance_from_client_handle    (T_BAT_client client_handle, 
                                                     T_BAT_instance_maintain **instance);
T_BAT_return bat_get_new_client                     (T_BAT_instance instance, 
                                                     T_BAT_client *client);
T_BAT_return bat_init_new_client                    (T_BAT_client client, 
                                                     int(*response_cb)( T_BAT_client client, 
                                                                        T_BAT_cmd_response *rsp),
                                                     void(*signal_cb)(  T_BAT_client client, 
                                                                        T_BAT_signal));

T_BAT_return bat_make_client_handle_from_gdd_and_client_id 
                                                    (T_GDD_CON_HANDLE con_handle, 
                                                     U8 client_id, 
                                                     T_BAT_client *client);

T_BAT_return bat_get_client_from_client_handle      (T_BAT_client client, 
                                                     T_BAT_client_maintain **client_maintain);

T_BAT_return bat_response_rcv                       (T_BAT_instance inst_hndl, 
                                                     T_BAT_cmd_response *rsp);
T_BAT_return bat_unsolicited_code_rcv               (T_BAT_instance inst_hndl, 
                                                     T_BAT_cmd_response *rsp);
T_BAT_return bat_get_instance_from_instance_handle  (T_BAT_instance instance, 
                                                     T_BAT_instance_maintain **inst_mt);
T_BAT_return bat_control_confirm_rcv                (T_BAT_instance inst_hndl, 
                                                     T_BATC_confirm cnf);
T_BAT_return bat_command_response_rcv               (T_BAT_instance inst_hndl, 
                                                     U8 clnt_id, 
                                                     T_BAT_cmd_response *rsp);
T_BAT_return bat_send_cmd_data                      (T_BAT_client clnt_hndl,  
                                                     T_BAT_cmd_send *data);
T_BAT_return bat_send_ctrl_data                     (T_BAT_instance inst_hndl, 
                                                     T_BATC_signal *data);

/* callbacks for GDD */

void        bat_gdd_signal_cb         (T_GDD_CON_HANDLE con, T_GDD_SIGNAL signal);
GDD_RESULT  bat_gdd_receive_data_cb   (T_GDD_CON_HANDLE con, T_GDD_BUF *buf);

BOOL bat_l2p_receive (U8 inst_hndl, T_GDD_BUF *buf);


/* callbacks for L2P */
void *bat_l2p_get_tx_buffer(U8 inst_hndl, U16 data_size, 
                            void **seg_hdr_ptr,  U16 *total_size, U16 *seg_size);
int bat_l2p_send_frame(U8 inst_hndl);
void *bat_l2p_get_rx_buffer(U8 inst_hndl);
void bat_l2p_message_rxd (U8 inst_hndl, U8 client_id, U32 data_tag, 
                          void *data_ptr, U16 data_size);

void *bat_l2p_get_next_buf_seg (U8 inst_hndl, void *seg_hdr, void **seg_hdr_ptr,  U16 *segSize);

#endif /* BAT_INTERN_H */



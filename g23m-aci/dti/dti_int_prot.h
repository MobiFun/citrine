/*
+-------------------------------------------------------------------+
| PROJECT:  DTILIB                   $Workfile:: dti.h             $|
| $Author:: xof                      $$Revision::                  $|
| CREATED:  15-Feb-2001              $Modtime::                    $|
+-------------------------------------------------------------------+

   MODULE  : DTI

   PURPOSE : Definitions for the internal Data Transmission Interface
             Library, DTI.

   $Log:: /GSM/Condat/MS/SRC/dti/dti.h                                $
*/
/*
 *  Version 1.6
 */

#ifndef DTI_INT_PROT_H
#define DTI_INT_PROT_H


/*==== Prototypes ==================================================*/

EXTERN void set_default_para_data_base(DTI_HANDLE hDTI);

EXTERN void set_default_para_link_table(
  DTI_HANDLE hDTI,
  DTI_LINK *link,
  U32 link_id,
  U8 direction
  );

EXTERN void init_link_table(DTI_LINK *link);

EXTERN void set_reset_req_para_link_table ( DTI_HANDLE hDTI, DTI_LINK *link);

EXTERN void set_open_para_link_table
  (
  DTI_HANDLE hDTI,
  DTI_LINK *link,
  U32   version,
  U32   link_options,
  U8    instance,
  U8    interfac,
  U8    channel,
  U8    queue_size,
  U8    connect_state
  );

EXTERN void free_dti_link_structure(DTI_HANDLE hDTI);

EXTERN void trace_message( T_HANDLE handle, char *str, U32 entity_options);

EXTERN void trace_message_link_id (T_HANDLE handle, char *str, U32 link_id, U32 entity_options);

EXTERN void trace_message_l_dl (T_HANDLE handle,
                                char *str,
                                U32 link_id,
                                U16 length,
                                U32 entity_options);

EXTERN void trace_message_l_e (T_HANDLE handle, char *str, U32 link_id, char* entity, U32 entity_options);

EXTERN void trace_message_iic (T_HANDLE handle, char *str, U8 instance, U8
                                                interfac, U8 channel, U32
                                                entity_options);

EXTERN void trace_function( T_HANDLE handle, char *str, U32 entity_options);

EXTERN DTI_LINK * get_pointer_link_table
  (
  DTI_HANDLE hDTI,
  U32 select_link_id,
  U8 select_direction
  );

EXTERN DTI_LINK * get_pointer_link_table_channel
  (
  DTI_HANDLE hDTI,
  U8 instance,
  U8 interfac,
  U8 channel
  );

EXTERN DTI_LINK * get_pointer_free_link(DTI_HANDLE hDTI);

EXTERN DTI_RESULT validate_open_parameters
  (
  DTI_HANDLE hDTI,
  U32 link_id,
  U8 *queue_size,
  U8 *direction,
  U32 *link_options,
  U32 version,
  U8 *neighbor_entity
  );

EXTERN void open_comm_channel(T_HANDLE entity_handle, T_HANDLE *com_handle,
                              char *name, U32 entity_options);

EXTERN BOOL check_dti_version (DTI_HANDLE hDTI, U32 request_version);

EXTERN void close_link_with_signal(DTI_HANDLE hDTI, DTI_LINK *link);

EXTERN void flow_control_prim_received
  (
  DTI_HANDLE hDTI,
  U32 link_id,
  U8 direction
  );
EXTERN void connect_init_prim_received
  (
  DTI_HANDLE hDTI,
  U32 link_id,
  U32 version,
  U8 direction
  );
EXTERN void connect_confirm_prim_received
  (
  DTI_HANDLE hDTI,
  U32 link_id,
  U32 version,
  U8 direction
  );
EXTERN void disconnect_prim_received
  (
  DTI_HANDLE hDTI,
  U32 link_id,
  U8 direction
  );
EXTERN void data_prim_received
  (
  DTI_HANDLE hDTI,
  T_DTI2_DATA_IND *dti_data_ind,
  U8 direction
  );
EXTERN void data_test_prim_received
  (
  DTI_HANDLE hDTI,
  T_DTI2_DATA_TEST_IND *dti_data_test_ind,
  U8 direction
  );

EXTERN void put_dti_data_ind_in_queue
  (
  DTI_HANDLE hDTI,
  DTI_LINK *link,
  T_DTI2_DATA_IND *dti_data_ind
  );

EXTERN void put_dti_data_ind_in_queue_managed
  (
  DTI_HANDLE hDTI,
  DTI_LINK *link,
  T_DTI2_DATA_IND *dti_data_ind
  );

EXTERN T_DTI2_DATA_IND * get_dti_data_ind_from_queue( DTI_HANDLE hDTI, DTI_LINK * link);

#ifdef MEMORY_SUPERVISION
EXTERN SHORT vsi_c_psend_ntrace ( T_HANDLE Caller, T_HANDLE ComHandle,
                           T_VOID_STRUCT *ptr, ULONG MsgLen, const char *file, int line );
#else  /* MEMORY_SUPERVISION */
EXTERN SHORT vsi_c_psend_ntrace ( T_HANDLE Caller, T_HANDLE ComHandle,
                           T_VOID_STRUCT *ptr, ULONG MsgLen );
#endif /* MEMORY_SUPERVISION */

#endif /* DTI_INT_PROT_H */

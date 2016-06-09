/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_DYN_DWL_PROTO.H
 *
 *        Filename l1_dyn_dwl_proto.h
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#if (L1_DYN_DSP_DWNLD == 1)

#ifndef _L1_DYN_DWL_PROTO_H_
#define _L1_DYN_DWL_PROTO_H_

/* Functions declared in l1_dyn_dwl_apihisr.c */
void   l1_dyn_dwnld_apihisr(void);
BOOL   l1_dyn_dwnld_apihisr_error_handler(void);
UWORD8 l1_dyn_dwnld_copy_patch_process (BOOL new_patch);


/* Functions declared in l1_dyn_dwl_afunc.c*/
void    l1_handle_particular_cases_for_TCS211(UWORD32 msg_code, BOOL *return_flag);
BOOL    l1_does_the_incoming_primitive_trigger_dynamic_dwnld(UWORD32 msg_code, BOOL delay_flag);
UWORD16 l1_lookup_primitive_patch_matrix(UWORD32 msg_code, UWORD16* patch_id_p);
BOOL    l1_is_patch_already_installed (UWORD16 patch_id);
BOOL    l1_is_patch_id_in_uninstall_set (UWORD16 patch_id, UWORD16 *uninstall_patch_vect, UWORD16 num_of_uninstall_elem);
BOOL    l1_manage_patch_incompatibilty(UWORD16 num_of_patch_id_to_dwnld, UWORD16 *num_of_uninstall_elem, UWORD16 *uninstall_patch_id_vect);
void    l1_update_semaphores_for_all_state_machines(UWORD16 num_of_patch_id_to_dwnld);
void    l1_set_semaphores_for_all_state_machines_involved(UWORD16 delay_num_patch, UWORD16* delay_patch_array);
void    l1_reset_semaphores();
void    l1_disable_DSP_trace();
void    l1_enable_DSP_trace();
BOOL    l1_pop_Primitive(UWORD32 *p_primitive);
UWORD32 l1_check_Fifo_Primitive();
BOOL    l1_push_Primitive(UWORD32 primitive_id);

/* Functions declared in l1_dyn_dwl_async.c */
BOOL    l1_dynamic_download_manager(UWORD32 SignalCode, BOOL delay_flag);

/* Functions declared in l1_dyn_dwl_func.c*/

void l1_set_uninstall_parameters(void);
BOOL    l1_set_dyn_dwnld_install_vect(UWORD16* size_p, UWORD32* dest_addr, UWORD16* crc_vect, const UWORD8 *patch_array_code, UWORD16 i);
UWORD16 l1_initialize_patch_parameters();
void    l1_initialize_pointers_for_copy(UWORD16 **pp_dest_mcu, UWORD16 **pp_src_mcu);
void    l1_copy_till_the_end_of_the_patch_and_update_write_pointer(UWORD16 tmp_patch_size, UWORD16* p_dest_mcu, UWORD16* p_src_mcu);
void    l1_copy_till_end_of_dwnld_area_and_update_write_pointer(UWORD16 tmp_dwnld_area_size,UWORD16 *p_dest_mcu, UWORD16 *p_tmp_patch_size, UWORD16 **pp_src_mcu);
BOOL    l1_copy_first_N_words (UWORD16 *dwnld_area_size_p, UWORD16 *patch_area_size_p, UWORD16 **pp_dest_mcu, UWORD16 **pp_src_mcu);
UWORD16 l1_initialize_download_area_parameters();
BOOL    l1_init_pointers_and_copy_first_block_of_data(UWORD16 *dwnld_area_size_p, UWORD16 *patch_size_p, UWORD16 **pp_dest_mcu, UWORD16 **pp_src_mcu, BOOL new_patch);

/* Functions declared in l1_dyn_dwl_init.c*/

void l1_dyn_dwnld_initialize_var();
void l1_dyn_dwnld_reset();
void l1_dyn_dwnld_reset_api();


#endif    // _L1_DYN_DWL_PROTO_H_
#endif    // L1_DYN_DSP_DWNLD

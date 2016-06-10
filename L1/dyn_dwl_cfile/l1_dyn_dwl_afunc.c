/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_DYN_DWL_AFUNC.C
 *
 *        Filename l1_dyn_dwl_afunc.c
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#include "l1_confg.h"
#include "l1_types.h"
#include "sys_types.h"
#include "cust_os.h"
#include "l1_macro.h"
#include "l1_const.h"
#if TESTMODE
  #include "l1tm_defty.h"
#endif
#if (AUDIO_TASK == 1)
  #include "l1audio_const.h"
  #include "l1audio_cust.h"
  #include "l1audio_defty.h"
  #include "l1audio_signa.h"
#endif
#if (L1_GTT == 1)
  #include "l1gtt_const.h"
  #include "l1gtt_defty.h"
  #include "l1gtt_signa.h"
#endif
#if (L1_DYN_DSP_DWNLD == 1)
  #include "l1_dyn_dwl_msgty.h"
  #include "l1_dyn_dwl_defty.h"
  #include "l1_dyn_dwl_proto.h"
  #include "l1_dyn_dwl_const.h"
#endif //L1_DYN_DSP_DWNLD
#if (L1_MP3 == 1)
  #include "l1mp3_signa.h"
  #include "l1mp3_defty.h"
#endif //L1_MP3
#if (L1_MIDI == 1)
  #include "l1midi_defty.h"
#endif
#if (L1_AAC == 1)
  #include "l1aac_signa.h"
  #include "l1aac_defty.h"
#endif //L1_AAC
#include "l1_defty.h"
#include "l1_varex.h"
#include "l1_msgty.h"
#include "l1_proto.h"
#include "l1_signa.h"
#include <string.h>
#include <stdio.h>


#if (TRACE_TYPE == 1) ||(TRACE_TYPE == 4) || (TRACE_TYPE == 7) || (TESTMODE)
  #include "l1_trace.h"
#endif

#if (L1_DYN_DSP_DWNLD == 1)

extern const BOOL primitives_to_patch_matrix[][MAX_NUM_OF_PATCH_IDS];
extern const BOOL incompatibility_matrix[][MAX_NUM_OF_PATCH_IDS];
extern const BOOL semaphore_matrix[][MAX_NUM_OF_SEMAPHORES];

/* table reconstructed from disassembly of TCS211 binary object */
const T_SIGNAL_PATCH signal_patch_array[NUM_OF_DYN_DWNLD_PRIMITIVES] =
{
	{L1C_STOP_DEDICATED_DONE,			0},
    #if (L1_GTT == 1)
	{MMI_GTT_START_REQ,				1},
    #endif
	{MPHC_IMMED_ASSIGN_REQ,				2},
    #if (MELODY_E2 == 1)
	{MMI_MELODY0_E2_START_REQ,			3},
	{MMI_MELODY1_E2_START_REQ,			4},
	{L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON,	5},
    #endif
};


/*
 * LoCosto version of L1 has a function called l1_check_flag_for_download_area()
 * here.  TCS211 binary object has l1_handle_particular_cases_for_TCS211()
 * instead.  We are going to reconstruct the TCS211 function based on
 * disassembly, using the LoCosto version as our starting point.
 */

void l1_handle_particular_cases_for_TCS211(UWORD32 msg_code, BOOL *return_flag)
{
  switch(msg_code)
  {
    /* from disassembly */
    case MPHC_IMMED_ASSIGN_REQ:				/* 0x0033 */
	/* code at 0x8e */
	l1a.dyn_dwnld.dedicated_stop_flag = FALSE;
	return;
    case L1C_STOP_DEDICATED_DONE:			/* 0x0081 */
	/* code at 0x86 */
	l1a.dyn_dwnld.dedicated_stop_flag = TRUE;
	return;
  #if (MELODY_E2 == 1)
    case MMI_MELODY0_E2_START_REQ:			/* 0x182E */
	/* code at 0x6c */
	l1a.dyn_dwnld.melody0_E2_flag_activated = TRUE;
	if (l1a.dyn_dwnld.melody1_E2_flag_activated == TRUE) {
		/* code at 0x80 */
		*return_flag = FALSE;
		return;
	}
	return;
    case MMI_MELODY1_E2_START_REQ:			/* 0x1832 */
	/* code at 0x56 */
	l1a.dyn_dwnld.melody1_E2_flag_activated = TRUE;
	if (l1a.dyn_dwnld.melody0_E2_flag_activated == TRUE) {
		/* code at 0x80 */
		*return_flag = FALSE;
		return;
	}
	return;
    case L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON:	/* 0x1866 */
	/* code at 0x3e */
	if (l1a.dyn_dwnld.melody0_E2_flag_activated == TRUE ||
	    l1a.dyn_dwnld.melody1_E2_flag_activated == TRUE) {
		/* code at 0x80 */
		*return_flag = FALSE;
		return;
	}
	return;
    case L1_MELODY0_E2_STOP_CON:			/* 0x1867 */
	/* code at 0x36 */
	l1a.dyn_dwnld.melody0_E2_flag_activated = FALSE;
	return;
    case L1_MELODY1_E2_STOP_CON:			/* 0x1868 */
	/* code at 0x2e */
	l1a.dyn_dwnld.melody1_E2_flag_activated = FALSE;
	return;
  #endif
    default:
	return;
  }

}
/*------------------------------------------------------------------------------------------------------------------- */
/* l1_does_the_incoming_primitive_trigger_dynamic_dwnld                                                               */
/*------------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                    */
/* Parameters : Signal Code of the message, delay flag                                                                */
/*                                                                                                                    */
/* Return     :    TRUE if the primitive triggers a dynamic download, FALSE in the other case                         */
/*                                                                                                                    */
/* Description :  Check if the primitive triggers a dynamic download; if yes it return TRUE, otherwise FALSE          */
/*                                                                                                                    */
/*                                                                                                                    */
/*------------------------------------------------------------------------------------------------------------------- */

BOOL l1_does_the_incoming_primitive_trigger_dynamic_dwnld(UWORD32 msg_code, BOOL delay_flag)
{
  BOOL return_flag = FALSE;
  UWORD16 i;
  for (i=0;i<l1a.dyn_dwnld.num_of_primitives;i++)
  {
    if(msg_code == signal_patch_array[i].primitive)
      return_flag = TRUE;

  }

  l1_handle_particular_cases_for_TCS211(msg_code, &return_flag);

  return return_flag;
}

/*----------------------------------------------------------------------------------------   */
/* l1_lookup_primitive_patch_matrix                                                          */
/*----------------------------------------------------------------------------------------   */
/*                                                                                           */
/* Parameters : Signal Code of the message, patch_id vector                                  */
/*                                                                                           */
/* Return     :    number of patches to download, patch ids vectore (by reference)           */
/*                                                                                           */
/* Description :  Retrieves the patch ids to be downloaded and number of patches to download */
/*                     Patch ID is retrieved via look-up in primitives to patch ID matrix    */
/*                                                                                           */
/*----------------------------------------------------------------------------------------   */

UWORD16 l1_lookup_primitive_patch_matrix(UWORD32 msg_code, UWORD16* patch_id_p)
{
  UWORD16 j;
  UWORD16 counter=0;
  UWORD16 msg_code_id;

  // Check if one of the messages belongs to the set of primitves which triggers a dynamic download and saves its index

#if 0	// LoCosto-ism not present in TCS211
 if (!((l1a.state[L1A_GTT_STATE] == 0) || (l1a.state[L1A_GTT_STATE] == 1))) // TTY ON
  {
      if(msg_code == MMI_TCH_VOCODER_CFG_REQ) // AND Current_message == VOCODER
      	{
      	    // Dont do anything
      	    return 0;
      	}
  }
#endif
  
  for (j=0;j<NUM_OF_DYN_DWNLD_PRIMITIVES;j++)
  {
    if (msg_code == signal_patch_array[j].primitive)
      msg_code_id = signal_patch_array[j].msg_id;
  }


  // Compute the number of elements to be downloaded and their ids
  for (j=0; j<MAX_NUM_OF_PATCH_IDS; j++)
  {
    patch_id_p[j]=0xFFFF; //omaps00090550 ;
    if(primitives_to_patch_matrix[msg_code_id][j] == 1)
    {
      patch_id_p[counter++]=j;
     }
  }
  return counter;
}

/*------------------------------------------------------------------------- */
/* l1_is_patch_already_installed                                            */
/*------------------------------------------------------------------------- */
/*                                                                          */
/* Parameters : patch id                                                    */
/*                                                                          */
/* Return     :    TRUE patch is already installed, FALSE otherwise         */
/*                                                                          */
/* Description :  Check if the selected patch id is already installed or not*/
/*                                                                          */
/*                                                                          */
/*------------------------------------------------------------------------- */

BOOL l1_is_patch_already_installed (UWORD16 patch_id)
{
  UWORD16 i=0;
  while(i<l1a.dyn_dwnld.num_patches_installed)
  {
    if(l1a.dyn_dwnld.patch_id[i]==patch_id)
      break;
    else
      i++;
  }
  if(i<l1a.dyn_dwnld.num_patches_installed)
    return TRUE;
  else
    return FALSE;
}

/*-----------------------------------------------------------------------------  */
/* l1_is_patch_id_in_uninstall_set                                               */
/*-----------------------------------------------------------------------------  */
/*                                                                               */
/* Parameters : patch id, uninstall address vector, num of uninstall elements    */
/*                                                                               */
/* Return     :    TRUE patch is in the set, FALSE otherwise                     */
/*                                                                               */
/* Description :  Check if the selected patch id is in the uninstall set or not  */
/*                                                                               */
/*                                                                               */
/*-----------------------------------------------------------------------------  */

BOOL l1_is_patch_id_in_uninstall_set (UWORD16 patch_id, UWORD16 *uninstall_patch_vect, UWORD16 num_of_uninstall_elem)
{
  UWORD16 i;
  for (i=0;i<num_of_uninstall_elem;i++)
  {
    if(patch_id==uninstall_patch_vect[i])
      return TRUE;
  }
  return FALSE;
}

/*--------------------------------------------------------------------------------------------------------------------------------------- */
/* l1_manage_patch_incompatibilty                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                                        */
/* Parameters :    num of patch to download, num of uninstalled elements (pointer), uninstall vect                                        */
/* Return     :    TRUE if there is some patch to uninstall, FALSE otherwise. By reference, num of patches to uninstall and their IDs     */
/*                                                                                                                                        */
/* Description :  Check if some patch must be uninstalled: in case, retrieves the number of patch to uninstall and their IDs              */
/*                                                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------------------------- */

BOOL l1_manage_patch_incompatibilty(UWORD16 num_of_patch_id_to_dwnld, UWORD16 *num_of_uninstall_elem, UWORD16 *uninstall_patch_id_vect)
{
  UWORD16 i,j;
  UWORD16 patch_id;

  // Reset values passed by reference
  *num_of_uninstall_elem=0;
  for(j=0;j<MAX_NUM_OF_PATCH_IDS;j++)
    uninstall_patch_id_vect[j]=0xFFFF; //omaps00090550 ;

   // First cycle: for all the patches that must be downloaded
  for(i=0;i<num_of_patch_id_to_dwnld;i++)
  {
    patch_id=l1a.dyn_dwnld.next_patch_id[i];
    // Check all incompatibilities
    for(j=0;j<MAX_NUM_OF_PATCH_IDS;j++)
    {
      // If patch ID j is incompatible with selected patch id that must be downloaded
      if(incompatibility_matrix[patch_id][j]==1)
      {
        // If patch j is already installed and not yet in the uninstall set
        if((l1_is_patch_already_installed(j) == TRUE) && (l1_is_patch_id_in_uninstall_set(j,uninstall_patch_id_vect,(*num_of_uninstall_elem))==FALSE))
        {
          // Add patch j in the uninstall set and increase number of uninstall elements
          uninstall_patch_id_vect[*num_of_uninstall_elem]=j;
          (*num_of_uninstall_elem)++;
        }
    }
    }
  }
  if(*num_of_uninstall_elem==0)
    return TRUE;
  else
    return FALSE;
}

/*----------------------------------------------------------------- */
/* l1_set_semaphores_for_all_state_machines_involved                */
/*----------------------------------------------------------------- */
/*                                                                  */
/* Parameters :  num of patches to download,                        */
/* Return     :   none                                              */
/*                                                                  */
/* Description :  Set semaphores for the state machines impacted    */
/*                                                                  */
/*                                                                  */
/*----------------------------------------------------------------- */

void l1_set_semaphores_for_all_state_machines_involved(UWORD16 num_of_patch_id_to_dwnld, UWORD16* next_patch_id)
{
  UWORD16 i,j, patch_id;
  for (i=0;i<num_of_patch_id_to_dwnld;i++)
  {
    patch_id = next_patch_id[i];
    for(j=0;j<MAX_NUM_OF_SEMAPHORES;j++)
    {
      if(semaphore_matrix[patch_id][j]==1)
      {
        l1a.dyn_dwnld.semaphore_vect[j]=RED;
      }
    }
  }
}

/*----------------------------------------------------------------- */
/* l1_update_semaphores_for_all_state_machines                      */
/*----------------------------------------------------------------- */
/*                                                                  */
/* Parameters :  num of patches to download,                        */
/* Return     :   none                                              */
/*                                                                  */
/* Description :  Update semaphores for the state machines impacted */
/*                (DELAY case)                                      */
/*                                                                  */
/*----------------------------------------------------------------- */

void l1_update_semaphores_for_all_state_machines(UWORD16 num_of_patch_id_to_dwnld)
{
  /* In case of delay the patch which is DELAYED may have been be already installed*/
  /* The semaphores that have been raised during the previous dynamic download */
  /* for that patch must be updated */

  UWORD16 i,j, patch_id;
  for(j=0;j<MAX_NUM_OF_SEMAPHORES;j++)
    l1a.dyn_dwnld.semaphore_vect[j] = GREEN;
  for (i=0;i<num_of_patch_id_to_dwnld;i++)
  {
    patch_id = l1a.dyn_dwnld.next_patch_id[i];
    for(j=0;j<MAX_NUM_OF_SEMAPHORES;j++)
    {
      if(semaphore_matrix[patch_id][j]==1)
      {
        l1a.dyn_dwnld.semaphore_vect[j]=RED;
      }
    }
  }
}

/*-------------------------------------------------------*/
/* l1_reset_semaphores                                   */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters :  none                                    */
/* Return     :                                          */
/*                                                       */
/* Description :  Reset semaphores only for the state machines related to the currently downloaded patches                      */
/*                     In fact, because of delay, some semaphores can be set for patch that will be downloaded in the follwoing step                                  */
/*                                                       */
/*-------------------------------------------------------*/
void l1_reset_semaphores()
{
  UWORD16 i,j, patch_id;
  for (i=0;i<l1a.dyn_dwnld.num_of_elem_to_copy;i++)
  {
    patch_id = l1a.dyn_dwnld.next_patch_id[i];
    for(j=0;j<MAX_NUM_OF_SEMAPHORES;j++)
    {
      if(semaphore_matrix[patch_id][j]==1)
      {
        l1a.dyn_dwnld.semaphore_vect[j]=GREEN;
      }
    }
  }
}

/*
 * TCS211 had l1_disable_DSP_trace() and l1_enable_DSP_trace() functions
 * implemented here in the l1_dyn_dwl code.  The LoCosto version of L1
 * has l1_{dis,en}able_dsp_trace() instead (note the case difference)
 * implemented in the l1_trace.c module.  The LoCosto versions of these
 * functions implement more complex logic with nesting, and use more
 * state variables than are provided in TCS211 L1 data structures which
 * we cannot change at this early phase of deblobbing.  Therefore,
 * we are going to recreate the simpler logic of the older
 * l1_{dis,en}able_DSP_trace() functions from disassembly.
 */

void l1_disable_DSP_trace()
{
#if (CODE_VERSION != SIMULATION)
  T_NDB_MCU_DSP* dsp_ndb_ptr = (T_NDB_MCU_DSP *) NDB_ADR;
#else
  T_NDB_MCU_DSP* dsp_ndb_ptr = l1s_dsp_com.dsp_ndb_ptr;
#endif

  if (dsp_ndb_ptr->d_debug_trace_type != 0x0000)
  {
    l1a.dyn_dwnld.dsp_trace_level_copy = dsp_ndb_ptr->d_debug_trace_type;
    dsp_ndb_ptr->d_debug_trace_type = (API)0x8000;	/* 0x9000 in LoCosto */
    l1a.dyn_dwnld.trace_flag_blocked = TRUE;
  }
}

void l1_enable_DSP_trace()
{
#if (CODE_VERSION != SIMULATION)
  T_NDB_MCU_DSP* dsp_ndb_ptr = (T_NDB_MCU_DSP *) NDB_ADR;
#else
  T_NDB_MCU_DSP* dsp_ndb_ptr = l1s_dsp_com.dsp_ndb_ptr;
#endif

  if ((l1a.dyn_dwnld.trace_flag_blocked == TRUE) && (dsp_ndb_ptr->d_debug_trace_type == 0x0000))
  {
    l1a.dyn_dwnld.trace_flag_blocked = FALSE;

    dsp_ndb_ptr->d_debug_trace_type = (API)l1a.dyn_dwnld.dsp_trace_level_copy | 0x8000;
    l1a.dyn_dwnld.dsp_trace_level_copy = 0x0000;
  }
}

/*---------------------------------------------------------- */
/* l1_push_Primitive                                         */
/*---------------------------------------------------------- */
/*                                                           */
/* Parameters :  primitive ID                                */
/* Return     :  TRUE if push is successful, FALSE otherwise */
/*                                                           */
/* Description :  Push primitive ID into DELAY FIFO          */
/*                                                           */
/*                                                           */
/*---------------------------------------------------------- */

BOOL l1_push_Primitive(UWORD32 primitive_id)
{
  UWORD32 num_elem = l1a.dyn_dwnld.waiting_patch_fifo.num_of_elem;
  BOOL return_flag = TRUE;

  if(num_elem >= MAX_NUM_OF_PATCH_IDS)
  {
    return_flag = FALSE;
  }
  else
  {
    l1a.dyn_dwnld.waiting_patch_fifo.signal_code_vect[num_elem++] = primitive_id;
    l1a.dyn_dwnld.waiting_patch_fifo.num_of_elem = num_elem;
  }
  return return_flag;
}

/*---------------------------------------------------------- */
/* l1_pop_Primitive                                          */
/*---------------------------------------------------------- */
/*                                                           */
/* Parameters :  primitive ID (pointer)                      */
/* Return     :  TRUE if pop is successful, FALSE otherwise  */
/*                                                           */
/* Description :  Pop primitive from DELAY FIFO              */
/*                                                           */
/*                                                           */
/*---------------------------------------------------------- */

BOOL l1_pop_Primitive(UWORD32 *p_primitive)
{
  UWORD32 primitive_id;
  UWORD8 ind;
  UWORD32 num_elem = l1a.dyn_dwnld.waiting_patch_fifo.num_of_elem;
  BOOL return_flag = TRUE;

  if(num_elem <= 0)
  {
    return_flag = FALSE;
  }
  else
  {
    primitive_id = l1a.dyn_dwnld.waiting_patch_fifo.signal_code_vect[0];
    for (ind = 0;ind<num_elem-1;ind++)
      l1a.dyn_dwnld.waiting_patch_fifo.signal_code_vect[ind] = l1a.dyn_dwnld.waiting_patch_fifo.signal_code_vect[ind+1];
    l1a.dyn_dwnld.waiting_patch_fifo.signal_code_vect[num_elem-1] = 0;
    l1a.dyn_dwnld.waiting_patch_fifo.num_of_elem = num_elem-1;
    *p_primitive = primitive_id;
  }
  return return_flag;
}
/*---------------------------------------------------------- */
/* l1_check_Fifo_Primitive                                   */
/*---------------------------------------------------------- */
/*                                                           */
/* Parameters :  none                                        */
/* Return     :  Number of elements in the FIFO              */
/*                                                           */
/* Description :  Return number of elements in the FIFO      */
/*                                                           */
/*                                                           */
/*---------------------------------------------------------- */

UWORD32 l1_check_Fifo_Primitive()
{
  return l1a.dyn_dwnld.waiting_patch_fifo.num_of_elem;
}
#endif // L1_DYN_DSP_DWNLD == 1

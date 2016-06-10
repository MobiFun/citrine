/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_DYN_DWL_INIT.C
 *
 *        Filename l1_dyn_dwl_init.c
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/
#include "config.h"
#include "l1_confg.h"
#include "sys_types.h"
#include "../../riviera/rv/rv_general.h"
#include "../../nucleus/nucleus.h"
#include "l1_types.h"
#include "l1audio_const.h"
#include "l1audio_cust.h"
#include "l1audio_defty.h"
#include "l1_const.h"
#include "l1tm_defty.h"

#if (L1_GTT == 1)
  #include "l1gtt_const.h"
  #include "l1gtt_defty.h"
#endif
#if (L1_DYN_DSP_DWNLD == 1)
 #include "l1_dyn_dwl_const.h"
 #include "l1_dyn_dwl_defty.h"
 #include "l1_dyn_dwl_proto.h"
#endif

#if (L1_MP3 == 1)
  #include "l1mp3_defty.h"
#endif //L1_MP3
#if (L1_MIDI == 1)
  #include "l1midi_defty.h"
#endif
#if (L1_AAC == 1)
  #include "l1aac_defty.h"
#endif //L1_AAC

#include "l1_defty.h"
#include "../../gpf/inc/cust_os.h"
/* #include "nu_main.h" */
#include "l1audio_signa.h"
#include "l1_varex.h"
#include "l1_macro.h"
#include "l1_trace.h"

#if (L1_DYN_DSP_DWNLD == 1)
extern T_DYN_DWNLD_MCU_DSP *dyn_dwl_ndb;

/* reconstructing from disassembly of TCS211 binary object */

/* patch ID mapping:
   0 = GPRS
   1 = AMR_SCH
   2 = TTY
   3 = AMR_MMS
   4 = E2
*/

const BOOL  primitives_to_patch_matrix[NUM_OF_DYN_DWNLD_PRIMITIVES][MAX_NUM_OF_PATCH_IDS] = {

/* L1C_STOP_DEDICATED_DONE			*/	{1, 0, 0, 0, 0},
/* MMI_GTT_START_REQ (?)			*/	{0, 0, 1, 0, 0},
/* MPHC_IMMED_ASSIGN_REQ			*/	{0, 1, 0, 0, 0},
/* MMI_MELODY0_E2_START_REQ			*/	{0, 0, 0, 0, 1},
/* MMI_MELODY1_E2_START_REQ			*/	{0, 0, 0, 0, 1},
/* L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON	*/	{0, 0, 0, 1, 0}

};
  
const BOOL incompatibility_matrix[MAX_NUM_OF_PATCH_IDS][MAX_NUM_OF_PATCH_IDS] ={
	{0, 1, 1, 0, 0},
	{1, 0, 0, 0, 0},
	{0, 0, 0, 1, 1},
	{0, 0, 1, 0, 1},
	{0, 0, 1, 1, 0},
};

const BOOL semaphore_matrix[MAX_NUM_OF_PATCH_IDS][MAX_NUM_OF_SEMAPHORES] = {
	{0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 1},
	{0, 1, 0, 0, 0, 1},
	{0, 0, 0, 1, 1, 1},
	{0, 0, 1, 0, 0, 1},
};

UWORD32 dyn_dwnld_address_vect[MAX_NUM_OF_PATCH_IDS];
UWORD16 dyn_dwnld_crc_vect[MAX_NUM_OF_PATCH_IDS];
UWORD16 size_vect[MAX_NUM_OF_PATCH_IDS];

UWORD8 const *dyn_dwnld_copy_MCU_vect[MAX_NUM_OF_PATCH_IDS];

extern const UWORD8 amr_mms_patch_array[];
extern const UWORD8 amr_sch_patch_array[];
extern const UWORD8 e2_patch_array[];
extern const UWORD8 gprs_patch_array[];
extern const UWORD8 speech_acoustic_patch_array[];
extern const UWORD8 tty_patch_array[];

/*-------------------------------------------------------*/
/* l1_dyn_dwnld_initialize_var()                         */
/*-------------------------------------------------------*/
/* Parameters : none                                     */
/* Return     : nothing                                  */
/* Functionality : Initialize Dynamic Download variables */
/*                                                       */
/*-------------------------------------------------------*/

void l1_dyn_dwnld_initialize_var()
{
  UWORD16 i;

  /* Init Dynamic Download NDB */
  dyn_dwl_ndb=(T_DYN_DWNLD_MCU_DSP *)API_address_dsp2mcu(C_DYN_DWNLD_API_BASE_ADDRESS);
#if (CODE_VERSION == SIMULATION)
  {
    extern T_DYN_DWNLD_MCU_DSP dyn_dwl_ndb_sim;
    dyn_dwl_ndb=&dyn_dwl_ndb_sim;
  }
#endif // CODE_VERSION == SIMULATION

  /* Init Dynamic Download DSP background */
  l1s_dsp_com.dsp_ndb_ptr->a_background_tasks[C_BGD_DSP_DYN_DWNLD] = (API)((C_BGD_DSP_DYN_DWNLD<<11) | 0);
  l1s_dsp_com.dsp_ndb_ptr->d_background_enable&=(API)(~(1<<C_BGD_DSP_DYN_DWNLD));

  if(l1s_dsp_com.dsp_ndb_ptr->d_max_background<(C_BGD_DSP_DYN_DWNLD+1))
    l1s_dsp_com.dsp_ndb_ptr->d_max_background=(API)(C_BGD_DSP_DYN_DWNLD+1);

  dyn_dwnld_copy_MCU_vect[0] = gprs_patch_array;
  l1_set_dyn_dwnld_install_vect(size_vect,dyn_dwnld_address_vect,dyn_dwnld_crc_vect, gprs_patch_array, 0);

  dyn_dwnld_copy_MCU_vect[1] = amr_sch_patch_array;
  l1_set_dyn_dwnld_install_vect(size_vect,dyn_dwnld_address_vect,dyn_dwnld_crc_vect, amr_sch_patch_array, 1);

  #if (L1_GTT == 1)
    dyn_dwnld_copy_MCU_vect[2] = tty_patch_array;
    l1_set_dyn_dwnld_install_vect(size_vect,dyn_dwnld_address_vect,dyn_dwnld_crc_vect, tty_patch_array, 2);
  #endif

  #if (MELODY_E2 == 1)
    dyn_dwnld_copy_MCU_vect[3] = amr_mms_patch_array;
    l1_set_dyn_dwnld_install_vect(size_vect,dyn_dwnld_address_vect,dyn_dwnld_crc_vect, amr_mms_patch_array, 3);
    dyn_dwnld_copy_MCU_vect[4] = e2_patch_array;
    l1_set_dyn_dwnld_install_vect(size_vect,dyn_dwnld_address_vect,dyn_dwnld_crc_vect, e2_patch_array, 4);
  #endif

  /*---------- Set L1A globals at phone init ----------------------------*/
  l1a.dyn_dwnld.melody0_E2_flag_activated = FALSE;
  l1a.dyn_dwnld.melody1_E2_flag_activated = FALSE;
  l1a.dyn_dwnld.dedicated_stop_flag       = FALSE;

  l1a.dyn_dwnld.num_patches_installed     = 2;
  l1a.dyn_dwnld.state                     = 0;
  l1a.dyn_dwnld.dsp_trace_level_copy      = 0;

  l1a.dyn_dwnld.patch_id[0] = GPRS_PATCH;
  l1a.dyn_dwnld.patch_id[1] = AMR_MMS_PATCH;

  for (i=l1a.dyn_dwnld.num_patches_installed;i<MAX_NUM_OF_PATCH_IDS;i++)
  {  
    l1a.dyn_dwnld.patch_id[i]      = 0xFFFF; //omaps00090550 ;
    l1a.dyn_dwnld.next_patch_id[i] = 0xFFFF; //omaps00090550 ;
  }
  for (i=0;i<MAX_NUM_OF_SEMAPHORES;i++)
     l1a.dyn_dwnld.semaphore_vect[i] = GREEN;

  l1a.dyn_dwnld.trace_flag_blocked = FALSE;

  l1a.dyn_dwnld.waiting_patch_fifo.num_of_elem = 0;
  for (i=0;i<MAX_NUM_OF_PATCH_IDS;i++)
    l1a.dyn_dwnld.waiting_patch_fifo.signal_code_vect[i] = 0;

  /**************************************************/

  // Compute number of current primitives defined via compilation switch
  l1a.dyn_dwnld.num_of_primitives = 2;
  #if (L1_GTT == 1)
    l1a.dyn_dwnld.num_of_primitives++;
  #endif // L1_GTT
  #if (MELODY_E2 == 1)
    // Two primitives for MELODY E2
    // FreeCalypso: apparently there are 3 in TCS211
    l1a.dyn_dwnld.num_of_primitives += 3;
  #endif // MELODY_E2


  /*---------------------------------------------------------------------*/

  /*---------- Set L1S globals at phone init ----------------------------*/
  
  l1s.dyn_dwnld_state=0;
  
  /*---------------------------------------------------------------------*/


  /* Set API ACCESS bit to 16 bits mode */
  /* Bit 5: 0 -> 32 bits mode           */
  /*        1 -> 16 bits mode           */

#if (CODE_VERSION != SIMULATION)
  (*(volatile UWORD16 *) 0xFFFFFB0E) |= 0x0020;
#endif // CODE_VERSION != SIMULATION
}

/*-------------------------------------------------------*/
/* l1_dyn_dwl_reset()                                    */
/*                                                       */
/*-------------------------------------------------------*/
/* Parameters : none                                     */
/* Return     : nothing                                  */
/* Functionality : Reset Dynamic Download variables      */
/*                                                       */
/*-------------------------------------------------------*/

void l1_dyn_dwnld_reset()
{
  UWORD8 indx_loop;

  /* Reset L1A-L1S commands */
  l1a_l1s_com.dyn_dwnld_task.start = FALSE;
  l1a_l1s_com.dyn_dwnld_task.stop  = FALSE;
 
  /* Reset L1A-API HISR commands */

  l1a_apihisr_com.dyn_dwnld.command.restart = FALSE;
  l1a_apihisr_com.dyn_dwnld.uninstall_parameters.num_of_elem = 0;

  for (indx_loop = 0 ; indx_loop < MAX_NUM_OF_PATCH_IDS ; indx_loop++  )
  {
    l1a_apihisr_com.dyn_dwnld.uninstall_parameters.address[indx_loop]
	= 0;
    l1a_apihisr_com.dyn_dwnld.copy_parameters.start_MCU_copy_address[indx_loop]
	= 0;
    l1a_apihisr_com.dyn_dwnld.copy_parameters.address_to_install[indx_loop]
	= 0; 
    l1a_apihisr_com.dyn_dwnld.copy_parameters.size_array[indx_loop]
	= 0;
  }

  l1a_apihisr_com.dyn_dwnld.copy_parameters.size_of_dwnld_area  = 0;
  l1a_apihisr_com.dyn_dwnld.copy_parameters.start_of_dwnld_area = 0;
  l1a_apihisr_com.dyn_dwnld.copy_parameters.num_of_elem         = 0;

  /* Reset global API HISR variable */
  l1_apihisr.dyn_dwnld.state                  = 0;
  l1_apihisr.dyn_dwnld.running                = FALSE;
  l1_apihisr.dyn_dwnld.uninstall_counter      = 0;
  l1_apihisr.dyn_dwnld.patch_ids_counter      = 0;
  l1_apihisr.dyn_dwnld.running_source_pointer = 0;
  l1_apihisr.dyn_dwnld.tmp_patch_size = 0;
  
  /* Reset L1A global FIFO for handling the DELAY (when two or more primitives fall within a dynamic download time frame)*/
  l1a.dyn_dwnld.waiting_patch_fifo.num_of_elem = 0;
  for (indx_loop=0;indx_loop<MAX_NUM_OF_PATCH_IDS;indx_loop++)
  {  
    l1a.dyn_dwnld.waiting_patch_fifo.signal_code_vect[indx_loop] = 0;
    l1a.dyn_dwnld.next_patch_id[indx_loop] = 0xFFFF; //omaps00090550 ;
  }
}

/*---------------------------------------------------------------------*/
/* l1_dyn_dwnld_reset_api()                                            */
/*---------------------------------------------------------------------*/
/* Parameters  : none                                                  */
/* Return         : nothing                                            */
/* Functionality : Reset Dynamic Download API variables                */
/*                                                                     */
/*---------------------------------------------------------------------*/
void l1_dyn_dwnld_reset_api()
{
  dyn_dwl_ndb->d_api_dwl_download_ctrl       = (API) 0;
  dyn_dwl_ndb->d_api_dwl_error_code          = (API) 0;
  dyn_dwl_ndb->d_api_dwl_function_address[0] = (API) 0;
  dyn_dwl_ndb->d_api_dwl_function_address[1] = (API) 0;
  dyn_dwl_ndb->d_api_dwl_crc                 = (API) 0;
  dyn_dwl_ndb->d_api_dwl_size                = (API) 0;
  dyn_dwl_ndb->d_api_dwl_write_pointer       = (API) 0;
}

#endif  // L1_DYN_DSP_DWNLD

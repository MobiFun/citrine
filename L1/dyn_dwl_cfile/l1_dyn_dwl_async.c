/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_DYN_DWL_ASYNC.C
 *
 *        Filename l1_dyn_dwl_async.c
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/
#include <stdio.h>
#include <string.h>
#include "nucleus.h"
#include "l1_confg.h"
#include "l1_types.h"
#include "sys_types.h"
#include "cust_os.h"
#include "l1audio_signa.h"
#include "l1audio_const.h"
#include "l1audio_cust.h"
#include "l1audio_defty.h"
#include "l1_const.h"
#include "l1tm_defty.h"
#if (L1_GTT == 1)
  #include "l1gtt_const.h"
  #include "l1gtt_defty.h"
#endif
#if (L1_DYN_DSP_DWNLD==1)
  #include "l1_dyn_dwl_defty.h"
  #include "l1_dyn_dwl_msgty.h"
  #include "l1_dyn_dwl_const.h"
  #include "l1_dyn_dwl_signa.h"
  #include "l1_dyn_dwl_error.h"
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
#include "l1_varex.h"
#include "l1_trace.h"
#include "sys_dma.h"

#if (OP_RIV_AUDIO == 1)
  #include "rv/rv_general.h"
#endif


#if (L1_DYN_DSP_DWNLD == 1)
#if(CODE_VERSION == SIMULATION)
  extern   VOID trace_fct_simu_dyn_dwnld(CHAR *fct_name);
#endif // CODE_VERSION == SIMULATION

  

extern UWORD32 dyn_dwnld_address_vect[];
extern const UWORD8 *dyn_dwnld_copy_MCU_vect[];
extern UWORD16 dyn_dwnld_crc_vect[];
extern UWORD16 size_vect[];

/*---------------------------------------------------------------------------------------------------------*/
/* l1_patch_id2string                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Parameters : patch identificator "patch_id", string vector "vector" which identifies the patch          */
/*                                                                                                         */
/* Return     : Fills the string vector containing the patch id by reference                               */
/*                                                                                                         */
/*                                                                                                         */
/* Description :  Links the patch ID number to its string value                                            */
/*                                                                                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#if 0
void l1_patch_id2string(UWORD16 patch_id, char* vect)
{

  switch(patch_id)
  {

#if ((CODE_VERSION == SIMULATION) || (((CHIPSET == 12) || (CHIPSET == 15))))  
    case MP3_PATCH:
    {
      vect[0] = 'M';
      vect[1] = 'P';
      vect[2] = '3';
      vect[3] = '\0';
    }
    break;
#endif
    case MMS_PATCH:
    {
      vect[0] = 'M';
      vect[1] = 'M';
      vect[2] = 'S';
      vect[3] = '\0';
    }
    break;

#if ((CODE_VERSION == SIMULATION) || (((CHIPSET == 12) || (CHIPSET == 15))))
    case E2_PATCH:
    {
      vect[0] = 'E';
      vect[1] = '2';
      vect[2] = ' ';
      vect[3] = '\0';
    }
    break;
#endif
    case TTY_PATCH:
    {
      vect[0] = 'T';
      vect[1] = 'T';
      vect[2] = 'Y';
      vect[3] = '\0';
    }
    break;
    case SPEECH_ACOUSTIC_PATCH:
    {
      vect[0] = 'A';
      vect[1] = 'N';
      vect[2] = 'R';
      vect[3] = '\0';
    }
    break;
#if ((CODE_VERSION == SIMULATION) || (CHIPSET == 12) || (CHIPSET == 15))
    case AAC_PATCH:
    {
      vect[0] = 'A';
      vect[1] = 'A';
      vect[2] = 'C';
      vect[3] = '\0';
    }
    break;
#endif
#if ((CODE_VERSION == SIMULATION) || (CHIPSET == 12) || (CHIPSET == 15))
    case PCM_EXTRACTION_PATCH:
    {
      vect[0] = 'P';
      vect[1] = 'C';
      vect[2] = 'M';
      vect[3] = '\0';
    }
    break;
#endif

  }
}
#endif

/*----------------------------------------------------------------------------------------*/
/* l1_dynamic_download_manager                                                            */
/*----------------------------------------------------------------------------------------*/
/*                                                                                        */
/* Parameters : Signal Code of the message, delay flag                                    */
/*                                                                                        */
/* Return     : True if the primitives triggers a dynamic download and sets all the       */
/*              parameters to perform a dynamic download, false otherwise                 */
/*                                                                                        */
/* Description :  Implements the dynamic download manager algorithm                       */
/*                                                                                        */
/*                                                                                        */
/*----------------------------------------------------------------------------------------*/


BOOL l1_dynamic_download_manager(UWORD32 SignalCode, BOOL delay_flag)
{

  UWORD16 temp_patch_array[MAX_NUM_OF_PATCH_IDS];
  UWORD16 temp_num_patch;
  UWORD16 num_of_patch_id_to_dwnld;
  UWORD16 num_of_uninstall_elem;
  UWORD16 i;
  UWORD16 patch_id_uninstall_vect[MAX_NUM_OF_PATCH_IDS];
  UWORD16 patch_id;
  UWORD16 temp_patch_id[MAX_NUM_OF_PATCH_IDS];
  BOOL return_flag = FALSE;

  // Primitive is processed only if it triggers a dynamic download or there is a delay
  if(delay_flag == TRUE || l1_does_the_incoming_primitive_trigger_dynamic_dwnld(SignalCode, FALSE) == TRUE)
  {

    //----------------------------------------------------------------------    
    // Compute which patch ids (passed by reference) and the number of patches to install
    //----------------------------------------------------------------------
    
    temp_num_patch= l1_lookup_primitive_patch_matrix(SignalCode,temp_patch_array);  
    i=0;
    num_of_patch_id_to_dwnld = 0;

    //--------------------------------------------
    // Check if there is any patch that still must be installed
    //--------------------------------------------
    
    while(i < temp_num_patch)
    {
      if(l1_is_patch_already_installed(temp_patch_array[i]) == FALSE)
      {
        l1a.dyn_dwnld.next_patch_id[num_of_patch_id_to_dwnld++] = temp_patch_array[i];
      }
      i++;
    }

    //--------------------------
    // Manage patch incompatibilities
    //--------------------------
    
    // if there's at least one patch that must be installed
    if(num_of_patch_id_to_dwnld!=0)
    {
      return_flag = TRUE;
      // Check if already installed patches are compatible or not and retrieve number of patches to uninstall and their ids
      if(l1_manage_patch_incompatibilty(num_of_patch_id_to_dwnld,&(num_of_uninstall_elem),patch_id_uninstall_vect))
      {
        // Set L1A-API HISR variable: no elements to uninstall
        l1a_apihisr_com.dyn_dwnld.uninstall_parameters.num_of_elem=0;

        // Trace number of elements to uninstall 
#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
        if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
        {
          char string[18];
          sprintf(string,"No UNINSTALL: \r\n");
#if(CODE_VERSION == SIMULATION)
          trace_fct_simu_dyn_dwnld(string);
#else
          rvt_send_trace_cpy((T_RVT_BUFFER)string,trace_info.l1_trace_user_id,strlen(string),RVT_ASCII_FORMAT);
#endif // CODE_VERSION == SIMULATION
        }
#endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
    }
    else
    {
      UWORD16 element,j,index_counter;
             
      // Set uninstall elements number and their address 
      l1a_apihisr_com.dyn_dwnld.uninstall_parameters.num_of_elem=num_of_uninstall_elem;

      // Trace number of elements to uninstall
#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
      if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
      {
        char str[20];
        sprintf(str,"UNINST_NUM: %d |\r\n", l1a_apihisr_com.dyn_dwnld.uninstall_parameters.num_of_elem);
#if(CODE_VERSION == SIMULATION)
        trace_fct_simu_dyn_dwnld(str);
#else
        rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
#endif // CODE_VERSION == SIMULATION
      }
#endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

      // For all patches to uninstall...
      for(i=0;i<num_of_uninstall_elem;i++)
      {  
        element=patch_id_uninstall_vect[i]; 

        // Trace IDs of elements to uninstall
#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
        if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
        {
          char str[18];
          sprintf(str,"UNINST_ID: %d \r\n", element);
#if(CODE_VERSION == SIMULATION)
          trace_fct_simu_dyn_dwnld(str);
#else
          rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
#endif // CODE_VERSION == SIMULATION
        }
#endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

        // Set uninstall function address 
        l1a_apihisr_com.dyn_dwnld.uninstall_parameters.address[i]=dyn_dwnld_address_vect[element];
      }

      index_counter = 0;
      for(i=0; i<l1a.dyn_dwnld.num_patches_installed;i++)
      {
        for(j=0;j<num_of_uninstall_elem;j++)
        {
          if(l1a.dyn_dwnld.patch_id[i] == patch_id_uninstall_vect[j])
          {
            l1a.dyn_dwnld.patch_id[i] = 0xFFFF; //omaps00090550 ;
            break;
          }  
        }
        if(j == num_of_uninstall_elem)
        {
          temp_patch_id[index_counter++]=l1a.dyn_dwnld.patch_id[i];
        }
      }
      // Reset global variables used by dynamic download
      l1a.dyn_dwnld.num_patches_installed-=num_of_uninstall_elem;
      for(i=0;i<l1a.dyn_dwnld.num_patches_installed;i++)
        l1a.dyn_dwnld.patch_id[i] = temp_patch_id[i];
    }

   //-----------------------------------------
   // Update L1A-L1APIHISR communication structure
   //-----------------------------------------
   
    l1a_apihisr_com.dyn_dwnld.copy_parameters.num_of_elem=num_of_patch_id_to_dwnld; 
    l1a.dyn_dwnld.num_of_elem_to_copy = num_of_patch_id_to_dwnld;
     
    // Trace number of elements to download
#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
    if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
    {
      char str[18];
      sprintf(str,"DWNLD_NUM: %d \r\n", num_of_patch_id_to_dwnld);
#if(CODE_VERSION == SIMULATION)
      trace_fct_simu_dyn_dwnld(str);
#else
      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
#endif // CODE_VERSION == SIMULATION
    }
#endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

    // For all patches that must be downloaded...
    for (i=0;i<num_of_patch_id_to_dwnld;i++)
    {
      patch_id=l1a.dyn_dwnld.next_patch_id[i];

#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
      if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
      {
        char str[18];
        sprintf(str,"DWNLD_ID: %d \r\n", patch_id);

#if(CODE_VERSION == SIMULATION)
        trace_fct_simu_dyn_dwnld(str);
#else
        rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
#endif // CODE_VERSION == SIMULATION
      }
#endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

      // Set patch IDs addresses: copy source address and install address             
      l1a_apihisr_com.dyn_dwnld.copy_parameters.start_MCU_copy_address[i] =
	  (UWORD32) dyn_dwnld_copy_MCU_vect[patch_id];
      l1a_apihisr_com.dyn_dwnld.copy_parameters.address_to_install[i] =
	  dyn_dwnld_address_vect[patch_id];     
      l1a_apihisr_com.dyn_dwnld.copy_parameters.crc[i] =
	  dyn_dwnld_crc_vect[patch_id];
      l1a_apihisr_com.dyn_dwnld.copy_parameters.size_array[i] =
	  size_vect[patch_id];
    }

   //-----------------------------------------
   // Set semaphores to lock involved state machines
   //-----------------------------------------

    if(delay_flag == FALSE)
      // Tell other state machines that they have to stay in steady state waiting for dynamic download activity to be completed

      l1_set_semaphores_for_all_state_machines_involved(num_of_patch_id_to_dwnld,l1a.dyn_dwnld.next_patch_id);
    else

      // Update the already set semaphores if it is the case

      l1_update_semaphores_for_all_state_machines(num_of_patch_id_to_dwnld);
    }
  }
  return return_flag;
}

/*----------------------------------------------------------------------------------------*/
/* l1a_dyn_dwnld_set_process                                                              */
/*----------------------------------------------------------------------------------------*/
/*                                                                                        */
/* Parameters : Signal Code of the message, delay flag                                    */
/*                                                                                        */
/* Return     : True if the primitives triggers a dynamic download and sets all the       */
/*              parameters to perform a dynamic download, false otherwise                 */
/*                                                                                        */
/* Description :  Implements the dynamic download manager algorithm                       */
/*                                                                                        */
/*                                                                                        */
/*----------------------------------------------------------------------------------------*/
void l1a_dyn_dwnld_set_process(void)
{

#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
  // Disable trace DSP upon Dynamic Download activation
  l1_disable_DSP_trace();
#endif
          
  // Reset API variables
  l1_dyn_dwnld_reset_api();

  // In case E2 is running, dynamic download must use a different area as E2 samples use trace DSP buffer
  // area which is used as well for dynamic download

  if ((l1a.dyn_dwnld.melody0_E2_flag_activated == TRUE) || (l1a.dyn_dwnld.melody1_E2_flag_activated == TRUE))

  // Set download parameters: size and start download address of API area and number of patches to download
  {
    l1a_apihisr_com.dyn_dwnld.copy_parameters.start_of_dwnld_area= START_API_DWNLD_AREA_DURING_E2;
    l1a_apihisr_com.dyn_dwnld.copy_parameters.size_of_dwnld_area= SIZE_API_DWNLD_AREA_DURING_E2;
  }
  else
  {
    l1a_apihisr_com.dyn_dwnld.copy_parameters.start_of_dwnld_area=START_API_DWNLD_AREA;
    l1a_apihisr_com.dyn_dwnld.copy_parameters.size_of_dwnld_area=SIZE_API_DWNLD_AREA;
  }

  // Dynamic download HISR can be activated
  l1_apihisr.dyn_dwnld.running=TRUE;

  // Tell the L1S to start the DSP background task
  l1a_l1s_com.dyn_dwnld_task.start=TRUE;
}


/*-------------------------------------------------------*/
/* l1a_dyn_dsp_dwnld_process()                           */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a state machine which handles the    */
/* dynamic download feature.                             */
/*                                                       */
/* Starting messages:                                    */
/*                                                       */
/* Result messages (input):                              */
/*                                                       */
/*                                                       */
/* Result messages (output):                             */
/*                                                       */
/* Reset messages (input):   none                        */
/*                                                       */
/* Stop message (input):                                 */
/*                                                       */
/* Stop message (output):                                */
/*                                                       */
/* Rem:                                                  */
/* ----                                                  */
/*                                                       */
/*-------------------------------------------------------*/

void l1a_dyn_dsp_dwnld_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET=0,
    WAIT_INIT,
    WAIT_RESULT,
    WAIT_STOP
  };

  UWORD8  *state    =&l1a.dyn_dwnld.state;
  UWORD32 SignalCode=msg->SignalCode;
  UWORD16 i;
  UWORD16 delay_patch_array[MAX_NUM_OF_PATCH_IDS];
  UWORD16 delay_num_patch;
  UWORD32 delay_primitive;

  while(1)
  {
    switch(*state)
    {
      // *********
      // * RESET *
      // *********
      case RESET:
      {
        // Reset intra L1 variables: L1A-L1S interface, L1A-API interface, global API HISR variables
        l1_dyn_dwnld_reset();

        // Change state
        *state = WAIT_INIT;
      }
      break;

      // ******************
      // * WAIT START REQ *
      // ******************
      case WAIT_INIT:
      {
        
        // *----------------------------*
        // * Dynamic download manager   *
        // *----------------------------*

        
        /* Run the dynamic download manager */
        if (l1_dynamic_download_manager(SignalCode, FALSE) == TRUE)
        {
         
          /* Initialisation of dynamic download process */
          l1a_dyn_dwnld_set_process();
         
          // Change state
          *state=WAIT_RESULT;

        }
        return;
      }
//omaps00090550      break;

      // ***************
      // * WAIT_RESULT *
      // ***************

      case WAIT_RESULT:
      {
        if (l1_does_the_incoming_primitive_trigger_dynamic_dwnld(SignalCode,TRUE) == TRUE)
        {
          if (l1_push_Primitive(SignalCode) == TRUE)
          {
            delay_num_patch = l1_lookup_primitive_patch_matrix(SignalCode,delay_patch_array);

            // Tell other state machines that they have to stay in steady state waiting for dynamic download activity to be completed
            l1_set_semaphores_for_all_state_machines_involved(delay_num_patch, delay_patch_array);
          }
          else
          {
#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
            if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
            {
              char str[16];
              sprintf(str,"Fifo ovflw:\r\n");
#if(CODE_VERSION == SIMULATION)
              trace_fct_simu_dyn_dwnld(str);
#else
              rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
#endif // CODE_VERSION == SIMULATION
            }
#endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
          }
        }
        switch(SignalCode)
        {

          // *----------------------------------------------------------------------------------*
          // * CRC reported is not OK or an error has been reported; Restart patch from scratch *
          // *----------------------------------------------------------------------------------*
          case API_L1_CRC_NOT_OK:
          {
#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
            // Trace 
            if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
            {
              char str[18];
              sprintf(str,"CRC NOK: %4x \r\n",((T_API_L1_CRC_NOT_OK *)(msg->SigP))->patch_id);
#if(CODE_VERSION == SIMULATION)
              trace_fct_simu_dyn_dwnld(str);
#else
              rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
#endif // CODE_VERSION == SIMULATION
            }
#endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

            // Set recovery flag to TRUE;
            l1a_l1s_com.recovery_flag = TRUE;
            *state = RESET;
          }
          break;
          case API_L1_DYN_DWNLD_STOP:
          {
#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
            if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
            {
              char str[15];
              sprintf(str,"DSP Error \r\n");
#if(CODE_VERSION == SIMULATION)
              trace_fct_simu_dyn_dwnld(str);
#else
              rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
#endif // CODE_VERSION == SIMULATION
            }
#endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

            // Set recovery flag to TRUE;
            l1a_l1s_com.recovery_flag = TRUE;
            *state = RESET;
          }
          break;

          // *-------------------------------------------------------------*
          // * Dynamic download finished: all patches have been downloaded *
          // *-------------------------------------------------------------*
          case API_L1_CRC_OK:
          {
#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
            // Trace 
            if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
            {
              char str[18];
              sprintf(str,"CRC OK: %4x \r\n",((T_API_L1_CRC_NOT_OK *)(msg->SigP))->patch_id);
#if(CODE_VERSION == SIMULATION)
              trace_fct_simu_dyn_dwnld(str);
#else
              rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
#endif // CODE_VERSION == SIMULATION
            }
#endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
          }
          break;
          case API_L1_DYN_DWNLD_FINISHED:
          {

            // Store currently installed patch
            for (i=0;i<l1a.dyn_dwnld.num_of_elem_to_copy;i++)
            {
              l1a.dyn_dwnld.patch_id[l1a.dyn_dwnld.num_patches_installed+i]=l1a.dyn_dwnld.next_patch_id[i];
            }

            // Update the counter of the patch currently installed and reset sempahores
            l1a.dyn_dwnld.num_patches_installed+=l1a.dyn_dwnld.num_of_elem_to_copy;
            l1_reset_semaphores();

            // Reset number of element to copy in the next phase
            l1a.dyn_dwnld.num_of_elem_to_copy = 0;

            // Reset next patch ID global variables
            for(i=0;i<MAX_NUM_OF_PATCH_IDS;i++)
              l1a.dyn_dwnld.next_patch_id[i] = 0xFFFF; //omaps00090550 ;
            
#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
            // Trace 
            if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
            {
              char str[26];
              sprintf(str,"Patch dwnld finished \r\n");
#if(CODE_VERSION == SIMULATION)
              trace_fct_simu_dyn_dwnld(str);
#else
              rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
#endif // CODE_VERSION == SIMULATION
            }
#endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            
            //  Check if there is any pending primitive waiting to be de-queued
            {
#if 0	/* LoCosto code */
              UWORD32 remaining_primitive_flag=0;
              UWORD32 process_continue_flag=1;
              UWORD32 delay_primitive_processed_flag=0;
              
              remaining_primitive_flag = (l1_check_Fifo_Primitive()>0);

              while(remaining_primitive_flag && process_continue_flag)
              {
                l1_pop_Primitive(&(delay_primitive));
                if (l1_dynamic_download_manager(delay_primitive, TRUE) == TRUE)
                {
                  process_continue_flag=0;                  
                  delay_primitive_processed_flag=1;
                  // If yes set the restart command at apihisr level
                  l1a_apihisr_com.dyn_dwnld.command.restart = TRUE;

                  /********** WORKAROUND *************/
                #if (OP_RIV_AUDIO == 1)
                  {
                    // WARNING: temporary until os_activate_hisr() is declared in L3 functions
                    extern NU_HISR apiHISR;
                    NU_Activate_HISR(&apiHISR);
                  }
                #else
                  os_activate_hisr(API_HISR);
                #endif // OP_RIV_AUDIO == 1         
                  /********** WORKAROUND *************/
                  *state = WAIT_RESULT;
                }
                remaining_primitive_flag = (l1_check_Fifo_Primitive()>0);
              }

              // else stop the DSP background task as no other patch must be downloaded
              if(delay_primitive_processed_flag == 0)
              {
              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
                // Enable trace DSP upon Dynamic Download deactivation
                l1_enable_DSP_trace();
                //Trace_dsp_dump();
              #endif  // omaps00090550 #14-D removal
                l1a_l1s_com.dyn_dwnld_task.stop=TRUE;
                *state = WAIT_STOP;
              }

#else	/* TCS211 reconstruction from disassembly */

              if (l1_check_Fifo_Primitive() != 0)
              {
                l1_pop_Primitive(&(delay_primitive));
                if (l1_dynamic_download_manager(delay_primitive, TRUE) == TRUE)
                {
                  // If yes set the restart command at apihisr level
                  l1a_apihisr_com.dyn_dwnld.command.restart = TRUE;

                  /********** WORKAROUND *************/
                #if (OP_RIV_AUDIO == 1)
                  {
                    // WARNING: temporary until os_activate_hisr() is declared in L3 functions
                    extern NU_HISR apiHISR;
                    NU_Activate_HISR(&apiHISR);
                  }
                #else
                  os_activate_hisr(API_HISR);
                #endif // OP_RIV_AUDIO == 1         
                  /********** WORKAROUND *************/
                  *state = WAIT_RESULT;
                }
              }
              else
              {
              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
                // Enable trace DSP upon Dynamic Download deactivation
                l1_enable_DSP_trace();
                //Trace_dsp_dump();
              #endif  // omaps00090550 #14-D removal
                l1a_l1s_com.dyn_dwnld_task.stop=TRUE;
                *state = WAIT_STOP;
              }
#endif
            }
          }
          break;
        }  // switch(SignalCode)
        return;
      }
//omaps00090550      break;  // case WAIT_RESULT
      case WAIT_STOP:
      {
        /* In case of a primitive which triggers a dynamic download arrives we must start over */
	if (l1_does_the_incoming_primitive_trigger_dynamic_dwnld(SignalCode,TRUE) == TRUE)
	{
          /* Run the dynamic download manager */
          if (l1_dynamic_download_manager(SignalCode, FALSE) == TRUE)
	  {
            /* Initialisation of dynamic download process */
            l1a_dyn_dwnld_set_process();

            // Change state
            *state=WAIT_RESULT;
	  }
	}
	else if (SignalCode == L1_DYN_DWNLD_STOP_CON)
	{
          // DYN DWNLD HISR must be deactivated
          l1_apihisr.dyn_dwnld.running=FALSE;
          // Change state
          *state=RESET;	
	} 		
	return;		
      }
//omaps00090550      break;
    }
  }
}
#endif    // L1_DYN_DSP_DWNLD

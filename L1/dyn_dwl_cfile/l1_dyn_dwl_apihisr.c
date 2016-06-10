/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_DYN_DWL_APIHISR.C
 *
 *        Filename l1_dyn_dwl_apihisr.c
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "l1_confg.h"
#include "sys_types.h"
#include "../../riviera/rv/rv_general.h"
#include "../../nucleus/nucleus.h"
#include "l1_types.h"
#include "l1audio_cust.h"
#include "l1audio_defty.h"
#include "l1audio_const.h"
#include "l1_const.h"
#include "l1tm_defty.h"

#if (L1_GTT == 1)
  #include "l1gtt_const.h"
  #include "l1gtt_defty.h"
#endif

#if (L1_DYN_DSP_DWNLD == 1)
  #include "l1_dyn_dwl_const.h"
  #include "l1_dyn_dwl_signa.h"
  #include "l1_dyn_dwl_defty.h"
  #include "l1_dyn_dwl_msgty.h"
  #include "l1_dyn_dwl_error.h"
  #include "l1_dyn_dwl_proto.h"
  #include "l1_api_hisr.h"
#endif
#if (L1_MP3 == 1)
  #include "l1mp3_defty.h"
#endif //L1_MP3
#if (L1_MIDI == 1)
  #include "l1midi_defty.h"
#endif

#include "l1_defty.h"
#include "../../gpf/inc/cust_os.h"
/* #include "nu_main.h" */
#include "l1audio_signa.h"
#include "l1audio_cust.h"
#include "l1_varex.h"
#include "l1_macro.h"
#include "l1_api_hisr.h"
#include "l1_trace.h"

#if (L1_DYN_DSP_DWNLD == 1)

/* Dynamic Download NDB API */
T_DYN_DWNLD_MCU_DSP *dyn_dwl_ndb;

#if (CODE_VERSION == SIMULATION)
  T_DYN_DWNLD_MCU_DSP dyn_dwl_ndb_sim;
  UWORD16 dwnld_area_array[SIZE_DWNLD_AREA_SIMU];
#endif

enum states
  {
     RESET = 0, 
     WAIT_UNINSTALL = 1,
     WAIT_DSP_END_BUFFER = 2,
     WAIT_CRC = 3,
     WAIT_INSTALL = 4
  };
/*------------------------------------------------------------------------------------------------------------- */
/* l1_dyn_dwnld_copy_patch_process()                                                                            */
/*------------------------------------------------------------------------------------------------------------- */
/*                                                                                                              */
/* Parameters : BOOL new_patch: TRUE if the patch is copied from scratch, FALSE if it has been started so far   */
/*                                                                                                              */
/* Return     : state in which must be stepped into                                                             */
/*                                                                                                              */
/* Description : Performs the copy of the patch and computes next state of corresponding APIHISR state machine  */
/*                                                                                                              */
/*------------------------------------------------------------------------------------------------------------- */

UWORD8 l1_dyn_dwnld_copy_patch_process (BOOL new_patch)
{
  BOOL        still_words_to_be_copied;
  UWORD16     tmp_dwnld_area_size;
  UWORD8      return_state;
  UWORD16     *p_src_mcu = NULL;
  UWORD16     *p_dest_mcu = NULL;
  UWORD16     *tmp_pointer = NULL;

  UWORD16 tmp_patch_size = l1_apihisr.dyn_dwnld.tmp_patch_size;

  /* Copy first N block of data */
  still_words_to_be_copied = l1_init_pointers_and_copy_first_block_of_data(&(tmp_dwnld_area_size), &(tmp_patch_size), &(p_dest_mcu), &(p_src_mcu),new_patch);

  /* Set download command */
   dyn_dwl_ndb->d_api_dwl_download_ctrl = (API) C_DWL_DOWNLOAD_CTRL_DOWNLOAD;  

   #if (CODE_VERSION == SIMULATION)
     l1_trigger_api_interrupt();
   #endif

  /* Check if there are still words to be copied after first API interrupt generation: if not */
  /* the patch has been completely downloaded and MCU waits for CRC*/
  if(still_words_to_be_copied == FALSE)
  {
    return_state = WAIT_CRC;
  }

  /* If not copy the patch: if download area is bigger than patch size copy until the end of the patch */
  /* Else copy till the end of buffer download area and wait for DSP interrupt */
  else
  {
    if (tmp_dwnld_area_size >= tmp_patch_size)
    {
      l1_copy_till_the_end_of_the_patch_and_update_write_pointer(tmp_patch_size,p_dest_mcu,p_src_mcu);
      return_state = WAIT_CRC;
    }
    else
    {
      l1_copy_till_end_of_dwnld_area_and_update_write_pointer(tmp_dwnld_area_size,p_dest_mcu,&tmp_patch_size,&p_src_mcu);

      /* Save source patch file pointerand temporary patch size*/
      l1_apihisr.dyn_dwnld.running_source_pointer = (UWORD32) p_src_mcu;
      l1_apihisr.dyn_dwnld.tmp_patch_size  = tmp_patch_size;

      /* Change state*/
      return_state = WAIT_DSP_END_BUFFER;
    }
  }
  return return_state;
}
/*-------------------------------------------------------------*/
/* l1_dyn_dwnld_apihisr()                                      */
/*-------------------------------------------------------------*/
/*                                                             */
/* Parameters : none                                           */
/*                                                             */
/* Return     : n/a                                            */
/*                                                             */
/* Description : implements Dynamic Download API HISR          */
/*                                                             */
/*-------------------------------------------------------------*/

void l1_dyn_dwnld_apihisr()
{

  UWORD8 *state = &l1_apihisr.dyn_dwnld.state;


  /* Variables for copy process */
  
  xSignalHeaderRec    *conf_msg;
  BOOL                flag_error;
  
  /* Dynamic Download error handler : check if critical error occured */
  if( l1_dyn_dwnld_apihisr_error_handler() == TRUE )
  {
     /* Send notification to L1A */
     conf_msg = os_alloc_sig(sizeof(T_API_L1_DYN_DWNLD_STOP));
     DEBUGMSG(status,NU_ALLOC_ERR)
     conf_msg->SignalCode = API_L1_DYN_DWNLD_STOP;
     ((T_API_L1_DYN_DWNLD_STOP *) (conf_msg->SigP))->error = dyn_dwl_ndb->d_api_dwl_error_code;
     dyn_dwl_ndb->d_api_dwl_error_code = C_DWL_ERR_RESET;
     os_send_sig(conf_msg,L1C1_QUEUE);
     DEBUGMSG(status,NU_SEND_QUEUE_ERR)

     /* Branch state */
     *state = RESET;
     flag_error = TRUE;
     return;
  }
  else
  {
    flag_error = FALSE;
  }

  /****************/
  /*STATE MACHINE */
  /****************/

  while (1){
  switch(*state)
  {
    /*********/
    /* RESET */
    /*********/
    case RESET:
    {
      /* Check reset init command (if reset by DSP) / Restart in case dynamic download delayed */
      if ( dyn_dwl_ndb->d_api_dwl_download_ctrl == (API) C_DWL_DOWNLOAD_CTRL_DSP_ACK || 
      l1a_apihisr_com.dyn_dwnld.command.restart == TRUE)
      {
        if (flag_error == FALSE)
        {  

          /* Send confirmation to L1A */
          conf_msg = os_alloc_sig(0);
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = API_L1_DYN_DWNLD_START_CON;
          os_send_sig(conf_msg,L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)

          /* Store patch IDs to install counter in global API */
          l1_apihisr.dyn_dwnld.patch_ids_counter = l1a_apihisr_com.dyn_dwnld.copy_parameters.num_of_elem;
      
          /* Reset command */
          l1a_apihisr_com.dyn_dwnld.command.restart = FALSE;

          /* Test if number of uninstall elements is greater than zero */
          if ( l1a_apihisr_com.dyn_dwnld.uninstall_parameters.num_of_elem > 0 )
          {
            /* Copy num of elem L1A-API variable counter into global uninstall counter */
            l1_apihisr.dyn_dwnld.uninstall_counter = l1a_apihisr_com.dyn_dwnld.uninstall_parameters.num_of_elem;

            l1_set_uninstall_parameters();

            #if (CODE_VERSION == SIMULATION)
              l1_trigger_api_interrupt();
            #endif

            /* Change state */
            *state = WAIT_UNINSTALL;     
          }
          else /* No elements to uninstall*/
          {
            /* Reset patch size */
            l1_apihisr.dyn_dwnld.tmp_patch_size = 0;

            /* Copy the patch and update current state*/
            *state =  l1_dyn_dwnld_copy_patch_process(TRUE);

          }
        }
      }
      return;
    }   /* end case RESET */
//omaps00090550    break;

    /******************/
    /* WAIT_UNINSTALL  */
    /******************/
    case WAIT_UNINSTALL:
    {
      /* Check uninstall command (if reset by DSP) */
      if ( dyn_dwl_ndb->d_api_dwl_download_ctrl == (API) C_DWL_DOWNLOAD_CTRL_DSP_ACK )
      {
        /* Decrement uninstall counter */
        l1_apihisr.dyn_dwnld.uninstall_counter--;

        /* Check uninstall counter: if it is 0 no more uninstall to perform*/
        if ( l1_apihisr.dyn_dwnld.uninstall_counter == 0 )
        {
          /* Send confirmation to L1A */
          conf_msg = os_alloc_sig(0);
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = API_L1_DYN_DWNLD_UNINST_OK;
          os_send_sig(conf_msg,L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)

          /* Reset patch size */
          l1_apihisr.dyn_dwnld.tmp_patch_size = 0;

          /* Copy the patch and update current state*/
           *state =  l1_dyn_dwnld_copy_patch_process(TRUE);
        }  
        else /* there are some uninstall to perform */
        {
          l1_set_uninstall_parameters();

		  #if (CODE_VERSION == SIMULATION)
            l1_trigger_api_interrupt();
          #endif
        }
      }
      return; 
    }   /* end case WAIT_UNINSTALL */
//omaps00090550    break;

    /***********************/
    /* WAIT_DSP_END_BUFFER */
    /***********************/
    case WAIT_DSP_END_BUFFER:
    {
      if ( dyn_dwl_ndb->d_api_dwl_download_ctrl == (API) C_DWL_DOWNLOAD_CTRL_DSP_ACK )
      {
        /* Copy the patch and update current state*/
        *state = l1_dyn_dwnld_copy_patch_process(FALSE);
      }
      return;
    } /* end case WAIT_DSP_END_BUFFER */
//omaps00090550    break;

    /************/
    /* WAIT_CRC */
    /************/
    case WAIT_CRC:
    {
      /* Check if DSP install command is reset */
      if (dyn_dwl_ndb->d_api_dwl_download_ctrl == (API) C_DWL_DOWNLOAD_CTRL_DSP_ACK )
      {
        /* Test if CRC is OK */
        if ( dyn_dwl_ndb->d_api_dwl_crc != l1a_apihisr_com.dyn_dwnld.copy_parameters.crc[l1a_apihisr_com.dyn_dwnld.copy_parameters.num_of_elem-l1_apihisr.dyn_dwnld.patch_ids_counter])       /* CRC not OK */
        {
          /* Send notification to L1A */
          conf_msg = os_alloc_sig(sizeof(T_API_L1_CRC_NOT_OK));
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = API_L1_CRC_NOT_OK;
          ((T_API_L1_CRC_NOT_OK *) (conf_msg->SigP))->patch_id = dyn_dwl_ndb->d_api_dwl_crc;
          os_send_sig(conf_msg,L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)

          /* Change state */
          *state = RESET;
        }
        else              /* CRC OK */
        {
          conf_msg = os_alloc_sig(sizeof(T_API_L1_CRC_NOT_OK));
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = API_L1_CRC_OK;
          ((T_API_L1_CRC_NOT_OK *) (conf_msg->SigP))->patch_id = l1a_apihisr_com.dyn_dwnld.copy_parameters.crc[l1a_apihisr_com.dyn_dwnld.copy_parameters.num_of_elem-l1_apihisr.dyn_dwnld.patch_ids_counter];
          os_send_sig(conf_msg,L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)
          *state = WAIT_INSTALL;

          /* Set install parameters */
          
          dyn_dwl_ndb->d_api_dwl_function_address[0] = 
          (API) (l1a_apihisr_com.dyn_dwnld.copy_parameters.address_to_install[l1a_apihisr_com.dyn_dwnld.copy_parameters.num_of_elem-l1_apihisr.dyn_dwnld.patch_ids_counter] & 0x0000FFFF);
          dyn_dwl_ndb->d_api_dwl_function_address[1] = 
          (API) ((l1a_apihisr_com.dyn_dwnld.copy_parameters.address_to_install[l1a_apihisr_com.dyn_dwnld.copy_parameters.num_of_elem-l1_apihisr.dyn_dwnld.patch_ids_counter] >> 16) & 0x0000FFFF);

          dyn_dwl_ndb->d_api_dwl_download_ctrl = (API) C_DWL_DOWNLOAD_CTRL_INSTALL;

		  #if (CODE_VERSION == SIMULATION)
            l1_trigger_api_interrupt();
          #endif
        }
      }  
      return;
    }   /* end case WAIT_CRC */
//omaps00090550    break;

    /****************/
    /* WAIT_INSTALL */
    /****************/
    case WAIT_INSTALL:
    {
      /* Check if DSP install command is reset */
      if (dyn_dwl_ndb->d_api_dwl_download_ctrl == (API) C_DWL_DOWNLOAD_CTRL_DSP_ACK )
      {
        /* Decrement patch counter */
        l1_apihisr.dyn_dwnld.patch_ids_counter--;

        /* Test if patch counter is null */
        if ( l1_apihisr.dyn_dwnld.patch_ids_counter == 0 )
        {
          /* Send notification to L1A */
          conf_msg = os_alloc_sig(0);
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = API_L1_DYN_DWNLD_FINISHED;
          os_send_sig(conf_msg,L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)
          *state = RESET;
        }
        else
        {
          /* Reset patch size */
          l1_apihisr.dyn_dwnld.tmp_patch_size = 0;

          /* Copy the patch and update current state*/
          *state =  l1_dyn_dwnld_copy_patch_process(TRUE);
        }
      }
      return;
    }   /* end case WAIT_INSTALL */
//omaps00090550    break;
  }   /* end switch */
}  /* end while */
}

/*-------------------------------------------------------------*/
/* l1_dyn_dwnld_apihisr_error_handler()                        */
/*-------------------------------------------------------------*/
/*                                                             */ 
/* Parameters : error_code (OUT) error_code received from DSP  */
/*                                                             */
/* Return     : TRUE if errors signaled by DSP                 */
/*                                                             */
/*-------------------------------------------------------------*/
 BOOL l1_dyn_dwnld_apihisr_error_handler()
{
  BOOL  critical;

  /* Initialisation */
  critical = FALSE;

  if(dyn_dwl_ndb->d_api_dwl_error_code != 0)
  {
    critical = TRUE;
  }

  return critical;
}


#endif  /* L1_DYN_DSP_DWNLD */


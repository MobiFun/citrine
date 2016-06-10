/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_DYN_DWL_FUNC.C
 *
 *        Filename l1_dyn_dwl_func.c
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#include <stdio.h>
#include <string.h>
#include "l1_confg.h"
#include "l1_types.h"
#include "l1_const.h"        
#include "l1_signa.h"     
#include "sys_types.h"

#if(L1_DYN_DSP_DWNLD == 1)
#include "l1_dyn_dwl_const.h"
#include "l1_dyn_dwl_proto.h"
#include "l1_dyn_dwl_defty.h"
#endif
#if TESTMODE
  #include "l1tm_defty.h"
#endif
#if (AUDIO_TASK == 1)
  #include "l1audio_const.h"
  #include "l1audio_cust.h"
  #include "l1audio_defty.h"
#endif
#if (L1_GTT == 1)
  #include "l1gtt_const.h"
  #include "l1gtt_defty.h"
#endif
#if (L1_MP3 == 1)
  #include "l1mp3_defty.h"
#endif //L1_MP3
#if (L1_MIDI == 1)
  #include "l1midi_defty.h"
#endif

 #include "l1_defty.h"
 #include "l1_varex.h"
 #include "l1_macro.h"

#if (L1_DYN_DSP_DWNLD == 1)
  #ifndef NULL
#define NULL 0
  #endif

  extern T_DYN_DWNLD_MCU_DSP *dyn_dwl_ndb;
  #if (CODE_VERSION == SIMULATION)
    extern T_DYN_DWNLD_MCU_DSP dyn_dwl_ndb_sim;
  extern UWORD16 dwnld_area_array[SIZE_DWNLD_AREA_SIMU];
#endif // CODE_VERSION == SIMULATION


/*---------------------------------------------------------------------------- */
/* l1_initialize_patch_parameters                                              */
/*-------------------------------------------------------------------------    */
/*                                                                             */
/* Parameters :                                                                */
/*                                                                             */
/* Return     :    size of the patch                                           */
/*                                                                             */
/* Description :  Initialize patch parameters and returns the size of the patch*/
/*                                                                             */
/*                                                                             */
/*---------------------------------------------------------------------------- */
 
UWORD16 l1_initialize_patch_parameters(void)
{
  UWORD16 patch_size = 0;

  /* Initialize download patch parameters */
  l1_apihisr.dyn_dwnld.running_source_pointer = l1a_apihisr_com.dyn_dwnld.copy_parameters.start_MCU_copy_address[l1a_apihisr_com.dyn_dwnld.copy_parameters.num_of_elem-l1_apihisr.dyn_dwnld.patch_ids_counter];
  patch_size = l1a_apihisr_com.dyn_dwnld.copy_parameters.size_array[l1a_apihisr_com.dyn_dwnld.copy_parameters.num_of_elem-l1_apihisr.dyn_dwnld.patch_ids_counter];
  dyn_dwl_ndb->d_api_dwl_crc = 0x0; 
  return patch_size;
}

/*---------------------------------------------------------------------------- */
/* l1_set_uninstall_parameters                                                 */
/*-------------------------------------------------------------------------    */
/*                                                                             */
/* Parameters : void                                                           */
/*                                                                             */
/* Return     :    void                                                        */
/*                                                                             */
/* Description :  Set uninstall parameters                                     */
/*                                                                             */
/*                                                                             */
/*---------------------------------------------------------------------------- */
 
void l1_set_uninstall_parameters(void)
{
  /* Set next uninstall adress (pointer incremented) */
  dyn_dwl_ndb->d_api_dwl_function_address[0] = (API) (l1a_apihisr_com.dyn_dwnld.uninstall_parameters.address[l1a_apihisr_com.dyn_dwnld.uninstall_parameters.num_of_elem - l1_apihisr.dyn_dwnld.uninstall_counter] & 0x0000FFFF); 
  dyn_dwl_ndb->d_api_dwl_function_address[1] = (API) ((l1a_apihisr_com.dyn_dwnld.uninstall_parameters.address[l1a_apihisr_com.dyn_dwnld.uninstall_parameters.num_of_elem - l1_apihisr.dyn_dwnld.uninstall_counter] >> 16) & 0x0000FFFF); 

  /* Set uninstall command */
  dyn_dwl_ndb->d_api_dwl_download_ctrl = (API) C_DWL_DOWNLOAD_CTRL_UNINSTALL;   
}

/*---------------------------------------------------------------------------- */
/* l1_initialize_pointers_for_copy                                             */
/*-------------------------------------------------------------------------    */
/*                                                                             */
/* Parameters :   address of source and destination pointer                    */
/*                                                                             */
/* Return     :   source and destination address modified by reference         */
/*                                                                             */
/* Description :  Initialize the pointers for the copy                         */
/*                                                                             */
/*                                                                             */
/*---------------------------------------------------------------------------- */

void l1_initialize_pointers_for_copy(UWORD16 **pp_dest_mcu, UWORD16 **pp_src_mcu)
{
  /* BEGIN: Initialize download area parameters at start download area */
  dyn_dwl_ndb->d_api_dwl_write_pointer = l1a_apihisr_com.dyn_dwnld.copy_parameters.start_of_dwnld_area - 1; // correction
    
  /* Initialize pointers */
#if (CODE_VERSION == SIMULATION)
  *pp_dest_mcu = (UWORD16 *) dwnld_area_array;
#else
  *pp_dest_mcu = (UWORD16 *) API_address_dsp2mcu(l1a_apihisr_com.dyn_dwnld.copy_parameters.start_of_dwnld_area);
#endif // CODE_VERSION == SIMULATION

  *pp_src_mcu  = (UWORD16 *) l1_apihisr.dyn_dwnld.running_source_pointer;   
}

/*--------------------------------------------------------*/
/* l1_memcpy_16bit()                                      */
/*--------------------------------------------------------*/
/*                                                        */
/* Description:                                           */
/* ------------                                           */
/* This function is equivalemt of memcopy. Thid function  */
/* does only 8/16 bit accessed to both source and         */
/* destination                                            */
/*                                                        */
/* Input parameter:                                       */
/* ---------------                                        */
/* "src" - input pointer                                  */
/* "len" - number of bytes to copy                        */
/*                                                        */
/* Output parameter:                                      */
/* ----------------                                       */
/*  "dst" - output pointer                                */
/*                                                        */
/*--------------------------------------------------------*/
void l1_memcpy_16bit(void *dst,void* src,unsigned int len)
{
	unsigned int i;
	unsigned int tempLen;
	unsigned char *cdst,*csrc;
	unsigned short *ssrc,*sdst;

	cdst=dst;
	csrc=src;
	sdst=dst;
	ssrc=src;

  if(((unsigned int)src&0x01) || ((unsigned int)dst&0x01)){
  // if either source or destination is not 16-bit aligned do the entire memcopy
  // in 8-bit
    for(i=0;i<len;i++){
      *cdst++=*csrc++;
    }
  }
  else{
    // if both the source and destination are 16-bit aligned do the memcopy
    // in 16-bits
    tempLen = len>>1;
    for(i=0;i<tempLen;i++){
      *sdst++ = *ssrc++;
    }
    if(len & 0x1){
      // if the caller wanted to copy odd number of bytes do a last 8-bit copy
      cdst=(unsigned char*)sdst;
      csrc=(unsigned char*)ssrc;
      *cdst++ = *csrc++;
    }
  }
  return;
}

/*---------------------------------------------------------------------------- */
/* l1_copy_till_the_end_of_the_patch_and_update_write_pointer                  */
/*---------------------------------------------------------------------------- */
/*                                                                             */
/* Parameters : size of the patch, source and destination pointer              */
/*                                                                             */
/* Return     : none                                                           */
/*                                                                             */
/* Description : Copy until the end of the patch is reached                    */
/*                                                                             */
/*                                                                             */
/*---------------------------------------------------------------------------- */

void l1_copy_till_the_end_of_the_patch_and_update_write_pointer(UWORD16 tmp_patch_size, UWORD16* p_dest_mcu, UWORD16* p_src_mcu)
{
  while (tmp_patch_size > NUM_WORDS_COPY_API)
  {
    l1_memcpy_16bit(p_dest_mcu,p_src_mcu, NUM_WORDS_COPY_API*sizeof(UWORD16));
    p_dest_mcu += NUM_WORDS_COPY_API;
    p_src_mcu  += NUM_WORDS_COPY_API;
    tmp_patch_size -= NUM_WORDS_COPY_API;
    dyn_dwl_ndb->d_api_dwl_write_pointer += NUM_WORDS_COPY_API;
  }
  if (tmp_patch_size != 0)
  {  
    l1_memcpy_16bit(p_dest_mcu,p_src_mcu, tmp_patch_size*sizeof(UWORD16));
    dyn_dwl_ndb->d_api_dwl_write_pointer += tmp_patch_size;
  }
}

/*---------------------------------------------------------------------------- */
/* l1_copy_till_end_of_dwnld_area_and_update_write_pointer                     */
/*---------------------------------------------------------------------------- */
/*                                                                             */
/* Parameters : address of size of the patch, size of download area,           */
/*              addresses of source pointer, destination pointer               */
/*                                                                             */
/* Return     : source pointer and size modified by reference                  */
/*                                                                             */
/* Description : Copy until the end of download area is reached                */
/*                                                                             */
/*                                                                             */
/*---------------------------------------------------------------------------- */

void l1_copy_till_end_of_dwnld_area_and_update_write_pointer(UWORD16 tmp_dwnld_area_size,UWORD16 *p_dest_mcu, UWORD16 *p_tmp_patch_size, UWORD16 **pp_src_mcu)
{
  UWORD16 tmp_patch_size = *p_tmp_patch_size;
  UWORD16 *p_src_mcu = (UWORD16 *)*pp_src_mcu;

  while (tmp_dwnld_area_size > NUM_WORDS_COPY_API)
  {
    l1_memcpy_16bit(p_dest_mcu,p_src_mcu, NUM_WORDS_COPY_API*sizeof(UWORD16));
    p_dest_mcu += NUM_WORDS_COPY_API;
    p_src_mcu  += NUM_WORDS_COPY_API;
    tmp_patch_size      -= NUM_WORDS_COPY_API;
    tmp_dwnld_area_size -= NUM_WORDS_COPY_API;
    dyn_dwl_ndb->d_api_dwl_write_pointer += NUM_WORDS_COPY_API;
  }

  if (tmp_dwnld_area_size > 0)
  {
    l1_memcpy_16bit(p_dest_mcu,p_src_mcu, tmp_dwnld_area_size*sizeof(UWORD16));
    p_src_mcu += tmp_dwnld_area_size;  
    tmp_patch_size -= tmp_dwnld_area_size;
    dyn_dwl_ndb->d_api_dwl_write_pointer += tmp_dwnld_area_size;
  }
  *pp_src_mcu = (UWORD16 *) p_src_mcu;
  *p_tmp_patch_size = tmp_patch_size;
}

/*---------------------------------------------------------------------------- */
/* l1_copy_first_N_words                                                       */
/*---------------------------------------------------------------------------- */
/*                                                                             */
/* Parameters : address of size of the patch, address of size of download area,*/
/*              addresses of source pointer, address of destination pointer    */
/*                                                                             */
/* Return     : source and  destination  pointer modified by reference         */
/*              size of download area and patch area modified by reference     */
/*                                                                             */
/* Description : Copy the min(N, remaining size of the patch) at the beginning */
/*               download area                                                 */
/*                                                                             */
/*                                                                             */
/*---------------------------------------------------------------------------- */

BOOL l1_copy_first_N_words (UWORD16 *dwnld_area_size_p, UWORD16 *patch_area_size_p, UWORD16 **pp_dest_mcu, UWORD16 **pp_src_mcu)
{
  BOOL    return_flag;
  UWORD16 num_words_interrupt;
  UWORD16 tmp_patch_size = *patch_area_size_p;
  UWORD16 tmp_dwnld_area_size = *dwnld_area_size_p;
  UWORD16 *p_dest_mcu =(UWORD16 *)*pp_dest_mcu;
  UWORD16 *p_src_mcu = (UWORD16 *)*pp_src_mcu;

  /* Copy first N words and generate API interrupt*/
  if (tmp_patch_size > NUM_WORDS_COPY_API)
  {  
    num_words_interrupt = NUM_WORDS_COPY_API;
    return_flag = TRUE;
  }
  else
  {
    num_words_interrupt = tmp_patch_size;
    return_flag = FALSE;
  }

  l1_memcpy_16bit(p_dest_mcu,p_src_mcu, num_words_interrupt*sizeof(UWORD16));

  p_dest_mcu += num_words_interrupt;
  p_src_mcu  += num_words_interrupt;

  tmp_patch_size      -= num_words_interrupt;
  tmp_dwnld_area_size -= num_words_interrupt;
  
  dyn_dwl_ndb->d_api_dwl_write_pointer+=num_words_interrupt;
  
  *patch_area_size_p = tmp_patch_size;
  *dwnld_area_size_p = tmp_dwnld_area_size;
  *pp_dest_mcu       = (UWORD16 *)p_dest_mcu;
  *pp_src_mcu        = (UWORD16 *)p_src_mcu ;
  
  return return_flag;
}

/*---------------------------------------------------------------------------- */
/* l1_initialize_download_area_parameters                                      */
/*---------------------------------------------------------------------------- */
/*                                                                             */
/* Parameters : none                                                           */
/*                                                                             */
/*                                                                             */
/* Return     : download area size                                             */
/*                                                                             */
/* Description : Initialize download area: all the parameters                  */
/*                                                                             */
/*                                                                             */
/*---------------------------------------------------------------------------- */

UWORD16 l1_initialize_download_area_parameters(void)
{
  UWORD16 dwnld_area_size = 0;

  /* Set download address and size in API-DSP com */
  dyn_dwl_ndb->d_api_dwl_function_address[0] = l1a_apihisr_com.dyn_dwnld.copy_parameters.start_of_dwnld_area;
  dyn_dwl_ndb->d_api_dwl_function_address[1] = 0x0;      

#if (CODE_VERSION == SIMULATION)
  dyn_dwl_ndb->d_api_dwl_size = SIZE_DWNLD_AREA_SIMU;
#else
  dyn_dwl_ndb->d_api_dwl_size = l1a_apihisr_com.dyn_dwnld.copy_parameters.size_of_dwnld_area;
#endif  // CODE_VERSION == SIMULATION

  dwnld_area_size = l1a_apihisr_com.dyn_dwnld.copy_parameters.size_of_dwnld_area;
  return dwnld_area_size;
}

/*---------------------------------------------------------------------------- */
/* l1_init_pointers_and_copy_first_block_of_data                               */
/*---------------------------------------------------------------------------- */
/*                                                                             */
/* Parameters : address of size of the patch, address of size of download area,*/
/*              addresses of source pointer, address of destination pointer    */
/*              new patch flag                                                 */
/* Return     : TRUE if N< size of patch, FALSE otherwise                      */
/*              source and  destination  pointer modified by reference         */
/*              size of download area and patch area modified by reference,    */
/*                                                                             */
/* Description : Initialize pointers and starts the copy.                      */
/*                                                                             */
/*                                                                             */
/*                                                                             */
/*---------------------------------------------------------------------------- */

BOOL l1_init_pointers_and_copy_first_block_of_data(UWORD16 *dwnld_area_size_p, UWORD16 *patch_size_p, UWORD16 **pp_dest_mcu, UWORD16 **pp_src_mcu, BOOL new_patch)
{
  BOOL return_flag;

  /* Initialize download area*/
  *dwnld_area_size_p = l1_initialize_download_area_parameters();

  /* In case this is a new patch, initialize patch parameters*/
  if (new_patch == TRUE)
    *patch_size_p = l1_initialize_patch_parameters();

  /* Initialize pointers for the copy*/
  l1_initialize_pointers_for_copy(pp_dest_mcu, pp_src_mcu);

  /* If this is a new patch, the header of the patch must be taken off from the copy*/
  if (new_patch == TRUE)
  {
    /* Take the initial header off */
    *pp_src_mcu= (*pp_src_mcu)+HEADER_PATCH_SIZE;
    *patch_size_p= (*patch_size_p)-HEADER_PATCH_SIZE;
  }
    
  /* Copy first N words; if remaining size of the patch is smaller than N copy until the end of the patch */
  /* In this case, return FALSE as there are no more words to be copied*/
  return_flag = l1_copy_first_N_words (dwnld_area_size_p, patch_size_p, pp_dest_mcu, pp_src_mcu);
    
  return return_flag;
}


/*---------------------------------------------------------------------------- */
/* l1_set_dyn_dwnld_install_vect                                               */
/*---------------------------------------------------------------------------- */
/*                                                                             */
/* Parameters : size vector, destination address vector, crc vector            */
/*              patch code vector, identifier i of the patch                   */
/*              new patch flag                                                 */
/* Return     : TRUE operation is successful, FALSE otherwise                  */
/*                                                                             */
/* Description : Compute address, size and crc of i-th patch                   */
/*                                                                             */
/*---------------------------------------------------------------------------- */

BOOL l1_set_dyn_dwnld_install_vect(UWORD16* size_p, UWORD32* dest_addr, UWORD16* crc_vect, const UWORD8 *patch_array_code, UWORD16 i)
{
  UWORD16 full_size_32_bit, size, size_ext, dsp_addr, dsp_addr_ext, crc_value;
  UWORD16 *codePtr;
  UWORD16 size_total = 0;
  BOOL status_flag = FALSE;
  codePtr = (UWORD16 *) patch_array_code;
  
  if ( (0 == *codePtr++) && (0 == *codePtr++)) 
  {  // NULL TAG detection
    if ( (3 == *codePtr++) && (0 == *codePtr++)) 
    {   // coff2c version number detection
      size = *codePtr++;
      size_ext        = *codePtr++; 
   
      // first header:4
      size_total+=HEADER_PATCH_SIZE; 
      while(size != 0 || size_ext != 0)
      {    
        full_size_32_bit = (size_ext << 16) + size;
        // reconstruction of the total 32-bit size of the section
        size_total+=HEADER_PATCH_SIZE+full_size_32_bit+1;
        // Header + size_block + cntrl_word(0xDEAD):1
        codePtr+=full_size_32_bit+2+1; 
        // Two words for address and one for the cntrl word 0xDEAD
        size = *codePtr++;
        size_ext = *codePtr++;
      }
      size_total+=HEADER_PATCH_SIZE;
      // Last header to consider

      dsp_addr = *codePtr++;
      dsp_addr_ext = *codePtr++;
      crc_value = *codePtr++;
      dest_addr[i] = (dsp_addr_ext << 16)+dsp_addr;
      crc_vect[i] = crc_value;
      size_p[i] = size_total;
      status_flag = TRUE;
    }
  }
  return status_flag;
}
#endif //L1_DYN_DSP_DWNLD == 1

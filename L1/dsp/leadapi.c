/********************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
  Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                 
                                                           
   Filename       	: leadapi.c

   Description    	: Boot the LEAD through the API
			  Target : Arm

   Project        	: 

   Author         	: A0917556

   Version number	: 1.7

   Date and time	: 01/30/01 10:22:25

   Previous delta 	: 12/19/00 14:27:48

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/RELEASE_GPRS/drivers1/common/SCCS/s.leadapi.c

   Sccs Id  (SID)       : '@(#) leadapi.c 1.7 01/30/01 10:22:25 '

 
*****************************************************************************/


#define LEADAPI_C  1

#include "../../include/config.h"
#include "../../include/sys_types.h"

#include "../../bsp/mem.h"
#include "../../bsp/clkm.h"
#include "leadapi.h"


void LA_ResetLead(void) 
{
   (*(SYS_UWORD16 *) CLKM_CNTL_RST) |= CLKM_LEAD_RST; 
}

/*
 * LA_StartLead 
 *
 * Parameter : pll is the value to set in the PLL register
 */
void LA_StartLead(SYS_UWORD16 pll)
{
   volatile int j;

#if ((CHIPSET == 2) || (CHIPSET == 3) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 9))
   // Set PLL    
   (*(SYS_UWORD16 *) CLKM_LEAD_PLL_CNTL) = pll;

   // Wait 100 microseconds for PLL to start
   wait_ARM_cycles(convert_nanosec_to_cycles(100000));
#endif

   (*(SYS_UWORD16 *) CLKM_CNTL_RST) &= ~CLKM_LEAD_RST; 
}


/*
 * LA_InitialLeadBoot16
 *
 * For RAM-based LEAD
 * 
 * Copy all sections to API 
 * Dedicated with coff2c with 16-bit size and address (used with coff version 1 until DSP v1110)
 */
void LA_InitialLeadBoot16(const unsigned char bootCode[]) 
{
   int i;
   SYS_UWORD16 *origin;
   SYS_UWORD16 *destination;
   SYS_UWORD16 *currentSection;

   (*(SYS_UWORD16 *) CLKM_CNTL_RST) |= CLKM_LEAD_RST; // Reset Lead 

   currentSection = (SYS_UWORD16*) bootCode;
   
   while (*currentSection != 0)     // *currentSection is "size"
   {
      origin = currentSection + 2;  // origin points on 1st word  of current section
      destination = (SYS_UWORD16 *)
         (BASE_API_ARM + ((*(currentSection+1) - BASE_API_LEAD) * 2));
                                    // destination is "addr" in API 
                                    // (*(currentSection+1) is "size" of current section

      for (i=0 ; i< *currentSection ; i++) 
      {
          *destination = *origin++;
          destination = destination + 1;  // destination is UNSIGNED16
      }
      currentSection = origin;
   }
}


/*
 * LA_InitialLeadBoot
 *
 * For RAM-based LEAD
 * 
 * Copy all sections to API 
 * Dedicated with coff2c with 32-bit size and address (perl or v3 version used with coff version 2 from v1500)
 * 
 */
void LA_InitialLeadBoot(const unsigned char bootCode[]) 
{
  int i;
  short error = NULL;
  SYS_UWORD16 size, size_ext;                // dsp_addr[0:15] and dsp_addr[16:31] of the current section as specified in bootCode[] array
  SYS_UWORD16 dsp_address, dsp_ext_address;  // size[0:15] and size[16:31] of the current section as specified in bootCode[] array
  SYS_UWORD16 *bootCodePtr, *destinationPtr; // pointer in bootCode[] array and pointer in the API (MCU addresses)
  
  (*(SYS_UWORD16 *) CLKM_CNTL_RST) |= CLKM_LEAD_RST;         // reset Lead 
  
  bootCodePtr = (SYS_UWORD16 *) bootCode;                    // initialisation of bootCodePtr on the first word of the C array
  
  if ( (NULL == *bootCodePtr++) && (NULL == *bootCodePtr++) ) { // NULL TAG detection
    
    if ( ( 3 == *bootCodePtr++) && (NULL == *bootCodePtr++) ) { // coff2c version number detection 
      
      // initialization for the first section
      size            = *bootCodePtr++;
      size_ext        = *bootCodePtr++;
      dsp_address     = *bootCodePtr++;
      dsp_ext_address = *bootCodePtr++;
      
      while (size != NULL) {                                    // loop until last section whose size is null
	if ( (NULL == size_ext) && (NULL == dsp_ext_address) ) {// size and address must 16-bit values in LA_InitialLeadBoot() 
	  destinationPtr = (SYS_UWORD16 *) (BASE_API_ARM + (dsp_address - BASE_API_LEAD) * 2); // destination in API from the MCU side
	  
	  for (i=0 ; i<size ; i++) {
	    *destinationPtr++ = *bootCodePtr++;
	  }
	  
	  // next section
	  size            = *bootCodePtr++;
	  size_ext        = *bootCodePtr++;
	  dsp_address     = *bootCodePtr++;
	  dsp_ext_address = *bootCodePtr++;
	}
	else {
	  error   = LA_BAD_EXT_VALUE;
	  size    = NULL;
	}
      }
    }
    else {
      error = LA_BAD_VERSION;
    }
  }
  else {
    error = LA_BAD_TAG;
  }
  
  if (error != NULL) {                                          // if an error was detected in the coff2c format,
    LA_InitialLeadBoot16(bootCode);                             // try to download a coff-v1.0 coff2c output
  }
}


/*
 * LA_LoadPage
 * 
 * Final LEAD boot - needs to communicate with initial LEAD Boot program
 *
 * Copy all sections to API 
 *
 * Parameters : pointer to code, LEAD page, flag to start executing
 * Return value : 0 for success, 1 for timeout
 */
short LA_LoadPage(const unsigned char code[], SYS_UWORD16 page, SYS_UWORD16 start) 
{
  int t;                                // time counter for synchronization
  SYS_UWORD16 stat;                  // status parameter for synchronization
  int i       = NULL;
  short error = NULL;
  SYS_UWORD16 current_block_size;    // size of the current block to be copied into API
  int remaining_size32;                 // remaining size of the current section to be copied after the last block
  int remaining_size_DSPpage32;         // size remaining in the current DSP page
  int max_block_size;    //biggest block size used during the patch download
  SYS_UWORD16 size, size_ext;                // size[0:15] and size[16:31] of the current section as specified in code[] array
  SYS_UWORD16 dsp_address, dsp_ext_address;  // dsp_addr[0:15] and dsp_addr[16:31] of the current section as specified in code[] array
  SYS_UWORD16 *codePtr, *destinationPtr;     // pointer in code[] array and pointer in the API (MCU addresses)
    
  codePtr = (SYS_UWORD16 *) code;                    // initialisation of codePtr on the first word of the C array
  max_block_size = 0;
  
  if ( (NULL == *codePtr++) && (NULL == *codePtr++)) {  // NULL TAG detection
    
    if ( (3 == *codePtr++) && (NULL == *codePtr++)) {   // coff2c version number detection 
      
      // Set the data page 
      //-------------------
      // Wait until LEAD is ready
      t = 0;
      do
	  {
	    stat = *((volatile SYS_UWORD16 *) DOWNLOAD_STATUS);
	    t++;
	    if (t > LA_TIMEOUT)
	      return(LA_ERR_TIMEOUT);
	  }
      while (stat != LEAD_READY);
      
      destinationPtr  = (SYS_UWORD16 *) BASE_API_ARM;
      *destinationPtr = page;
      *(volatile SYS_UWORD16 *) DOWNLOAD_STATUS = PAGE_SELECTION;
   
      // Code/Data download block by block
      //-----------------------------------
      do {                                                      // SECTION BY SECTION COPY
	    size            = *codePtr++;
        size_ext        = *codePtr++;
        dsp_address     = *codePtr++;
        dsp_ext_address = *codePtr++;
      
        remaining_size32 = (size_ext << 16) + size;             // reconstruction of the total 32-bit size of the section

        while (remaining_size32) {                              // BLOCK BY BLOCK COPY

          // Wait until LEAD is ready
          t = 0;
          do
          {
            stat = *((volatile SYS_UWORD16 *) DOWNLOAD_STATUS);
            t++;
            if (t > LA_TIMEOUT)
              return(LA_ERR_TIMEOUT);    
          }
          while (stat != LEAD_READY);
	  
          // DSP address managment including the extended page
          remaining_size_DSPpage32 = MAX_UINT - dsp_address +1; // calculate the max available size in the current DSP page (MAX_UINT=65535)
          if (NULL == remaining_size_DSPpage32) {
            dsp_address = 0;                                    // continue on the next DSP page
            dsp_ext_address += 1;
          }

          // partitionning of the current section into blocks
          if (remaining_size32 >= MAX_BLOCK_SIZE) { 
            if (MAX_BLOCK_SIZE <= remaining_size_DSPpage32)
              current_block_size = MAX_BLOCK_SIZE;              // block by block partitioning
            else 
              current_block_size = remaining_size_DSPpage32;
          }
          else { 
            if(remaining_size32 <= remaining_size_DSPpage32)
              current_block_size = remaining_size32;             // the end of the section fits and is copied
            else
              current_block_size = remaining_size_DSPpage32;
          }
	  
          if ( current_block_size > max_block_size )
          {
            max_block_size = current_block_size;
          }
          
          // set parameters in the share memory
          *(volatile SYS_UWORD16 *) DOWNLOAD_SIZE     = current_block_size;
          *(volatile SYS_UWORD16 *) DOWNLOAD_ADDR     = dsp_address;
          *(volatile SYS_UWORD16 *) DOWNLOAD_EXT_PAGE = dsp_ext_address;

          // perform the copy
          destinationPtr  = (SYS_UWORD16 *) BASE_API_ARM;
          for (i=0 ; i<current_block_size ; i++) {
            *destinationPtr++ = *codePtr++;
          }
	  
          // synchronize and prepare the next step
          *(volatile SYS_UWORD16 *) DOWNLOAD_STATUS = BLOCK_READY;
          dsp_address      += current_block_size;
          remaining_size32 -= current_block_size;
        }
      } while ( (size != NULL) || (size_ext != NULL) );
      
      // Setting of the starting address if required
      //---------------------------------------------
      // Wait until LEAD is ready
      t = 0;
      do
      {
        stat = *((volatile SYS_UWORD16 *) DOWNLOAD_STATUS);
        t++;
        if (t > LA_TIMEOUT)
          return(LA_ERR_TIMEOUT);    
      }
      while (stat != LEAD_READY);

      /* the part of the API used for the download must be reseted at end of download */
      /* in case some values are not initialized within API before DSP start:*/
      /* DSP start after DOWNLOAD_SIZE is set to zero.*/
      destinationPtr  = (SYS_UWORD16 *) BASE_API_ARM;
      for (i=0 ; i<max_block_size ; i++) {
        *destinationPtr++ = 0x0000;
      }

      if (start)
      {
        /* Set the last block, which is the starting address */   
        *(volatile SYS_UWORD16 *) DOWNLOAD_SIZE     = 0;
        *(volatile SYS_UWORD16 *) DOWNLOAD_ADDR     = dsp_address;
        *(volatile SYS_UWORD16 *) DOWNLOAD_EXT_PAGE = dsp_ext_address;
        *(volatile SYS_UWORD16 *) DOWNLOAD_STATUS   = BLOCK_READY;
      }
      
      return(LA_SUCCESS);
    }
    else {
      error = LA_BAD_VERSION;
    }
  }
  else {
    error = LA_BAD_TAG;
  }

  if (error != NULL)                                  // if an error was detected in the coff2c format,
    error = LA_LoadPage16(code, page, start);         // try to download a coff-v1.0 coff2c output
    return(error);                                    // and return its result
}


/*
 * LA_LoadPage16
 * 
 * Final LEAD boot - needs to communicate with initial LEAD Boot program
 *
 * Copy all sections to API 
 *
 * Parameters : pointer to code, LEAD page, flag to start executing
 * Return value : 0 for success, 1 for timeout
 */
short LA_LoadPage16(const unsigned char code[], SYS_UWORD16 page, SYS_UWORD16 start) 
{
   int i = 0;
   int remainingSize, currentBlockSize;
   volatile int j;
   int t;
   int max_block_size;    //biggest block size used during the patch download

   volatile SYS_UWORD16 *origin;
   volatile SYS_UWORD16 *destination;
   volatile SYS_UWORD16 *currentSection;
   SYS_UWORD16 addr, size, stat;

   currentSection = (SYS_UWORD16*) code;   /* Take GSM application s/w */
   max_block_size = 0;

   // Set the data page if needed
   if (page == 1)
   {
      // Wait until LEAD is ready
      t = 0;
      do
      {
         stat = *((volatile SYS_UWORD16 *) DOWNLOAD_STATUS);
         t++;
         if (t > LA_TIMEOUT)
            return(LA_ERR_TIMEOUT);
            
      }
      while ( stat != LEAD_READY);

      destination = (volatile SYS_UWORD16 *) BASE_API_ARM;
      *destination = 1;
      *(volatile SYS_UWORD16 *) DOWNLOAD_STATUS = PAGE_SELECTION;
   }
   

   do
   {                                   /* while there is a section to transfer */
      origin = currentSection + 2;
      size = *currentSection;
      addr = *(currentSection+1);

      remainingSize = size;
      
      while (remainingSize) 
      {  
         if (remainingSize > MAX_BLOCK_SIZE) 
            currentBlockSize = MAX_BLOCK_SIZE;
         else 
            currentBlockSize = remainingSize;

         /* Wait until LEAD is ready */
         t = 0;
         do
         {
            stat = *((volatile SYS_UWORD16 *) DOWNLOAD_STATUS);
            t++;
            if (t > LA_TIMEOUT)
               return(LA_ERR_TIMEOUT);
               
         }
         while (stat != LEAD_READY);

         /* Set the block size and address in shared memory */   
         *(volatile SYS_UWORD16 *) DOWNLOAD_SIZE = currentBlockSize;
         *(volatile SYS_UWORD16 *) DOWNLOAD_ADDR = addr + size - remainingSize;

         if ( currentBlockSize > max_block_size )
         {
             max_block_size = currentBlockSize;
         }

         /* Copy the block */
         destination = (volatile SYS_UWORD16 *) BASE_API_ARM;
         for (i=0 ; i< currentBlockSize ; i++) 
         {
            *destination = *origin++;     
            destination += 1;  // API is really 16-bit wide for MCU now !
         }

         *(volatile SYS_UWORD16 *) DOWNLOAD_STATUS = BLOCK_READY;

         remainingSize -= currentBlockSize;
      }
      currentSection = origin;
   }
   while (size != 0);

   /* Wait until LEAD is ready */
   t = 0;
   do
   {
      stat = *((volatile SYS_UWORD16 *) DOWNLOAD_STATUS);
      t++;
      if (t > LA_TIMEOUT)
         return(LA_ERR_TIMEOUT);
         
   }
   while (stat != LEAD_READY);

  
   /* the part of the API used for the download must be reseted at end of download */
   /* in case some values are not initialized within API before DSP start:*/
   /* DSP start after DOWNLOAD_SIZE is set to zero.*/
   destination  = (SYS_UWORD16 *) BASE_API_ARM;
   for (i=0 ; i<max_block_size ; i++) {
      *destination++ = 0x0000;
   }

   if (start)
   {
      /* Set the last block, which is the starting address */   
      *(volatile SYS_UWORD16 *) DOWNLOAD_SIZE = 0;
      *(volatile SYS_UWORD16 *) DOWNLOAD_ADDR = addr;
      *(volatile SYS_UWORD16 *) DOWNLOAD_STATUS = BLOCK_READY;
   }

   return(LA_SUCCESS);
}

/*
 * LeadBoot
 *
 * Start the LEAD without downloading any code
 */
short LeadBoot(SYS_UWORD16 entryPoint, SYS_UWORD16 pll)
{
   SYS_UWORD16 section[2];
   short res;
   
   section[0] = 0;      // null size 
   section[1] = entryPoint;
   
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
   CLKM_RELEASELEADRESET;
#else
   LA_StartLead(pll);
#endif
   
   res = LA_LoadPage((const unsigned char *) section, 0, 1);
   return(res);
   
}


/*
 * LA_ReleaseLead
 *
 */
void LA_ReleaseLead(void)
{
  (*(unsigned short *) CLKM_CNTL_RST) &= ~CLKM_LEAD_RST;
}

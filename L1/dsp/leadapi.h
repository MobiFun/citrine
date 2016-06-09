/*******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
  Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                 
                                                           
   Filename       	: leadapi.h

   Description    	: Boot the LEAD - header file

   Target               : ARM

   Project        	: 

   Author         	: A0917556

   Version number	: 1.8

   Date and time	: 02/22/01 11:54:48

   Previous delta 	: 01/22/01 10:32:42

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/RELEASE_GPRS/drivers1/common/SCCS/s.leadapi.h

   Sccs Id  (SID)       : '@(#) leadapi.h 1.8 02/22/01 11:54:48 '


*****************************************************************************/

#undef  NULL		// appease gcc
#define	NULL          0

#define APIF_ADDR     0xFFD00000L
#define BASE_API_ARM  APIF_ADDR     /* API RAM for ARM */
#if (CHIPSET == 4)
  #define BASE_API_LEAD 0xE800      /* API RAM for Lead */
#elif (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12)
  #define BASE_API_LEAD 0xE000      /* API RAM for Lead */
#else
  #define BASE_API_LEAD 0xF800      /* API RAM for Lead */
#endif

#define DOWNLOAD_EXT_PAGE   (APIF_ADDR + 0x0FF8)   /* Address of the extended DSP page of the */
#define DOWNLOAD_SIZE       (APIF_ADDR + 0x0FFA)   /* Address of the download size variable */
#define DOWNLOAD_ADDR       (APIF_ADDR + 0x0FFC)   /* Address of the download address variable */
#define DOWNLOAD_STATUS     (APIF_ADDR + 0x0FFE)   /* Address of the download status variable */

/* Maximum size of a block which can be copied into the API RAM */

#define MAX_BLOCK_SIZE 0x7F0       
#define MAX_UINT       65535   

/* Possible values for the download status */

#define LEAD_READY      1
#define BLOCK_READY     2
#define PROGRAM_DONE    3
#define PAGE_SELECTION  4


// Constants for timeout
#define LA_TIMEOUT      10000
#define LA_SUCCESS      0
#define LA_ERR_TIMEOUT  1

// Constants for error
#define LA_BAD_EXT_VALUE  2
#define LA_BAD_VERSION    3
#define LA_BAD_TAG        4

// Prototypes
void LA_InitialLeadBoot16(const unsigned char bootCode[]);
void LA_InitialLeadBoot(const unsigned char bootCode[]);
short  LA_LoadPage16(const unsigned char code[], SYS_UWORD16 page, SYS_UWORD16 start);
short  LA_LoadPage(const unsigned char code[], SYS_UWORD16 page, SYS_UWORD16 start);
void LA_ResetLead(void);
void LA_StartLead(SYS_UWORD16 pll);

extern const unsigned char bootCode[];

/*
 * Global variables
 */ 
#ifdef LEADAPI_C
#define LA_GLOBAL 
#else
#define LA_GLOBAL extern
#endif

LA_GLOBAL volatile unsigned leadInt;

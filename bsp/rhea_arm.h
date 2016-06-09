/******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
   Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                  
                                                           
   Filename       	: rhea_arm.h

   Description    	: Header file for the ARM RHEA interface

   Project        	: drivers

   Author         	: pmonteil@tif.ti.com  Patrice Monteil.

   Version number	: 1.5

   Date and time	: 01/30/01 10:22:28

   Previous delta 	: 12/08/00 11:38:10

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/RELEASE_GPRS/drivers1/common/SCCS/s.rhea_arm.h

   Sccs Id  (SID)       : '@(#) rhea_arm.h 1.5 01/30/01 10:22:28 '

 
*****************************************************************************/

#include "../include/config.h"

/**** RHEA control register ****/

#define RHEA_CNTL_FACT_0   0x000f	/* Division factor for strobe 0 */
#define RHEA_CNTL_FACT_1   0x00f0	/* Division factor for strobe 1 */
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define RHEA_CNTL_TIMEOUT  0xff00 	/* RHEA bus access timeout */
#else
  #define RHEA_CNTL_TIMEOUT  0xfe00 	/* RHEA bus access timeout */
#endif

/**** API control register ****/

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define RHEA_API_WS_H   0x001f		/* API wait states when DSP is in HOM mode */
  #define RHEA_API_WS_S   0x03e0		/* API wait states when DSP in in SAM mode */
#else
  #define RHEA_API_WS_H   0x001f		/* API wait states for High clkout */
  #define RHEA_API_WS_L   0x02e0		/* API wait states for Low clkout */
#endif

/**** ARM RHEA control register ****/

#define RHEA_ARM_WEN_0   0x0001		/* Write enable for strobe 0 */
#define RHEA_ARM_WEN_1   0x0002		/* Write enable for strobe 1 */

/*---------------------------------------------------------------/
/*  RHEA_INITRHEA()						*/
/*--------------------------------------------------------------*/
/* Parameters :Fac0 acces factor strb0, Fac1 acces factor strb1	*/
/*		timeout max time periph stall the processor	*/
/* Return     :	none						*/
/* Functionality :Initialize the RHEA control register		*/
/*--------------------------------------------------------------*/

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define RHEA_INITRHEA(Fac0,Fac1,TimeOut) (* (unsigned short *) MEM_RHEA_CNTL = (Fac0 | Fac1 << 4 | TimeOut << 8))
#else
  #define RHEA_INITRHEA(Fac0,Fac1,TimeOut) (* (unsigned short *) MEM_RHEA_CNTL = (Fac0 | Fac1 << 4 | TimeOut << 9))
#endif

/*---------------------------------------------------------------/
/*  RHEA_INITAPI()						*/
/*--------------------------------------------------------------*/
/* Parameters :wsH wait states when freq high, wsL wait states	*/
/*		when freq low					*/
/* Return     :	none						*/
/* Functionality :Initialize the API control register		*/
/*--------------------------------------------------------------*/

#define RHEA_INITAPI(wsH, wsL) (* (SYS_UWORD16 *) MEM_API_CNTL = ((wsH) | (wsL) << 5))

/*---------------------------------------------------------------/
/*  RHEA_INITARM()						*/
/*--------------------------------------------------------------*/
/* Parameters : Wen0 write enable domain strb0			*/
/*		Wen1 write enable domain strb1			*/
/* Return     :	none						*/
/* Functionality :Initialize the ARM RHEA control register	*/
/*--------------------------------------------------------------*/

#define RHEA_INITARM(Wen0,Wen1) (* (SYS_UWORD16 *) MEM_ARM_RHEA = ((Wen0) | (Wen1) << 1))
 
/* ----- Prototypes ----- */

short RHEA_GetRHEA(unsigned short *Fac0, unsigned short *Fac1, unsigned short *TimeOut);
short RHEA_GetAPI(unsigned short *wsL, unsigned short *wsH);
short RHEA_GetARM(unsigned short *Wen0, unsigned short *Wen1);

/*******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
   Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                  
                                                           
   Filename       	: inth.c

   Description    	: inth.c saturn interupt handler

   Project        	: drivers

   Author         	: pmonteil@tif.ti.com  Patrice Monteil.

   Version number	: 1.8

   Date and time	: 01/30/01 10:22:26

   Previous delta 	: 12/19/00 14:39:21

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/RELEASE_GPRS/drivers1/common/SCCS/s.inth.c

   Sccs Id  (SID)       : '@(#) inth.c 1.8 01/30/01 10:22:26 '

 
*****************************************************************************/

#include "../include/config.h"
#include "../include/sys_types.h"

#include  "mem.h"
#include  "inth.h"




/*---------------------------------------------------------------/
/*  INTH_Ack()							*/
/*--------------------------------------------------------------*/
/* Parameters : num of it					*/
/* Return     :	none						*/
/* Functionality :Acknowledge an interrupt and return the origin*/
/*	of the interrupt (binary format)			*/
/*--------------------------------------------------------------*/

SYS_UWORD16 INTH_Ack (int intARM)
{
   if (intARM == INTH_IRQ)
	return((* (volatile SYS_UWORD16 *) INTH_B_IRQ_REG) & INTH_SRC_NUM);
   else
	return((* (volatile SYS_UWORD16 *) INTH_B_FIQ_REG) & INTH_SRC_NUM);
}
	
/*---------------------------------------------------------------/
/*  INTH_GetPending()						*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : Return the pending interrupts 		*/
/*--------------------------------------------------------------*/

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  unsigned long INTH_GetPending (void)
  {
    return((unsigned long)((* (volatile SYS_UWORD16 *) INTH_IT_REG1) | 
           ((* (volatile SYS_UWORD16 *) INTH_IT_REG2) << 16)));
  }		
#else
  unsigned short INTH_GetPending (void)
  {
    return(* (volatile SYS_UWORD16 *) INTH_IT_REG);
  }		
#endif

/*---------------------------------------------------------------/
/*  INTH_InitLevel()						*/
/*--------------------------------------------------------------*/
/* Parameters : num it,FIQ/IRQ,priority level,edge/level.	*/
/* Return     :	none						*/
/* Functionality : Initialize an interrupt			*/
/*		   - put it on IRQ or FIQ			*/
/*		   - set its priority level			*/
/*						 		*/
/*--------------------------------------------------------------*/

void INTH_InitLevel (int inputInt, int FIQ_nIRQ, int priority, int edge)
{
   volatile SYS_UWORD16 *inthLevelReg = (SYS_UWORD16 *) INTH_EXT_REG; 

   inthLevelReg = inthLevelReg + inputInt;

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
   *inthLevelReg = (FIQ_nIRQ | (edge << 1) | (priority << 2));
#else
   *inthLevelReg = (FIQ_nIRQ | (priority << 1) | (edge << 5));
#endif
}

/*---------------------------------------------------------------/
/*  INTH_ResetIT()						*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : reset the inth it register			*/
/*--------------------------------------------------------------*/

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  unsigned long INTH_ResetIT (void)
  {
    * (volatile SYS_UWORD16 *) INTH_IT_REG1 &= 0x0000; 
    * (volatile SYS_UWORD16 *) INTH_IT_REG2 &= 0x0000; 
    return((unsigned long)((* (volatile SYS_UWORD16 *) INTH_IT_REG1) | 
           ((* (volatile SYS_UWORD16 *) INTH_IT_REG2) << 16)));
  }
#else
  unsigned short INTH_ResetIT (void)
  {
    * (volatile SYS_UWORD16 *) INTH_IT_REG &= 0x0000; 
    return(* (volatile SYS_UWORD16 *) INTH_IT_REG);
  }
#endif

/******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
   Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                  
                                                           
   Filename       	: timer2.c

   Description    	: Set of useful functions for TIMER module test cases 

   Project        	: drivers

   Author         	: pmonteil@tif.ti.com  Patrice Monteil.
   Version number	: 1.3

   Date and time	: 08/18/00 14:42:39

   Previous delta 	: 07/10/00 16:15:18

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/RELEASE_GPRS/drivers1/common/SCCS/s.timer2.c

   Sccs Id  (SID)       : '@(#) timer2.c 1.3 08/18/00 14:42:39 '


 
*****************************************************************************/

#include "../include/sys_types.h"

#include "mem.h" 
#include "timer2.h" 

/*--------------------------------------------------------------*/
/*  Dtimer2_Get_cntlreg()					*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : read one of the TIMER register		*/
/*--------------------------------------------------------------*/

SYS_UWORD16 Dtimer2_Get_cntlreg(void)
{
    SYS_UWORD16 timerReg;

    timerReg = (( * (volatile SYS_UWORD16 *) D_TIMER_CNTL) & D_TIMER_CNTL_MASK);
    return(timerReg);
}


/*--------------------------------------------------------------*/
/*  Dtimer2_Init_cntl()						*/
/*--------------------------------------------------------------*/
/* Parameters :start/stop,reload yes/no,pre scale,ext clk on/off*/
/* Return     :	none						*/
/* Functionality : initialize the timer start, autoreload, Ptv 	*/
/*		   and clock enable				*/
/*--------------------------------------------------------------*/

void Dtimer2_Init_cntl (SYS_UWORD16 St, SYS_UWORD16 Reload, SYS_UWORD16 clockScale, SYS_UWORD16 clkon)
{
    SYS_UWORD16 cntl = * (volatile SYS_UWORD16 *) D_TIMER_CNTL; 


    cntl &= ~(D_TIMER_ST | D_TIMER_PTV);    	/* stop and reset values */

    (Reload) ? (cntl |= D_TIMER_AR) : (cntl &= ~D_TIMER_AR);

    cntl |= (clockScale << 2 );

    cntl |= ( clkon << 5 );


    * (volatile SYS_UWORD16 *) D_TIMER_LOAD = St;

    * (volatile SYS_UWORD16 *) D_TIMER_CNTL = cntl;
    }

/*--------------------------------------------------------------*/
/*  Dtimer2_AR()						*/
/*--------------------------------------------------------------*/
/* Parameters :start/stop,reload yes/no,pre scale,ext clk on/off*/
/* Return     :	none						*/
/* Functionality : set the AR bit Ar = 0 | 1			*/
/*--------------------------------------------------------------*/

void Dtimer2_AR(SYS_UWORD16 Ar)
{
SYS_UWORD16 cntl = * (volatile SYS_UWORD16 *) D_TIMER_CNTL; 

 cntl &= ~(D_TIMER_ST);			/* stop the timer */

 cntl &= ~(D_TIMER_AR);

 cntl |= ( Ar << 1 );

 * (volatile SYS_UWORD16 *) D_TIMER_CNTL = cntl;
}

/*--------------------------------------------------------------*/
/*  Dtimer2_PTV()						*/
/*--------------------------------------------------------------*/
/* Parameters : pre scale					*/
/* Return     :	none						*/
/* Functionality : set the Ptv bits Ptv = 0 => 7 		*/
/*--------------------------------------------------------------*/

void Dtimer2_PTV(SYS_UWORD16 Ptv)
{
SYS_UWORD16 cntl = * (volatile SYS_UWORD16 *) D_TIMER_CNTL;	
 
 cntl &= ~(D_TIMER_ST);			/* stop the timer */

 cntl &= ~(D_TIMER_PTV);		/* Ptv[4:0] set to 0 */

 cntl |= ( Ptv << 2 );

 * (volatile SYS_UWORD16 *) D_TIMER_CNTL = cntl;
}

/*--------------------------------------------------------------*/
/*     Dtimer1_Clken()						*/
/*--------------------------------------------------------------*/
/* Parameters : ext clk on/off					*/
/* Return     :	none						*/
/* Functionality :  set the clock_enable bit En = 0 | 1 	*/
/*--------------------------------------------------------------*/

void Dtimer2_Clken(SYS_UWORD16 En)
{
SYS_UWORD16 cntl = * (volatile SYS_UWORD16 *) D_TIMER_CNTL;
 
 cntl &= ( ~D_TIMER_CLK_EN);

 cntl |= ( En << 5 );

 * (volatile SYS_UWORD16 *) D_TIMER_CNTL = cntl;
}



/*--------------------------------------------------------------*/
/*  Dtimer2_Start()						*/
/*--------------------------------------------------------------*/
/* Parameters : on/off						*/
/* Return     :	none						*/
/* Functionality :  Start or stop the timer 			*/
/*--------------------------------------------------------------*/

void  Dtimer2_Start (SYS_UWORD16 startStop)
{
    (startStop == D_TIMER_ST) ? 	
	((* (SYS_UWORD16 *) D_TIMER_CNTL) |=  D_TIMER_ST) :	/* condition vrai */
 	((* (SYS_UWORD16 *) D_TIMER_CNTL) &= ~D_TIMER_ST);    	/* condition fausse */
}

/*--------------------------------------------------------------*/
/*  Dtimer2_ReadValue()						*/
/*--------------------------------------------------------------*/
/* Parameters : on/off						*/
/* Return     :	none						*/
/* Functionality :  Read timer value				*/
/*--------------------------------------------------------------*/

SYS_UWORD16 Dtimer2_ReadValue (void)
{
 return(* (SYS_UWORD16 *) D_TIMER_READ);
}

/*--------------------------------------------------------------*/
/* Parameters : on/off						*/
/* Return     :	none						*/
/* Functionality :  Write timer value				*/
/*--------------------------------------------------------------*/

void Dtimer2_WriteValue (SYS_UWORD16 value)
{
 (* (SYS_UWORD16 *) D_TIMER_LOAD) = value;
}

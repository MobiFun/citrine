/*******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
  Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                 
                                                           
   Filename       	: timer1.h

   Description    	:TIMER1 

   Project        	: drivers

   Author         	: pmonteil@tif.ti.com  Patrice Monteil.

   Version number	: 1.7

   Date and time	: 02/15/01 15:47:06

   Previous delta 	: 02/15/01 15:47:06

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/mod/emu_p/EMU_P_FRED/drivers1/common/SCCS/s.timer1.h

   Sccs Id  (SID)       : '@(#) timer1.h 1.7 02/15/01 15:47:06 '

 
*****************************************************************************/

#include "../include/sys_types.h"

/**** DIONE TIMERs configuration register ****/

#define D_TIMER_ADDR        0xfffe3800

#define D_TIMER_CNTL_MASK	0x001f

#define CNTL_D_TIMER_OFFSET	0x0000
#define LOAD_D_TIMER_OFFSET	0x0002
#define READ_D_TIMER_OFFSET	0x0004

#define D_TIMER_CNTL		(D_TIMER_ADDR+CNTL_D_TIMER_OFFSET)
#define D_TIMER_LOAD		(D_TIMER_ADDR+LOAD_D_TIMER_OFFSET)
#define D_TIMER_READ		(D_TIMER_ADDR+READ_D_TIMER_OFFSET)

#define D_TIMER_ST		0x0001		/* bit 0 */
#define D_TIMER_AR		0x0002		/* bit 1 */
#define D_TIMER_PTV		0x001c		/* bits 4:2 */
#define D_TIMER_CLK_EN		0x0020		/* bit 5  */
#define D_TIMER_RUN		0x0021		/* bit 5 ,0 */

#define LOAD_TIM		0xffff		/* bits 15:0 */





/* ----- Prototypes ----- */
SYS_UWORD16 Dtimer1_Get_cntlreg(void);

void Dtimer1_AR(SYS_UWORD16 Ar);

void Dtimer1_PTV(SYS_UWORD16 Ptv);

void Dtimer1_Clken(SYS_UWORD16 En);

void  Dtimer1_Start (SYS_UWORD16 startStop);

void Dtimer1_Init_cntl (SYS_UWORD16 St, SYS_UWORD16 Reload, SYS_UWORD16 clockScale, SYS_UWORD16 clkon);

void Dtimer1_WriteValue (SYS_UWORD16 value);

SYS_UWORD16 Dtimer1_ReadValue (void);

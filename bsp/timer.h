/*******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
   Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                  
                                                           
   Filename         : timer.h

   Description      : timer.c header

   Project          : drivers

   Author           : pmonteil@tif.ti.com  Patrice Monteil.

   Version number   : 1.3

   Date and time    : 07/23/98 16:25:53
   Previous delta   : 07/23/98 16:25:52

   SCCS file        : /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release1.5/mod/emu/EMU_MCMP/eva3_drivers/source/SCCS/s.timer.h

   Sccs Id  (SID)       : '@(#) timer.h 1.3 07/23/98 16:25:53 '

 
*****************************************************************************/

#include "../include/sys_types.h"

#define TIMER_CNTL_REG  MEM_TIMER_ADDR      /* watchdog Control Timer register */
 
#define TIMER_ST        0x0080       
#define TIMER_AR        0x0100       
#define TIMER_PTV       0x0e00   
#define	TIMER_CNTL_MASK	0x0f80	 
#define	TIMER_MODE_MASK	0x80ff	 


#define TIMER_LOAD_REG  (MEM_TIMER_ADDR + 0x02)   /* Timer load register */ 
#define TIMER_READ_REG  (MEM_TIMER_ADDR + 0x02)   /* Timer read register */ 


#define TIMER_MODE_REG  (MEM_TIMER_ADDR + 0x04)   /* Timer mode register */ 
#define TIMER_WDOG      0x8000          /* watch dog */

#define START_STOP      1           /*to start or stop a timer */
#define AR              0x0002       
#define PTV             0x001c 
#define TIMER_CLK_EN    0x0020


#define TIMER1_CNTL         (MEM_TIMER1 + 0x00)
#define TIMER1_LOAD_TIM     (MEM_TIMER1 + 0x02)
#define TIMER1_READ_TIM     (MEM_TIMER1 + 0x04)
#define TIMER2_CNTL         (MEM_TIMER2 + 0x00)
#define TIMER2_LOAD_TIM     (MEM_TIMER2 + 0x02)
#define TIMER2_READ_TIM     (MEM_TIMER2 + 0x04)


/*---------------------------------------------------------------/
/*   TIMER_START_STOP ()						*/
/*--------------------------------------------------------------*/
/* Parameters : start or stop command	 			*/
/* Return     :	none						*/
/* Functionality : Start or Stop the timer  			*/
/*--------------------------------------------------------------*/
#define TIMER_START_STOP(startStop) ((startStop) ? ((* (volatile SYS_UWORD16 *) TIMER_CNTL_REG) |=  TIMER_ST) : \
((* (volatile SYS_UWORD16 *) TIMER_CNTL_REG) &= ~TIMER_ST))


/* Prototype of the functions */

void        TM_ResetTimer (SYS_UWORD16 timerNum, SYS_UWORD16 countValue,
                           SYS_UWORD16 autoReload, SYS_UWORD16 clockScale);
void        TM_StopTimer (int timerNum);
void        TM_StartTimer (int timerNum);
SYS_UWORD16      TM_ReadTimer (int timerNum);

void        TM_DisableWatchdog(void);
void        TM_EnableWatchdog(void);
void        TM_ResetWatchdog(SYS_UWORD16 cnt);
void        TM_EnableTimer (int timerNum);
void        TM_DisableTimer (int timerNum);

SYS_UWORD16 TIMER_Read (unsigned short);
void TIMER_WriteValue (SYS_UWORD16);
unsigned short TIMER_ReadValue (void);


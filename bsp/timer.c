/*******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
   Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                  
                                                           
   Filename         : timer.c

   Description      : timer.c

   Project          : drivers

   Author           : pmonteil@tif.ti.com  Patrice Monteil.

   Version number   : 1.3

   Date and time    : 07/23/98 16:25:32
   Previous delta   : 07/23/98 16:25:32

   SCCS file        : /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release1.5/mod/emu/EMU_MCMP/eva3_drivers/source/SCCS/s.timer.c

   Sccs Id  (SID)       : '@(#) timer.c 1.3 07/23/98 16:25:32 '

 
*****************************************************************************/

#include "../include/sys_types.h"

#include "mem.h"
#include "iq.h"
#include "timer.h"


/*--------------------------------------------------------------
 *  TIMER_Read()
 *--------------------------------------------------------------
 * Parameters :	num of the register to be read
 * Return     :value of the timer register read
 * Functionality :  read one of the TIMER register
 *--------------------------------------------------------------*/
SYS_UWORD16 TIMER_Read (unsigned short regNum)
{
   SYS_UWORD16 timerReg;

   switch (regNum) {
   case 0:
      timerReg = ( * (volatile SYS_UWORD16 *) TIMER_CNTL_REG) & TIMER_CNTL_MASK;
      break;
   case 1:
      timerReg = *(volatile SYS_UWORD16 *) TIMER_READ_REG;
      break;
   case 2:
      timerReg = ( * (volatile SYS_UWORD16 *) TIMER_MODE_REG) & TIMER_MODE_MASK;
      break;
   default:
       break;
   }
   return(timerReg);
}
/*--------------------------------------------------------------
 *  TM_ResetTimer()
 *--------------------------------------------------------------
 * Parameters : timer number (1 or 2) 
 *              timer value, reload yes or not, scale
 * Return     : none
 * Functionality : Give the timewr state
 *--------------------------------------------------------------*/
void  TM_ResetTimer (SYS_UWORD16 timerNum,  SYS_UWORD16 countValue, 
                     SYS_UWORD16 autoReload, SYS_UWORD16 clockScale)
{
     volatile SYS_UWORD16 *cntl;
    
    if (timerNum == 1) 
    {
        cntl = (volatile SYS_UWORD16 *) TIMER1_CNTL; 

        *cntl &= ~(START_STOP | PTV);        /* stop and reset values */

        (autoReload) ? (*cntl |= AR) : (*cntl &= ~AR);

        *cntl |= (clockScale << 2 );

        /*load the value */
        *(volatile SYS_UWORD16 *) TIMER1_LOAD_TIM = countValue;

        *cntl |= START_STOP;    
    }
    else 
    {
        cntl = (volatile SYS_UWORD16 *) TIMER2_CNTL; 

        *cntl &= ~(START_STOP | PTV);        /* stop and reset values */

        (autoReload) ? (*cntl |= AR) : (*cntl &= ~AR);

        *cntl |= (clockScale << 2 );
         
        /*load the value */
        *(volatile SYS_UWORD16 *) TIMER2_LOAD_TIM = countValue;

        *cntl |= START_STOP;    
    }
}

/*
 *  TM_StopTimer
 *
 * Parameters : timer number (1 or 2)
 */
void   TM_StopTimer (int timerNum)
{
    volatile SYS_UWORD16 *cntl;

    if (timerNum == 1) 
    {
        cntl = (volatile SYS_UWORD16 *) TIMER1_CNTL;
    }
    else
    {
        cntl = (volatile SYS_UWORD16 *) TIMER2_CNTL;
    }
    *cntl &= ~START_STOP;
}   

/*
 * TM_ReadTimer
 *
 * Returns current timer value
 *
 * Parameters : timer number (1 or 2)
 *
 */
SYS_UWORD16 TM_ReadTimer (int timerNum)
{
    if (timerNum == 1) 
    {
        return (* (SYS_UWORD16 *) TIMER1_READ_TIM);
    }
    else
    {
        return (* (SYS_UWORD16 *) TIMER2_READ_TIM);
    }
}   


/*
 * TM_StartTimer
 *
 * Parameters : timer number (1 or 2)
 *
 */
void TM_StartTimer (int timerNum)
{
    volatile SYS_UWORD16 *cntl;
    
    if (timerNum == 1) 
    {
        cntl = (volatile SYS_UWORD16 *) TIMER1_CNTL;
    }
    else
    {
        cntl = (volatile SYS_UWORD16 *) TIMER2_CNTL;
    }
    *cntl |= START_STOP;
}   

void TM_DisableWatchdog (void)
{
   /* volatile variable needed due C to optimization */
   volatile SYS_UWORD16 *reg = (volatile SYS_UWORD16 *) TIMER_MODE_REG;

   *reg = 0xf5;                
   *reg = 0xa0;
}


 /*
 * TM_EnableWatchdog
 * 
 */
void TM_EnableWatchdog(void)
{
    * ((volatile SYS_UWORD16 *) TIMER_MODE_REG) = TIMER_WDOG;
}

/*
 * TM_ResetWatchdog
 * 
 * Parameter : Tick count 
 * Use a different value each time, otherwise watchdog bites 
 */
void TM_ResetWatchdog(SYS_UWORD16 count)
{
  * ((volatile SYS_UWORD16 *) TIMER_LOAD_REG) = count;
}

/*
* TM_EnableTimer (int TimerNum)
* 
* Parameter : TimerNum : timer to enable (timer1 or timer2)
*
*/
void TM_EnableTimer (int TimerNum)
{
     volatile SYS_UWORD16 *cntl;
    
    if (TimerNum == 1) 
    {
        cntl = (volatile SYS_UWORD16 *) TIMER1_CNTL;
    }
    else
    {
        cntl = (volatile SYS_UWORD16 *) TIMER2_CNTL;
    }
    *cntl |= TIMER_CLK_EN;
}

/*
* TM_DisableTimer (int TimerNum)
* 
* Parameter : TimerNum : timer to enable (timer1 or timer2)
*
*/
void TM_DisableTimer (int TimerNum)
{
     volatile SYS_UWORD16 *cntl;
    
    if (TimerNum == 1) 
    {
        cntl = (volatile SYS_UWORD16 *) TIMER1_CNTL;
    }
    else
    {
        cntl = (volatile SYS_UWORD16 *) TIMER2_CNTL;
    }
    *cntl &= ~TIMER_CLK_EN;
}

/*--------------------------------------------------------------
 *  TIMER_ReadValue()
 *--------------------------------------------------------------
 * Parameters : none
 * Return     :	none
 * Functionality :  Read timer value
 *--------------------------------------------------------------*/

unsigned short TIMER_ReadValue (void)
{
  return(* (SYS_UWORD16 *) TIMER_READ_REG);
}

/*--------------------------------------------------------------
 *  TIMER_WriteValue()
 *--------------------------------------------------------------
 * Parameters : none
 * Return     :	none
 * Functionality :  Write timer value
 *--------------------------------------------------------------*/

void TIMER_WriteValue (SYS_UWORD16 value)
{
  * (SYS_UWORD16 *) TIMER_LOAD_REG = value;	/*load the value */
}

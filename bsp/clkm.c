/******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
   Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                  
                                                           
   Filename       	: clkm.c

   Description    	: Set of functions useful to test the Saturn
			  CLKM peripheral 

   Project        	: drivers

   Author         	: pmonteil@tif.ti.com  Patrice Monteil.

   Version number	: 1.11

   Date and time	: 10/23/01 14:43:31

   Previous delta 	: 10/23/01 14:43:31

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/mod/emu_p/EMU_P_FRED_CLOCK/drivers1/common/SCCS/s.clkm.c

   Sccs Id  (SID)       : '@(#) clkm.c 1.11 10/23/01 14:43:31 '


 
*****************************************************************************/

           //############################################################
           //############################################################
           //### Be careful: this file must be placed in Flash Memory ###
           //###     and compiled in 16 bits length intructions       ###
           //###        (CF. the function wait_ARM_cycles()           ###
           //############################################################
           //############################################################
           
#include "../include/config.h"
#include "../include/sys_types.h"

#include "mem.h"
#include "clkm.h"

static SYS_UWORD32 ratio_wait_loop = 0;

#if (CHIPSET == 12)
  const double dsp_div_value[CLKM_NB_DSP_DIV_VALUE] = {1, 1.5, 2, 3};
#endif

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  /*---------------------------------------------------------------/
  /*  CLKM_InitARMClock()                                         */
  /*--------------------------------------------------------------*/
  /* Parameters :  clk_src : 0x00 means DPLL selected             */
  /*                         0x01 means VTCX0 selected            */
  /*                         0x03 means CLKIN selected            */
  /*               clk_xp5 : Enable 1.5 or 2.5 division factor    */
  /*                         (0 or 1)                             */
  /*               clk_div : Division factor applied to clock     */
  /*                         source                               */
  /*             WARNING : reverse order in comparison to ULYSSE  */
  /*                                                              */
  /* Return     : none                                            */
  /* Functionality :Initialize the ARM Clock frequency            */
  /*--------------------------------------------------------------*/

  void CLKM_InitARMClock(SYS_UWORD16 clk_src, SYS_UWORD16 clk_div, SYS_UWORD16 clk_xp5)
  {
    SYS_UWORD16 cntl = * (volatile SYS_UWORD16 *) CLKM_ARM_CLK;
        
    cntl &= ~(CLKM_CLKIN0 | CLKM_CLKIN_SEL | CLKM_ARM_MCLK_XP5 | CLKM_MCLK_DIV);
    
    cntl |= ((clk_src << 1) | (clk_xp5 << 3) | (clk_div << 4));
    
    * (volatile SYS_UWORD16 *) CLKM_ARM_CLK = cntl;
  }
#else
  /*---------------------------------------------------------------/
  /*  CLKM_InitARMClock()						*/
  /*--------------------------------------------------------------*/
  /* Parameters :  clk_src : 0x00 means CLKIN selected		*/
  /*	  	  	   0x01 means 32 K selected		*/
  /*	 	  	   0x02 means External clock selected	*/
  /*								*/
  /* Return     :	none						*/
  /* Functionality :Initialize the ARM Clock frequency 		*/
  /*--------------------------------------------------------------*/

  void CLKM_InitARMClock(SYS_UWORD16 clk_src, SYS_UWORD16 clk_div)
  {
    SYS_UWORD16 cntl = * (volatile SYS_UWORD16 *) CLKM_ARM_CLK;

    cntl &= ~(CLKM_LOW_FRQ | CLKM_CLKIN_SEL | CLKM_MCLK_DIV);

    cntl |= ((clk_src << 1) | (clk_div << 4));

    * (volatile SYS_UWORD16 *) CLKM_ARM_CLK = cntl;
  }

#endif


/*-------------------------------------------------------*/ 
/* convert_nanosec_to_cycles()                           */
/*-------------------------------------------------------*/
/* parameter: time in 10E-9 seconds                      */
/* return: Number of cycles for the wait_ARM_cycles()    */
/*         function                                      */
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* convert x nanoseconds in y cycles used by the ASM loop*/
/* function . Before calling this function, call the     */ 
/* initialize_wait_loop() function                       */
/* Called when the HardWare needs time to wait           */
/*-------------------------------------------------------*/ 
SYS_UWORD32 convert_nanosec_to_cycles(SYS_UWORD32 time)
{ 
  return( time / ratio_wait_loop);  
}


/*-------------------------------------------------------*/
/* initialize_wait_loop()                                */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* Init the ratio used to convert time->Cycles according */
/* to hardware parameters                                */
/* measurement time for this function (ARM 39Mhz, 3 waits*/
/* states) = 75 micoseconds                              */
/*-------------------------------------------------------*/

void initialize_wait_loop(void)
{
  #define NBR_CYCLES_IN_LOOP   5   // this value is got from an oscilloscope measurement
  
  double src_ratio;
  double final_ratio;

  SYS_UWORD16 flash_access_size;
  SYS_UWORD16 flash_wait_state;
  SYS_UWORD32 nbr;
  SYS_UWORD32 arm_clock;

  //////////////////////////////////
  //  compute the ARM clock used  //
  //////////////////////////////////
  {
    SYS_UWORD16 arm_mclk_xp5;
    SYS_UWORD16 arm_ratio;
    SYS_UWORD16 clk_src;
    SYS_UWORD16 clkm_cntl_arm_clk_reg = * (volatile SYS_UWORD16 *) CLKM_CNTL_ARM_CLK;

    #if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
      clk_src  = (clkm_cntl_arm_clk_reg & MASK_CLKIN) >> 1;
      switch (clk_src)
      {
        case 0x00: //DPLL selected 
          // select the DPLL factor
          if (((* (volatile SYS_UWORD16 *) MEM_DPLL_ADDR) & DPLL_LOCK) != 0)
          {
             SYS_UWORD16 dpll_div;
             SYS_UWORD16 dpll_mul;

             dpll_div=DPLL_READ_DPLL_DIV;
             dpll_mul=DPLL_READ_DPLL_MUL;
             src_ratio = (double)(dpll_mul)/(double)(dpll_div+1);              
          }
          else // DPLL in bypass mode
          {
             SYS_UWORD16 dpll_div = DPLL_BYPASS_DIV;
             src_ratio= (double)(1)/(double)(dpll_div+1);
          }
          break;
        case 0x01: //VTCX0 selected 
          src_ratio = 1;
          break;
        case 0x03: //CLKIN selected   (external clock)
          src_ratio = 1;
          break;
      }
      // define the division factor applied to clock source (CLKIN or VTCXO or DPLL)
      arm_ratio = (clkm_cntl_arm_clk_reg & CLKM_MCLK_DIV) >> 4;

      // check if the 1.5 or 2.5 division factor is enabled
      arm_mclk_xp5  = clkm_cntl_arm_clk_reg & CLKM_ARM_MCLK_XP5;

      if (arm_mclk_xp5 == 0) // division factor enable for ARM clock ?
      {
        if (arm_ratio == 0) 
          arm_ratio =1; 
      }
      else
        arm_ratio = ((arm_ratio>>1) & 0x0001) == 0 ? 1.5 : 2.5;


    #else // CHIPSET
      src_ratio = 1;

      // define the division factor applied to clock source (CLKIN or VTCXO or DPLL)
      arm_ratio = (clkm_cntl_arm_clk_reg & CLKM_MCLK_DIV) >> 4;

      // check if the 1.5 or 2.5 division factor is enabled
      arm_mclk_xp5  = clkm_cntl_arm_clk_reg & MASK_ARM_MCLK_1P5;

      if (arm_mclk_xp5 == 1) // division factor enable for ARM clock ?
        arm_ratio = 1.5;  
      else
      {
        if (arm_ratio == 0)
          arm_ratio = 4;
        else 
          if (arm_ratio == 1 )
            arm_ratio = 2;
          else 
            arm_ratio = 1;
      }

    #endif // CHIPSET

   final_ratio = (src_ratio / (double) arm_ratio);

  }
  //////////////////////////////////////////
  //  compute the Flash wait states used  //
  //////////////////////////////////////////

  #if (CHIPSET == 12)
    flash_access_size  =  *((volatile SYS_UWORD16 *) MEM_REG_nCS5);
  #else
    flash_access_size  =  *((volatile SYS_UWORD16 *) MEM_REG_nCS0);
  #endif
  flash_access_size  = (flash_access_size >> 5) & 0x0003; // 0=>8bits, 1=>16 bits, 2 =>32 bits

  // the loop file is compiled in 16 bits it means
  //    flash 8  bits => 2 loads for 1 16 bits assembler instruction
  //    flash 16 bits => 1 loads for 1 16 bits assembler instruction
  //    flash 32 bits => 1 loads for 1 16 bits assembler instruction (ARM bus 16 bits !!)
  
  // !!!!!!!!! be careful: if this file is compile in 32 bits, change these 2 lines here after !!!
  if (flash_access_size == 0) flash_access_size = 2;
  else                        flash_access_size = 1;

  #if (CHIPSET == 12)
    /*
     *  WARNING - New ARM Memory Interface features are not supported here below (Page Mode, extended WS, Dummy Cycle,...).
     */
    flash_wait_state  =  *((volatile SYS_UWORD16 *) MEM_REG_nCS5);
  #else
    flash_wait_state  =  *((volatile SYS_UWORD16 *) MEM_REG_nCS0);
  #endif
  flash_wait_state &=  0x001F;

  //////////////////////////////////////
  //  compute the length of the loop  //
  //////////////////////////////////////

  // Number of flash cycles for the assembler loop
  nbr = NBR_CYCLES_IN_LOOP;

  // Number of ARM cycles for the assembler loop
  nbr = nbr * (flash_wait_state + 1) * (flash_access_size);

  // time for the assembler loop (unit nanoseconds: 10E-9)
  arm_clock = final_ratio * 13; // ARM clock in Mhz 
  ratio_wait_loop = (SYS_UWORD32)((nbr*1000) / arm_clock);
}


/*-------------------------------------------------------*/ 
/* wait_ARM_cycles()                                     */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* Called when the HardWare needs time to wait.          */
/* this function wait x cycles and is used with the      */
/* convert_nanosec_to_cycles() & initialize_wait_loop()  */
/*                                                       */
/*  Exemple:  wait 10 micro seconds:                     */
/*  initialize_wait_loop();                              */
/*  wait_ARM_cycles(convert_nanosec_to_cycles(10000))    */
/*                                                       */
/*  minimum time value with cpt_loop = 0  (estimated)    */
/*  and C-SAMPLE / flash 6,5Mhz  ~  1,5 micro seconds    */
/*                                                       */
/*                                                       */
/* Be careful : in order to respect the rule about the   */
/* conversion "time => number of cylcles in this loop"   */
/* (Cf the functions: convert_nanosec_to_cycles() and    */
/* initialize_wait_loop() ) respect the following rules: */
/* This function must be placed in Flash Memory and      */
/* compiled in 16 bits instructions length               */
/*-------------------------------------------------------*/
void wait_ARM_cycles(SYS_UWORD32 cpt_loop) 
{
  // C code:
  // while (cpt_loop -- != 0);

  asm(" CMP       A1, #0");                 
  asm(" BEQ       END_FUNCTION");           

  asm("LOOP_LINE:        ");                
  asm(" SUB       A1, A1, #1");
  asm(" CMP       A1, #0");
  asm(" BNE       LOOP_LINE");

  asm("END_FUNCTION:        ");  
} 

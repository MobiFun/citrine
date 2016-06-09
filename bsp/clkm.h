/******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
   Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                  
                                                           
   Filename       	: clkm.h

   Description    	: Header file for the CLKM module

   Project        	: drivers

   Author         	: pmonteil@tif.ti.com  Patrice Monteil.

   Version number	: 1.10

   Date and time	: 10/23/01 14:34:54

   Previous delta 	: 10/19/01 15:25:25

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/mod/emu_p/EMU_P_FRED_CLOCK/drivers1/common/SCCS/s.clkm.h

   Sccs Id  (SID)       : '@(#) clkm.h 1.10 10/23/01 14:34:54 '

 * FreeCalypso note: this version of clkm.h originates
 * from the MV100-0.1.rar find.
 
*****************************************************************************/

#include "../include/config.h"
#include "../include/sys_types.h"

#define CLKM_ARM_CLK 	MEM_CLKM_ADDR 		/* CLKM ARM CLock Control reg.*/
#define CLKM_MCLK_EN	0x0001 


#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
   #define MASK_CLKIN       0x0006
#endif

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define CLKM_CLKIN0       0x0002            // Mask to select between DPLL and VTCXO or CLKIN
#else
  #define CLKM_LOW_FRQ	    0x0002            // Mask to select low frequency input CLK_32K 
#endif
#define CLKM_CLKIN_SEL	    0x0004            // Mask to select between VTCXO and CLKIN 

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define CLKM_ARM_MCLK_XP5 0x0008            // Mask to enable the 1.5 or 2.5 division factor
  #define CLKM_MCLK_DIV     0x0070            // Mask to configure the division factor
#else
  #define MASK_ARM_MCLK_1P5 0x0008            // Mask to enable the 1.5 division factor
  #define CLKM_MCLK_DIV	    0x0030            // Mask to configure the division factor
#endif

#define CLKM_DEEP_PWR	    0x0f00            // Mask to configure deep power
#define CLKM_DEEP_SLEEP	    0x1000            // Mask to configure deep sleep

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define CLKM_SEL_DPLL     0x0000            // Selection of DPLL for ARM clock generation
  #define CLKM_SEL_VTCXO    0x0001            // Selection of VTCXO for ARM clock generation
  #define CLKM_SEL_CLKIN    0x0003            // Selection of CLKIN for ARM clock generation
  
  #define CLKM_ENABLE_XP5   0x0001            // Enable 1.5 or 2.5 division factor
  #define CLKM_DISABLE_XP5  0x0000            // Disable 1.5 or 2.5 division factor
  
  #define CLKM_ARM_MCLK_DIV_OFFSET 4          // Offset of ARM_MCLK_DIV bits in CNTL_ARM_CLK register
  
  #define CLKM_ARM_CLK_RESET 0x1081           // Reset value of CNTL_ARM_CLK register
#endif

#define CLKM_CNTL_ARM_CLK  (MEM_CLKM_ADDR + 0x00)
#define CLKM_CNTL_CLK	   (MEM_CLKM_ADDR + 2) 	/* CLKM Clock Control reg. */

#define CLKM_IRQ_DIS        0x0001     // IRQ clock is disabled and enabled according to the sleep command
#define CLKM_BRIDGE_DIS	    0x0002     // BRIDGE clock is disabled and enabled according to the sleep command
#define CLKM_TIMER_DIS	    0x0004     // TIMER clock is disabled and enabled according to the sleep command
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define CLKM_DPLL_DIS     0x0008     // DPLL is set in IDLE when both DSP and ARM are respectively in IDLE3 and sleep mode
#else
  #define CLKM_PLL_SEL	    0x0008     // CLKIN input is connected to the PLL
#endif
#define CLKM_CLKOUT_EN	    0x0010     // Enable CLKOUT(2:0) output clocks
#if (CHIPSET == 4)
  #define CLKM_EN_IDLE3_FLG 0x0020     // DSP idle flag control the API wait state
  #define CLKM_VTCXO_26     0x0040     // VTCXO is divided by 2
#elif (CHIPSET == 6)
  #define CLKM_VTCXO_26     0x0040     // VTCXO is divided by 2
#elif (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12)
  #define CLKM_EN_IDLE3_FLG 0x0020     // DSP idle flag control the API wait state
  #define CLKM_VCLKOUT_2    0x0040     // VTCXO is divided by 2
  #define CLKM_VTCXO_2      0x0080     // Input clock to DPLL is divided by 2
#endif

#define CLKM_CNTL_RST 	(MEM_CLKM_ADDR + 4) 	/* CLKM Reset Control reg. */
#define CLKM_LEAD_RST 	0x0002
#define CLKM_EXT_RST 	0x0004

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define DPLL_LOCK         0x0001     // Mask of DPLL lock status
  #define DPLL_BYPASS_DIV   0x000C     // Mask of bypass mode configuration
  #define DPLL_PLL_ENABLE   0x0010     // Enable DPLL
  #define DPLL_PLL_DIV      0x0060     // Mask of division factor configuration
  #define DPLL_PLL_MULT     0x0F80     // Mask of multiply factor configuration

  #define DPLL_BYPASS_DIV_1 0x0        // Configuration of bypass mode divided by 1
  #define DPLL_BYPASS_DIV_2 0x1        // Configuration of bypass mode divided by 2
  #define DPLL_BYPASS_DIV_4 0x2        // Configuration of bypass mode divided by 4

  #define DPLL_BYPASS_DIV_OFFSET 2     // Offset of bypass bits configuration
  #define DPLL_PLL_DIV_OFFSET    5     // Offset of division bits configuration
  #define DPLL_PLL_MULT_OFFSET   7     // Offset of multiply bits configuration

  #define DPLL_LOCK_DIV_1   0x0000     // Divide by 1 when DPLL is locked
  #define DPLL_LOCK_DIV_2   0x0001     // Divide by 2 when DPLL is locked
  #define DPLL_LOCK_DIV_3   0x0002     // Divide by 3 when DPLL is locked
  #define DPLL_LOCK_DIV_4   0x0003     // Divide by 4 when DPLL is locked

#else
  #define CLKM_LEAD_PLL_CNTL (MEM_CLKM_ADDR + 6)     /* Lead PLL */
  #define CLKM_PLONOFF	0x0001         // PLL enable signal
  #define CLKM_PLMUL	0x001e         // Mask of multiply factor configuration
  #define CLKM_PLLNDIV	0x0020         // PLL or divide mode selection
  #define CLKM_PLDIV	0x0040         // Mask of multiply factor configuration
  #define CLKM_LEAD_PLL_CNTL_MSK 0x00ef // Mask of PLL control register
#endif

#if (CHIPSET == 12)
  #define CLKM_CNTL_CLK_DSP (MEM_CLKM_ADDR + 0x8A) 	/* CLKM CNTL_CLK_REG register */
  
  #define CLKM_NB_DSP_DIV_VALUE   4
  
  #define CLKM_DSP_DIV_1    0x00
  #define CLKM_DSP_DIV_1_5  0x01
  #define CLKM_DSP_DIV_2    0x02
  #define CLKM_DSP_DIV_3    0x03
  
  #define CLKM_DSP_DIV_MASK 0x0003
  
  extern const double dsp_div_value[CLKM_NB_DSP_DIV_VALUE];
  
  /*---------------------------------------------------------------/
  /*  CLKM_DSP_DIV_FACTOR()						*/
  /*--------------------------------------------------------------*/
  /* Parameters : none						*/
  /* Return     :	none						*/
  /* Functionality : Set the DSP division factor			*/
  /*--------------------------------------------------------------*/

  #define CLKM_DSP_DIV_FACTOR(d_dsp_div) (* (volatile SYS_UWORD16 *) CLKM_CNTL_CLK_DSP = d_dsp_div)


  /*---------------------------------------------------------------/
  /*  CLKM_READ_DSP_DIV()						*/
  /*--------------------------------------------------------------*/
  /* Parameters : none						*/
  /* Return     :	none						*/
  /* Functionality : Read DSP division factor			*/
  /*--------------------------------------------------------------*/

  #define CLKM_READ_DSP_DIV               ((* (volatile SYS_UWORD16 *) CLKM_CNTL_CLK_DSP) & CLKM_DSP_DIV_MASK)
  
  #define CLKM_GET_DSP_DIV_VALUE          dsp_div_value[CLKM_READ_DSP_DIV]

#endif


/*---------------------------------------------------------------/
/*  CLKM_SETLEADRESET()						*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : Set the LEAD reset signal			*/
/*--------------------------------------------------------------*/

#define CLKM_SETLEADRESET (* (volatile SYS_UWORD16 *) CLKM_CNTL_RST |= CLKM_LEAD_RST)

/*---------------------------------------------------------------/
/*  CLKM_RELEASELEADRESET()					*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : Release the LEAD reset signal		*/
/*--------------------------------------------------------------*/

#define CLKM_RELEASELEADRESET (* (volatile SYS_UWORD16 *) CLKM_CNTL_RST &= ~CLKM_LEAD_RST)

/*---------------------------------------------------------------/
/*  CLKM_SETEXTRESET()						*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : Set the external reset signal		*/
/*--------------------------------------------------------------*/

#define CLKM_SETEXTRESET ( * (volatile SYS_UWORD16 *) CLKM_CNTL_RST |= CLKM_EXT_RST)

/*---------------------------------------------------------------/
/*  CLKM_CLEAREXTRESET()					*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : Clear the external reset signal		*/
/*--------------------------------------------------------------*/

#define CLKM_CLEAREXTRESET (* (volatile SYS_UWORD16 *) CLKM_CNTL_RST &= ~CLKM_EXT_RST)


/*---------------------------------------------------------------/
/*  CLKM_POWERDOWNARM()						*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : Power down the ARM mcu			*/
/*--------------------------------------------------------------*/
#define  CLKM_POWERDOWNARM (* (volatile SYS_UWORD16 *) CLKM_ARM_CLK &= ~CLKM_MCLK_EN)

/*---------------------------------------------------------------/
/*  CLKM_SET1P5()						*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : Set ARM_MCLK_1P5 bit				*/
/*--------------------------------------------------------------*/

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define CLKM_SETXP5 ( * (volatile SYS_UWORD16 *) CLKM_ARM_CLK |= CLKM_ARM_MCLK_XP5)
#else
  #define CLKM_SET1P5 ( * (volatile SYS_UWORD16 *) CLKM_ARM_CLK |= 0x0008)
#endif

/*---------------------------------------------------------------/
/*  CLKM_RESET1P5()						*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : Reset ARM_MCLK_1P5 bit			*/
/*--------------------------------------------------------------*/

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define CLKM_RESETXP5 ( * (volatile SYS_UWORD16 *) CLKM_ARM_CLK &= ~CLKM_ARM_MCLK_XP5)
#else
  #define CLKM_RESET1P5 ( * (volatile SYS_UWORD16 *) CLKM_ARM_CLK &= 0xfff7)
#endif

/*---------------------------------------------------------------/
/*  CLKM_INITCNTL()						*/
/*--------------------------------------------------------------*/
/* Parameters : value to write in the CNTL register		*/
/* Return     :	none						*/
/* Functionality :Initialize the CLKM Control Clock register	*/
/*--------------------------------------------------------------*/

#define CLKM_INITCNTL(value) (* (volatile SYS_UWORD16 *) CLKM_CNTL_CLK = value)

/*
 * NOTE: the version of the CLKM_INITCNTL() macro in the Sotomodem source
 * does |= instead of =.  It remains to be investigated which is more correct.
 *
 * For now I'll define the ORing version under a different (and more
 * descriptive) name: CLKM_CNTL_OR.
 */

#define CLKM_CNTL_OR(value) (* (volatile SYS_UWORD16 *) CLKM_CNTL_CLK |= value)

#if ((CHIPSET != 4) && (CHIPSET != 7) && (CHIPSET != 8) && (CHIPSET != 10) && (CHIPSET != 11) && (CHIPSET != 12))
  /*---------------------------------------------------------------/
  /*  CLKM_INITLEADPLL()						*/
  /*--------------------------------------------------------------*/
  /* Parameters : value to write in the CNTL_PLL LEAD register	*/
  /* Return     :	none						*/
  /* Functionality :Initialize LEAD PLL control register		*/
  /*--------------------------------------------------------------*/

  #define CLKM_INITLEADPLL(value) (* (volatile SYS_UWORD16 *) CLKM_LEAD_PLL_CNTL = value)
#endif

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  /*---------------------------------------------------------------/
  /*  CLKM_DPLL_SWITH_OFF_MODE_CONFIG()                           */
  /*--------------------------------------------------------------*/
  /* Parameters : None                                            */
  /* Return     : none                                            */
  /* Functionality : Configure DPLL switch off mode               */
  /*--------------------------------------------------------------*/

  #define CLKM_DPLL_SWITH_OFF_MODE_CONFIG (* (volatile SYS_UWORD16 *) CLKM_CNTL_CLK |= \
             (CLKM_DPLL_DIS | CLKM_IRQ_DIS | CLKM_BRIDGE_DIS | CLKM_TIMER_DIS))
             
  /*---------------------------------------------------------------/
  /*  CLKM_RESET_DPLL_SWITH_OFF_MODE_CONFIG()                     */
  /*--------------------------------------------------------------*/
  /* Parameters : None                                            */
  /* Return     : none                                            */
  /* Functionality : Reset configuration of DPLL switch off mode  */
  /*--------------------------------------------------------------*/
             
  #define CLKM_RESET_DPLL_SWITH_OFF_MODE_CONFIG (* (volatile SYS_UWORD16 *) CLKM_CNTL_CLK &=\
            ~(CLKM_DPLL_DIS | CLKM_IRQ_DIS | CLKM_BRIDGE_DIS | CLKM_TIMER_DIS))

  /*---------------------------------------------------------------/
  /*  CLKM_FORCE_API_HOM_IN_IDLE3()                               */
  /*--------------------------------------------------------------*/
  /* Parameters : None                                            */
  /* Return     : none                                            */
  /* Functionality : SAM/HOM wait-state register force to HOM when*/
  /*                 DSP is in IDLE3 mode                         */
  /*--------------------------------------------------------------*/

  #define CLKM_FORCE_API_HOM_IN_IDLE3 (* (volatile SYS_UWORD16 *) CLKM_CNTL_CLK |= (CLKM_EN_IDLE3_FLG))

  #if (CHIPSET == 4)
    /*---------------------------------------------------------------/
    /*  CLKM_USE_VTCXO_26MHZ()                                      */
    /*--------------------------------------------------------------*/
    /* Parameters : None                                            */
    /* Return     : none                                            */
    /* Functionality : Divide by 2 the clock used by the peripheral */
    /*                 when using external VTCXO at 26 MHz instead  */
    /*                 of 13MHz                                     */
    /*--------------------------------------------------------------*/

    #define CLKM_USE_VTCXO_26MHZ (* (volatile SYS_UWORD16 *) CLKM_CNTL_CLK |= (CLKM_VTCXO_26))

    /*---------------------------------------------------------------/
    /*  CLKM_UNUSED_VTCXO_26MHZ()                                   */
    /*--------------------------------------------------------------*/
    /* Parameters : None                                            */
    /* Return     : none                                            */
    /* Functionality : Use VTCXO=13MHz                              */
    /*--------------------------------------------------------------*/

    #define CLKM_UNUSED_VTCXO_26MHZ (* (volatile SYS_UWORD16 *) CLKM_CNTL_CLK &= ~(CLKM_VTCXO_26))
  #elif (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12)
    /*---------------------------------------------------------------/
    /*  CLKM_UNUSED_VTCXO_26MHZ()                                   */
    /*--------------------------------------------------------------*/
    /* Parameters : None                                            */
    /* Return     : none                                            */
    /* Functionality : Use VTCXO=13MHz                              */
    /*--------------------------------------------------------------*/

    #define CLKM_USE_VTCXO_26MHZ (* (volatile SYS_UWORD16 *) CLKM_CNTL_CLK |= (CLKM_VTCXO_2))

    #define CLKM_UNUSED_VTCXO_26MHZ (* (volatile SYS_UWORD16 *) CLKM_CNTL_CLK &= ~(CLKM_VCLKOUT_2 | CLKM_VTCXO_2))
  #endif


  #define DPLL_SET_PLL_ENABLE (* (volatile SYS_UWORD16 *) MEM_DPLL_ADDR |= DPLL_PLL_ENABLE)
  #define DPLL_RESET_PLL_ENABLE (* (volatile SYS_UWORD16 *) MEM_DPLL_ADDR &= ~DPLL_PLL_ENABLE)
  
  #define DPLL_INIT_BYPASS_MODE(d_bypass_mode) { \
        *((volatile SYS_UWORD16 *) MEM_DPLL_ADDR) &= ~DPLL_BYPASS_DIV; \
        *((volatile SYS_UWORD16 *) MEM_DPLL_ADDR) |= (d_bypass_mode << DPLL_BYPASS_DIV_OFFSET); \
       }
    
  #define DPLL_INIT_DPLL_CLOCK(d_pll_div, d_pll_mult) { \
        *((volatile SYS_UWORD16 *) MEM_DPLL_ADDR) &= ~(DPLL_PLL_DIV | DPLL_PLL_MULT); \
        *((volatile SYS_UWORD16 *) MEM_DPLL_ADDR) |= (d_pll_div << DPLL_PLL_DIV_OFFSET) |\
                                                        (d_pll_mult << DPLL_PLL_MULT_OFFSET); \
       }

  #define DPLL_READ_DPLL_DIV ( ((* (volatile SYS_UWORD16 *) MEM_DPLL_ADDR) & DPLL_PLL_DIV) >> DPLL_PLL_DIV_OFFSET)
  #define DPLL_READ_DPLL_MUL ( ((* (volatile SYS_UWORD16 *) MEM_DPLL_ADDR) & DPLL_PLL_MULT)>> DPLL_PLL_MULT_OFFSET)
  #define DPLL_READ_DPLL_LOCK ( (* (volatile SYS_UWORD16 *) MEM_DPLL_ADDR) & DPLL_LOCK)


#endif

/* ----- Prototypes ----- */

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  void CLKM_InitARMClock(SYS_UWORD16 clk_src, SYS_UWORD16 clk_div, SYS_UWORD16 clk_xp5);
#else
  void CLKM_InitARMClock(SYS_UWORD16 clk_src, SYS_UWORD16 clk_div);
#endif

void wait_ARM_cycles(SYS_UWORD32 cpt_loop);
void initialize_wait_loop(void);
SYS_UWORD32 convert_nanosec_to_cycles(SYS_UWORD32 time);

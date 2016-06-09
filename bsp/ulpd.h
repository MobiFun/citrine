/*******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
  Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                 
                                                           
   Filename       	: ulpd.h

   Description    	: Header for HYPERION/ULPD module tests
			  Target : Arm

   Project        	: Hyperion

   Author         	: smunsch@tif.ti.com  Sylvain Munsch.

   Version number	: 1.11

   Date and time	: 12/20/00 10:17:22

   Previous delta 	: 12/06/00 17:31:50

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/mod/emu_p/EMU_P/drivers1/common/SCCS/s.ulpd.h

   Sccs Id  (SID)       : '@(#) ulpd.h 1.11 12/20/00 10:17:22 '

 
*****************************************************************************/

#include "../include/config.h"

#include <limits.h>
#include <float.h>

// SLEEP MODES
//=======================
#define DO_NOT_SLEEP          00
#define FRAME_STOP            01 // little BIG SLEEP (CUST5...)
#define CLOCK_STOP            02 // Deep sleep


// ULPD registers address 
//=======================

#define ULPD_XIO_START               0xfffe2000

#define ULPD_INC_FRAC_REG             (SYS_UWORD16 *)(ULPD_XIO_START)	
#define ULPD_INC_SIXTEENTH_REG       ((SYS_UWORD16 *)(ULPD_XIO_START) + 1)
#define ULPD_SIXTEENTH_START_REG     ((SYS_UWORD16 *)(ULPD_XIO_START) + 2)
#define ULPD_SIXTEENTH_STOP_REG	     ((SYS_UWORD16 *)(ULPD_XIO_START) + 3)
#define ULPD_COUNTER_32_LSB_REG	     ((SYS_UWORD16 *)(ULPD_XIO_START) + 4)
#define ULPD_COUNTER_32_MSB_REG	     ((SYS_UWORD16 *)(ULPD_XIO_START) + 5)
#define ULPD_COUNTER_HI_FREQ_LSB_REG ((SYS_UWORD16 *)(ULPD_XIO_START) + 6)
#define ULPD_COUNTER_HI_FREQ_MSB_REG ((SYS_UWORD16 *)(ULPD_XIO_START) + 7)
#define ULPD_GAUGING_CTRL_REG        ((SYS_UWORD16 *)(ULPD_XIO_START) + 8)
#define ULPD_GAUGING_STATUS_REG      ((SYS_UWORD16 *)(ULPD_XIO_START) + 9)
#define ULPD_GSM_TIMER_CTRL_REG      ((SYS_UWORD16 *)(ULPD_XIO_START) + 10)  
#define ULPD_GSM_TIMER_INIT_REG      ((SYS_UWORD16 *)(ULPD_XIO_START) + 11)
#define ULPD_GSM_TIMER_VALUE_REG     ((SYS_UWORD16 *)(ULPD_XIO_START) + 12)
#define ULPD_GSM_TIMER_IT_REG        ((SYS_UWORD16 *)(ULPD_XIO_START) + 13) 
#define ULPD_SETUP_CLK13_REG   	     ((SYS_UWORD16 *)(ULPD_XIO_START) + 14)
#define ULPD_SETUP_SLICER_REG	     ((SYS_UWORD16 *)(ULPD_XIO_START) + 15)
#define ULPD_SETUP_VTCXO_REG         ((SYS_UWORD16 *)(ULPD_XIO_START) + 16) 
#define ULPD_SETUP_FRAME_REG         ((SYS_UWORD16 *)(ULPD_XIO_START) + 17) 
#define ULPD_SETUP_RF_REG            ((SYS_UWORD16 *)(ULPD_XIO_START) + 18)

/* TI's dyslexia */
#define ULDP_INC_SIXTEENTH_REG       ULPD_INC_SIXTEENTH_REG
#define ULDP_SIXTEENTH_START_REG     ULPD_SIXTEENTH_START_REG
#define ULDP_SIXTEENTH_STOP_REG	     ULPD_SIXTEENTH_STOP_REG
#define ULDP_COUNTER_32_LSB_REG	     ULPD_COUNTER_32_LSB_REG
#define ULDP_COUNTER_32_MSB_REG	     ULPD_COUNTER_32_MSB_REG
#define ULDP_COUNTER_HI_FREQ_LSB_REG ULPD_COUNTER_HI_FREQ_LSB_REG
#define ULDP_COUNTER_HI_FREQ_MSB_REG ULPD_COUNTER_HI_FREQ_MSB_REG
#define ULDP_GAUGING_CTRL_REG        ULPD_GAUGING_CTRL_REG
#define ULDP_GAUGING_STATUS_REG      ULPD_GAUGING_STATUS_REG
#define ULDP_GSM_TIMER_CTRL_REG      ULPD_GSM_TIMER_CTRL_REG
#define ULDP_GSM_TIMER_INIT_REG      ULPD_GSM_TIMER_INIT_REG
#define ULDP_GSM_TIMER_VALUE_REG     ULPD_GSM_TIMER_VALUE_REG
#define ULDP_GSM_TIMER_IT_REG        ULPD_GSM_TIMER_IT_REG
#define ULDP_SETUP_CLK13_REG   	     ULPD_SETUP_CLK13_REG
#define ULDP_SETUP_SLICER_REG	     ULPD_SETUP_SLICER_REG
#define ULDP_SETUP_VTCXO_REG         ULPD_SETUP_VTCXO_REG
#define ULDP_SETUP_FRAME_REG         ULPD_SETUP_FRAME_REG

// ULPD gauging control register description 
//==========================================

#define ULPD_GAUGING_EN	              0x0001   // Gauging is running 
#define ULPD_GAUGING_TYPE_HF          0x0002   // Gauging versus HFclock
#define ULPD_SEL_HF_PLL               0x0004   // High freq clock = PLL DSP 

/* more dyslexia */
#define ULDP_GAUGING_EN	              ULPD_GAUGING_EN
#define ULDP_GAUGING_TYPE_HF          ULPD_GAUGING_TYPE_HF
#define ULDP_SEL_HF_PLL               ULPD_SEL_HF_PLL

// ULPD gauging status register description 
//==========================================

#define ULPD_IT_GAUGING               0x0001  // Interrupt it_gauging occurence 
#define ULPD_OVF_HF                   0x0002  // Overflow on the HF counter 
#define ULPD_OVF_32                   0x0004  // Overflow on the 32 Khz counter

#define ULDP_IT_GAUGING               ULPD_IT_GAUGING
#define ULDP_OVF_HF                   ULPD_OVF_HF
#define ULDP_OVF_32                   ULPD_OVF_32

// WAKEup time
//==========================================
//  the setup time unit is the number of 32 Khz clock periods

#if (BOARD == 34)

#define SETUP_RF                      75      // adujstement time to minimize big_sleep duration
                                              // The SETUP_RF value must be used to delay as much as possible the true  
                                              // start time of the deep_sleep wake-up sequence for power consumption saving. 
                                              // This is required because the unit of the SETUP_FRAME counter is the  
                                              // GSM TDMA frame and not a T32K time period.

#define SETUP_VTCXO                   320     // The setup_vtcxo is the time the external RF device takes to deliver
                                              // stable signals to the VTCXO


#define SETUP_SLICER                  180     // The setup_slicer is the time that the vtcxo takes to deliver 
                                              // a stable output when vtcxo is enabled : usually 2 to 5ms
                                              // The SETUP_SLICER value should be smaller than 160(=4,8ms) but this 
                                              // parameter is directly related to the VTCXO device used in the phone
                                              // and consequently must be retrieved from the VTCXO data-sheet.

#define SETUP_CLK13                   31      // The setup_clk13 is time that the slicer takes to deliver
                                              // a stable output when slicer is enabled : max conservative value 1ms

#else

#define SETUP_RF                      0       // adujstement time to minimize big_sleep duration
                                              // The SETUP_RF value must be used to delay as much as possible the true  
                                              // start time of the deep_sleep wake-up sequence for power consumption saving. 
                                              // This is required because the unit of the SETUP_FRAME counter is the  
                                              // GSM TDMA frame and not a T32K time period.
#if (CHIPSET == 2)
  #define SETUP_VTCXO                 31      // The setup_vtcxo is the time the external RF device takes to deliver
#else                                         // stable signals to the VTCXO
  #define SETUP_VTCXO                 1114    // 34 ms for ABB LDO stabilization before 13MHz switch ON
                                              // Minimum value to be sure that ABB is awake while the DBB start running for 
                                              // SETUP_VTCXO  =  ((SLPDLY*16)+4+145)*T32KHz       
#endif  

#if (BOARD == 40) || (BOARD == 41) || (BOARD == 42) || (BOARD == 43) || (BOARD == 45)
  #if (RF_FAM==12)
    #define SETUP_SLICER                660
  #else 
    #define SETUP_SLICER                600     // 600/32x10^3 = 18.75ms required for VCXO stabilization
  #endif
#else
  #define SETUP_SLICER                180     // The setup_slicer is the time that the vtcxo takes to deliver 
                                              // a stable output when vtcxo is enabled : usually 2 to 5ms
                                              // The SETUP_SLICER value should be smaller than 160(=4,8ms) but this 
                                              // parameter is directly related to the VTCXO device used in the phone
                                              // and consequently must be retrieved from the VTCXO data-sheet.
#endif

#define SETUP_CLK13                   31      // The setup_clk13 is time that the slicer takes to deliver
                                              // a stable output when slicer is enabled : max conservative value 1ms

#endif // BOARD == 34

// SETUP_FRAME:
//-------------
// CF. Reference document: ULYS015 v1.1 page 24
// 1) Nominal Frequency = 32.768 Khz => 0.03051757 ms 
//    (0.03051757 ms / 4.615 ms) =  0.006612692 Frames 
// 2) The use of the RFEN signal is optional. It is necessary if the VTCXO function 
//    is part of an RF IC which must be first powered before enabling the VTCXO. 
//    However it can be use for any other purpose.
// 3) The term (1-DBL_EPSILON) corresponds to the rounding up of SETUP_FRAME.
#ifndef DBL_EPSILON  //CQ16723: For non TI compiler, DBL_EPSILON can be undefined.
 #define DBL_EPSILON 0
#endif

#define SETUP_FRAME  ((( SETUP_RF+SETUP_VTCXO+SETUP_SLICER+SETUP_CLK13)*0.006612692)+(1-DBL_EPSILON))

#define MAX_GSM_TIMER                 65535   // max duration for the wake up timer


// Default values for Cell selection and CS_MODE0
//===============================================
#define DEFAULT_HFMHZ_VALUE (13000000*l1_config.dpll)
#define DEFAULT_32KHZ_VALUE (32768)   // real value 32768.29038 hz
//with l1ctl_pgm_clk32(DEFAULT_HFMHZ_VALUE,DEFAULT_32KHZ_VALUE) and dpll = 65Mhz
//          => DEFAULT_INCSIXTEEN           132
//          => DEFAULT_INCFRAC              15915 





// ULPD GSM timer control register description 
//============================================

#define ULPD_TM_LOAD                  0x0001 // load the timer with init value 
#define ULPD_TM_FREEZE                0x0002 // 1=> GSM timer is frozen 
#define ULPD_IT_TIMER_GSM             0x0001 // Interrupt timer occurrence 	

/* TI's dyslexia */
#define ULDP_TM_LOAD                  ULPD_TM_LOAD
#define ULDP_TM_FREEZE                ULPD_TM_FREEZE

/*
 * The following accessor macros all have dyslexic names, unfortunately.
 * Too much of a pita to rename them all, so I'm leaving them be for now.
 * -SF
 */

//  ULDP_INCFRAC_UPDATE : update INCFRAC (16 bits)
//================================================
#define ULDP_INCFRAC_UPDATE(frac)  (* (volatile SYS_UWORD16 *)ULPD_INC_FRAC_REG = frac)


//  ULDP_INCSIXTEEN_UPDATE : update INCSIXTEEN (12 bits)
//======================================================
#define ULDP_INCSIXTEEN_UPDATE(inc)  (* (volatile SYS_UWORD16 *)ULDP_INC_SIXTEENTH_REG = inc)


// ULDP_GAUGING_RUN : Start the gauging 
//=====================================
#define ULDP_GAUGING_RUN  (* (volatile SYS_UWORD16 *)ULDP_GAUGING_CTRL_REG |= ULDP_GAUGING_EN)


// ULDP_GAUGING_STATUS : Return if it gauging occurence
//======================================================
#define ULDP_GAUGING_STATUS ((* (volatile SYS_UWORD16 *) ULDP_GAUGING_STATUS_REG) & ULDP_GAUGING_EN )

// ULDP_GAUGING_STOP : Stop the gauging  
//=====================================
#define ULDP_GAUGING_STOP (* (volatile SYS_UWORD16 *) ULDP_GAUGING_CTRL_REG &= ~ULDP_GAUGING_EN) 

// ULDP_GAUGING_START : Stop the gauging  
//=====================================
#define ULDP_GAUGING_START (* (volatile SYS_UWORD16 *) ULDP_GAUGING_CTRL_REG |= ULDP_GAUGING_EN) 

// ULDP_GAUGING_SET_HF : Set the gauging versus HF clock
//======================================================
#define ULDP_GAUGING_SET_HF (* (volatile SYS_UWORD16 *) ULDP_GAUGING_CTRL_REG |= ULDP_GAUGING_TYPE_HF)

//  ULDP_GAUGING_HF_PLL : Set the gauging HF versus PLL clock
//===========================================================
#define ULDP_GAUGING_HF_PLL (* (volatile SYS_UWORD16 *) ULDP_GAUGING_CTRL_REG |= ULDP_SEL_HF_PLL)


//  ULDP_GET_IT_GAG : Return if the interrupt it gauging occurence
//================================================================
#define ULDP_GET_IT_GAG ((* (volatile SYS_UWORD16 *) ULDP_GAUGING_STATUS_REG) & ULDP_IT_GAUGING )

//  ULDP_GET_OVF_HF : Return overflow occured on the HF counter  
//=============================================================
#define ULDP_GET_OVF_HF (((* (volatile SYS_UWORD16 *) ULDP_GAUGING_STATUS_REG) & ULDP_OVF_HF)>>1)

//  ULDP_GET_OVF_32 : Return overflow occured on the 32 counter  
//=============================================================
#define ULDP_GET_OVF_32 (((* (volatile SYS_UWORD16 *) ULDP_GAUGING_STATUS_REG) & ULDP_OVF_32)>>2)

//  ULDP_TIMER_INIT : Load the  timer_init value
//=========================================================
#define ULDP_TIMER_INIT(value) ((* (volatile SYS_UWORD16 *)ULDP_GSM_TIMER_INIT_REG) = value)

//  READ_ULDP_TIMER_INIT : Read the  timer_init value
//=========================================================
#define READ_ULDP_TIMER_INIT (* (volatile SYS_UWORD16 *)ULDP_GSM_TIMER_INIT_REG)

//  READ_ULDP_TIMER_VALUE : Read the  timer_init value
//=========================================================
#define READ_ULDP_TIMER_VALUE (* (volatile SYS_UWORD16 *)ULDP_GSM_TIMER_VALUE_REG)

//  ULDP_TIMER_LD : Load the timer with timer_init value
//=========================================================
#define ULDP_TIMER_LD ((* (volatile SYS_UWORD16 *)ULDP_GSM_TIMER_CTRL_REG) |= ULDP_TM_LOAD)

//  ULDP_TIMER_FREEZE : Freeze the timer 
//=========================================================
#define ULDP_TIMER_FREEZE ((* (volatile SYS_UWORD16 *)ULDP_GSM_TIMER_CTRL_REG) |= ULDP_TM_FREEZE)

//  ULDP_GSM_TIME_START : Run the GSM timer
//=========================================
#define ULDP_TIMER_START ((* (volatile SYS_UWORD16 *)ULDP_GSM_TIMER_CTRL_REG) &= ~ULDP_TM_FREEZE)

//  ULDP_GET_IT_TIMER : Return the it GSM timer occurence
//===========================================================
#define ULDP_GET_IT_TIMER ((* (volatile SYS_UWORD16 *) ULDP_GSM_TIMER_IT_REG) & ULPD_IT_TIMER_GSM )



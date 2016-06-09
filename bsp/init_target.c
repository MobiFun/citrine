/*
 * Init_Target() is the first function called from Application_Initialize().
 * But unfortunately, our TCS211 semi-src has this function in a binary lib.
 * I was able to find a conditioned-out version in the LoCosto source that
 * seems to be a fit - so I'm going to massage it a bit to match the sequence
 * of operations seen in the disassembly of our reference binary.
 */

#include "../include/config.h"
#include "../include/sys_types.h"

#include "mem.h"
#include "clkm.h"
#include "armio.h"
#include "dma.h"
#include "timer.h"
#include "inth.h"
#include "iq.h"
#include "rhea_arm.h"
#include "ulpd.h"

#if !CONFIG_INCLUDE_L1
#include "timer2.h"
#endif

/* TPU_FREEZE is defined in l1_const.h */
#include "../L1/include/l1_confg.h"
#include "../L1/include/l1_const.h"

void Init_Target(void)
{
#if 1 //(PSP_STANDALONE == 0)
    // RIF/SPI rising edge clock for ULYSSE
    //--------------------------------------------------
    #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3) || (ANALOG == 11))
      #if ((CHIPSET >= 3))
        #if (CHIPSET == 12)
          F_CONF_RIF_RX_RISING_EDGE;
          F_CONF_SPI_RX_RISING_EDGE;
        #elif (CHIPSET == 15)
	     //do the DRP init here for Locosto
	     #if (L1_DRP == 1)
	     //  drp_power_on(); This should be done after the script is downloaded.
	     #endif
        #else
          #if (BOARD==35)
            *((volatile SYS_UWORD16 *) ASIC_CONF) = 0x2000;
          #elif CONFIG_TARGET_PIRELLI	// from disasm of original fw
            *((volatile SYS_UWORD16 *) ASIC_CONF) = 0x6050;
          #else
            *((volatile SYS_UWORD16 *) ASIC_CONF) = 0x6000;
          #endif   /* (BOARD == 35) */
        #endif
      #endif
    #endif   /* ANLG(ANALOG)) */

    #if 0 //(OP_L1_STANDALONE == 1)
      #if (BOARD == 40) || (BOARD == 41) || \
            (BOARD == 42) || (BOARD == 43) || (BOARD == 45)
        // enable 8 Ohm amplifier for audio on D-sample
        AI_ConfigBitAsOutput (1);
        AI_SetBit(1);
      #elif (BOARD == 70) || (BOARD == 71)
	  //Locosto I-sample or UPP costo board.BOARD
	  // Initialize the ARMIO bits as per the I-sample spec
	  // FIXME
      #endif
    #endif   /* (OP_L1_STANDALONE == 1) */
#endif /* PSP_STANDALONE ==0 */

    // Watchdog
    //--------------------------------------------------
    TM_DisableWatchdog();    /* Disable Watchdog */
    #if (CHIPSET == 12) || (CHIPSET == 15)
      TM_SEC_DisableWatchdog();
    #endif

    freecalypso_disable_bootrom_pll();

    #if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15))

      /*
       *  Enable/Disable of clock switch off for INTH, TIMER, BRIDGE and DPLL modules
       */
      // IRQ, Timer and bridge may SLEEP
      // In first step, same configuration as SAMSON
      //--------------------------------------------------
      #if (CHIPSET == 12)
        CLKM_INITCNTL(CLKM_IRQ_DIS | CLKM_TIMER_DIS | CLKM_BRIDGE_DIS | CLKM_DPLL_DIS);
      #elif (CHIPSET == 15)
CLKM_INITCNTL(CLKM_IRQ_DIS | CLKM_TIMER_DIS | CLKM_CPORT_EN | CLKM_BRIDGE_DIS | 0x8000 ); /* CLKM_DPLL_DIS is remove by Ranga*/

      #else
        CLKM_CNTL_OR(CLKM_IRQ_DIS | CLKM_TIMER_DIS);

        // Select VTCXO input frequency
        //--------------------------------------------------
        CLKM_UNUSED_VTCXO_26MHZ;

        // Rita RF uses 26MHz VCXO
        #if (RF_FAM == 12)
          CLKM_USE_VTCXO_26MHZ;
        #endif
        // Renesas RF uses 26MHz on F-sample but 13MHz on TEB
        #if (RF_FAM == 43) && (BOARD == 46)
          CLKM_USE_VTCXO_26MHZ;
        #endif
      #endif

      // Control HOM/SAM automatic switching
      //--------------------------------------------------
      *((volatile unsigned short *) CLKM_CNTL_CLK) &= ~CLKM_EN_IDLE3_FLG;

      /*
       * Disassembly of Init_Target() in init.obj in main.lib in the
       * Leonardo reference version reveals that the code does the
       * following at this point:
       */
      RHEA_INITRHEA(0,0,0xFF);
      DPLL_INIT_BYPASS_MODE(DPLL_BYPASS_DIV_1);
      DPLL_INIT_DPLL_CLOCK(DPLL_LOCK_DIV_1, 8);
      CLKM_InitARMClock(0x00, 2, 0); /* no low freq, no ext clock, div by 1 */
      /* at this point the original code sets up the memory wait states */
      /* we'll do it differently */
      RHEA_INITAPI(0,1);
      RHEA_INITARM(0,0);
      DPLL_SET_PLL_ENABLE;

      /*
       *  Disable and Clear all pending interrupts
       */
      #if (CHIPSET == 12) || (CHIPSET == 15)
        F_INTH_DISABLE_ALL_IT;           // MASK all it
        F_INTH2_VALID_NEXT(C_INTH_IRQ);  // reset current IT in INTH2 IRQ
        F_INTH_VALID_NEXT(C_INTH_IRQ);   // reset current IT in INTH IRQ
        F_INTH_VALID_NEXT(C_INTH_FIQ);   // reset current IT in INTH FIQ
        F_INTH_RESET_ALL_IT;             // reset all IRQ/FIQ source
      #else
        INTH_DISABLEALLIT;
        INTH_RESETALLIT;
        INTH_CLEAR;                 /* reset IRQ/FIQ source */
      #endif

      // INTH
      //--------------------------------------------------
      #if (CHIPSET == 12) || (CHIPSET == 15)
        #if (GSM_IDLE_RAM != 0)
          f_inth_setup((T_INTH_CONFIG *)a_inth_config_idle_ram);   // setup configuration IT handlers
        #else
          f_inth_setup((T_INTH_CONFIG *)a_inth_config);   // setup configuration IT handlers
        #endif
      #else
        IQ_SetupInterrupts();
      #endif

      // DMA
      //--------------------------------------------------
      // channel0 = Arm, channel1 = Lead, channel2 = forced to Arm, channel3=forced to Arm, dma_burst = 0001, priority = same
      #if 1 //(OP_L1_STANDALONE == 0)
        DMA_ALLOCDMA(1,0,1,1);  // Channel 1 used by DSP with RIF RX
      #endif

      /* CHIPSET = 4 or 7 or 8 or 10 or 11 or 12 */

    #else

      // RHEA Bridge
      //--------------------------------------------------
      // ACCES_FAC_0 = 0, ACCES_FAC_1 = 0 ,TIMEOUT = 0x7F
      RHEA_INITRHEA(0,0,0x7F);

      #if (CHIPSET == 6)
        // WS_H = 1 , WS_L = 15
        RHEA_INITAPI(1,15);          // should be 0x01E1 for 65 Mhz
      #else
        // WS_H = 0 , WS_L = 7
        RHEA_INITAPI(0,7);           // should be 0x0101 for 65 Mhz
      #endif

      // Write_en_0 = 0 , Write_en_1 = 0
      RHEA_INITARM(0,0);

      // INTH
      //--------------------------------------------------
      INTH_DISABLEALLIT;          // MASK all it
      INTH_CLEAR;                 // reset IRQ/FIQ source
      IQ_SetupInterrupts();

      // DMA
      //--------------------------------------------------
      // channel0 = Arm, channel1 = Lead, dma_burst = 0001, priority = same
      DMA_ALLOCDMA(1,0,1,1);      // should be 0x25   (channel 1 = lead)

      #if (CHIPSET == 6)
        // Memory WS configuration for ULYSS/G1 (26 Mhz) board
        //-----------------------------------------------------
        MEM_INIT_CS2(2,MEM_DVS_16,MEM_WRITE_EN,0);
      #endif

      // CLKM
      //--------------------------------------------------
      CLKM_InitARMClock(0x00, 2); /* no low freq, no ext clock, div by 1 */

      #if (CHIPSET == 6)
        CLKM_INITCNTL(CLKM_IRQ_DIS | CLKM_BRIDGE_DIS | CLKM_TIMER_DIS | CLKM_VTCXO_26);
      #else
        CLKM_INITCNTL(CLKM_IRQ_DIS | CLKM_BRIDGE_DIS | CLKM_TIMER_DIS);
      #endif

    #endif   /* CHIPSET = 4 or 7 or 8 or 10 or 11 or 12 */

    // Freeze ULPD timer ....
    //--------------------------------------------------
    *((volatile SYS_UWORD16 *) ULDP_GSM_TIMER_INIT_REG ) = 0;
    *((volatile SYS_UWORD16 *) ULDP_GSM_TIMER_CTRL_REG ) = TPU_FREEZE;

    // reset INC_SIXTEEN and INC_FRAC
    //--------------------------------------------------
    #if 0 //(OP_L1_STANDALONE == 1)
      l1ctl_pgm_clk32(DEFAULT_HFMHZ_VALUE,DEFAULT_32KHZ_VALUE);
    #else
      ULDP_INCSIXTEEN_UPDATE(132);    //32768.29038  =>132, 	32500 => 133
                                      // 26000 --> 166
      ULDP_INCFRAC_UPDATE(15840);     //32768.29038  =>15840,	32500 => 21845
                                      // 26000 --> 43691
    #endif   /*  OP_L1_STANDALONE */

    // program ULPD WAKE-UP ....
    //=================================================
    #if (CHIPSET == 2)
       *((volatile SYS_UWORD16 *)ULDP_SETUP_FRAME_REG)  = SETUP_FRAME;  // 2 frame
       *((volatile SYS_UWORD16 *)ULDP_SETUP_VTCXO_REG)  = SETUP_VTCXO;  // 31 periods
       *((volatile SYS_UWORD16 *)ULDP_SETUP_SLICER_REG) = SETUP_SLICER; // 31 periods
       *((volatile SYS_UWORD16 *)ULDP_SETUP_CLK13_REG)  = SETUP_CLK13;  // 31 periods
    #else
       *((volatile SYS_UWORD16 *)ULDP_SETUP_FRAME_REG)  = SETUP_FRAME;  // 3 frames
       *((volatile SYS_UWORD16 *)ULDP_SETUP_VTCXO_REG)  = SETUP_VTCXO;  // 0 periods
       *((volatile SYS_UWORD16 *)ULDP_SETUP_SLICER_REG) = SETUP_SLICER; // 31 periods
       *((volatile SYS_UWORD16 *)ULDP_SETUP_CLK13_REG)  = SETUP_CLK13;  // 31 periods
       *((volatile SYS_UWORD16 *)ULPD_SETUP_RF_REG)     = SETUP_RF;     // 31 periods
    #endif

    #if (CHIPSET == 15)
      *((volatile SYS_UWORD16 *)ULPD_DCXO_SETUP_SLEEPN)    = SETUP_SLEEPZ;    // 0
      *((volatile SYS_UWORD16 *)ULPD_DCXO_SETUP_SYSCLKEN)  = SETUP_SYSCLKEN;  // 255 clocks of 32 KHz for 7.8 ms DCXO delay for Locosto
	  *((volatile SYS_UWORD16 *)0xFFFEF192) = 0x1; //CLRZ
  	  *((volatile SYS_UWORD16 *)0xFFFEF190) = 0x2; //SLPZ
	  *((volatile SYS_UWORD16 *)0xFFFEF18E)= 0x2; //SYSCLKEN
	  *((volatile SYS_UWORD16 *)0xFFFEF186) = 0x2; //CLK13_EN
	  *((volatile SYS_UWORD16 *)0xFFFEF18A) = 0x2; //DRP_DBB_SYSCLK
    #endif

    // Set Gauging versus HF (PLL)
    //=================================================
    ULDP_GAUGING_SET_HF;                // Enable gauging versus HF
    ULDP_GAUGING_HF_PLL;                // Gauging versus PLL

    // current supply for quartz oscillation
    //=================================================
    #if 0 //(OP_L1_STANDALONE == 1)
      #if ((CHIPSET != 9) && (CHIPSET != 12) && (CHIPSET !=15)) // programming model changed for Ulysse C035, stay with default value
        *(volatile SYS_UWORD16 *)QUARTZ_REG  = 0x27;
      #endif
    #else
      #if ((BOARD == 6) || (BOARD == 8) || (BOARD == 9) || (BOARD == 35) || (BOARD == 40) || (BOARD == 41))
        *((volatile SYS_UWORD16 *)QUARTZ_REG)  = 0x27;
      #elif (BOARD == 7)
        *((volatile SYS_UWORD16 *)QUARTZ_REG)  = 0x24;
      #endif
    #endif   /* OP_L1_STANDALONE */

    // stop Gauging if any (debug purpose ...)
    //--------------------------------------------------
    if ( *((volatile SYS_UWORD16 *) ULDP_GAUGING_CTRL_REG) & ULDP_GAUGING_EN)
    {
      volatile int j;
      ULDP_GAUGING_STOP; /* Stop the gauging */
      /* wait for gauging it*/
      // one 32khz period = 401 periods of 13Mhz
      for (j=1; j<50; j++);
      while (! (* (volatile SYS_UWORD16 *) ULDP_GAUGING_STATUS_REG) & ULDP_IT_GAUGING);
    }

    #if 1 //(OP_L1_STANDALONE == 0)
      AI_ClockEnable ();

      #if (BOARD == 7)
        // IOs configuration of the B-Sample in order to optimize the power consumption
        AI_InitIOConfig();

        // Set LPG instead of DSR_MODEM
        *((volatile SYS_UWORD16 *) ASIC_CONF) |= 0x40;
        // Reset the PERM_ON bit of LCR_REG
        *((volatile SYS_UWORD16 *) MEM_LPG) &= ~(0x80);
      #elif ((BOARD == 8) || (BOARD == 9))
        // IOs configuration of the C-Sample in order to optimize the power consumption
        AI_InitIOConfig();

        // set the debug latch to 0x00.
        *((volatile SYS_UWORD8 *) 0x2800000) = 0x00;
      #elif ((BOARD == 35) || (BOARD == 46))
        AI_InitIOConfig();
        // CSMI INTERFACE
        // Initialize CSMI clients for GSM control
        // and Fax/Data services
          CSMI_Init();
          GC_Initialize();  // GSM control initialization
          CU_Initialize();  // Trace initialization
          CF_Initialize();  // Fax/Data pre-initialization
      #elif ((BOARD == 40) || (BOARD == 41))
        // IOs configuration of the D-Sample in order to optimize the power consumption
        AI_InitIOConfig();

        #ifdef BTEMOBILE
          // Reset BT chip by toggling the Island's nRESET_OUT signal
          *((volatile SYS_UWORD16 *) 0xFFFFFD04) |= 0x04;
          *((volatile SYS_UWORD16 *) 0xFFFFFD04) &= ~(0x4);
        #endif

	#if 0	// FreeCalypso
        // set the debug latch to 0x0000.
 	    *((volatile SYS_UWORD16 *) 0x2700000) = 0x0000;
	#endif
      #elif ((BOARD == 70) || (BOARD == 71))
	    AI_InitIOConfig();
	    /* Mark The System configuration According to I-Sample */
		/* Adding GPIO Mux Setting Here */
		pin_configuration_all(); // Init Tuned for Power Management
		/* A22 is Enabled in int.s hence not Here */
		/* FIXME: PULL_UP Enable and PULL UP Values Need to revisited */

	/* Add code to find out the manufacture id of NOR flash*/

        // Copy ffsdrv_device_id_read() function code to RAM. The only known
        // way to determine the size of the code is to look either in the
        // linker-generated map file or in the assember output file.
        ffsdrv_copy_code_to_ram((UWORD16 *) detect_code,
                                (UWORD16 *) &ffsdrv_device_id_read,
                                sizeof(detect_code));

        // Combine bit 0 of the thumb mode function pointer with the address
        // of the code in RAM. Then call the detect function in RAM.
        myfp = (pf_t) (((int) &ffsdrv_device_id_read & 1) | (int) detect_code);
        (*myfp)(0x06000000, &manufact, device_id);

	enable_ps_ram_burst();

	if( 0x7e == device_id[0] )
	{
	   enable_flash_burst();
	   flash_device_id = 0x7E;
	}
	else
	{
	   enable_flash_burst_mirror();
   	   flash_device_id = 0;
	}

	/* FreeCalypso: a bunch of dead code cut out */

      #endif // BOARD

      // Enable HW Timers 1 & 2
      TM_EnableTimer (1);
      TM_EnableTimer (2);

      #if !CONFIG_INCLUDE_L1
	Dtimer2_Init_cntl (1875, 1, 0, 1);
	Dtimer2_Start (1);
      #endif

    #endif  /* (OP_L1_STANDALONE == 0) */

}

/*
 * Init_Unmask_IT() is the last function called from Application_Initialize();
 * it also had to be reconstructed from disassembly.
 */

void Init_Unmask_IT(void)
{
#if CONFIG_INCLUDE_L1
	IQ_Unmask(IQ_FRAME);
#endif
	IQ_Unmask(IQ_UART_IRDA_IT);
	IQ_Unmask(IQ_UART_IT);
#if 0
	IQ_Unmask(IQ_ARMIO);
#endif
#if L1_DYN_DSP_DWNLD
	IQ_Unmask(IQ_API);
#endif
#if !CONFIG_INCLUDE_L1
	IQ_Unmask(IQ_TIM2);
#endif
}

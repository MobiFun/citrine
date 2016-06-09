/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_CONFG.H
 *
 *        Filename l1_confg.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

#ifndef __L1_CONFG_H__
#define __L1_CONFG_H__

// Traces...
// TRACE_TYPE == 1,2,3 are used in standalone mode (L2-L3 Simul) with USART
// TRACE_TYPE == 4 is used on A-sample only (with UART): L1 + protocol stack
// TRACE_TYPE == 1 -> L1/L3 interface trace
// TRACE_TYPE == 2 -> Trace mode: ~33~~1~011...
// TRACE_TYPE == 3 -> same as above (2) plus FER or stats trace
// TRACE_TYPE == 4 -> L1/L3 interface trace on A-sample with protocol stack
// TRACE_TYPE == 5 -> trace for full simulation
// TRACE_TYPE == 6 -> CPU load trace for hisr
// TRACE_TYPE == 7 -> CPU LOAD trace for layer 1 hisr for all TDMA. Output on
//                    UART at 38400 bps =>
//                    format : <hisr cpu value in microseconds> <frame number>

// Code PB reported workaround
//------------------------------


// Code Version possible choices
//------------------------------
#define SIMULATION     1
#define NOT_SIMULATION 2

// RCL functions Version possible choices
//------------------------------
#define       POLL_FORCED     0
#define       RLC_SCENARIO    1
#define       MODEM_FLOW      2

// possible choices for UART trace output
//------------------------------
#define       MODEM_UART     0
#define       IRDA_UART      1
#if (CHIPSET == 12)
  #define     MODEM2_UART    2
#endif

//============
// CODE CHOICE
//============
#if 0
#if (OP_L1_STANDALONE==0)
#define CODE_VERSION NOT_SIMULATION
#else // OP_L1_STANDALONE
#ifdef WIN32
#define CODE_VERSION  SIMULATION
#else // WIN32
#define CODE_VERSION  NOT_SIMULATION
#endif // WIN32
#endif // OP_L1_STANDALONE
#endif // #if 0

/* FreeCalypso */
#define	CODE_VERSION	NOT_SIMULATION
#define	AMR		1
#define	L1_12NEIGH	1
#define	L1_DYN_DSP_DWNLD	0	/* for now */
#define	L1_EOTD		0
#define	L1_GTT		0
#define	ORDER2_TX_TEMP_CAL	1
#define	TRACE_TYPE	4
#define	VCXO_ALGO	1

/* TESTMODE will be enabled with feature l1tm */

#if CONFIG_AUDIO
#  define AUDIO_TASK	1  // Enable the L1 audio features
#  define MELODY_E2	1
#endif

#if CONFIG_GPRS
#  define L1_GPRS	1
#else
#  define L1_GPRS	0
#endif

//---------------------------------------------------------------------------------
// Test with full simulation.
//---------------------------------------------------------------------------------
#if (CODE_VERSION == SIMULATION)

  // Test Scenari...
  #define SCENARIO_FILE          1  // Test Scenario comes from input files.
  #define SCENARIO_MEM           0  // Test Scenario comes from RAM.

  // Traces...
  #undef TRACE_TYPE
  #define TRACE_TYPE             5
  #define LOGFILE_TRACE          1  // trace in an output logfile
  #define FLOWCHART              0  // Message sequence/flow chart trace.
  #define NUCLEUS_TRACE          0  // Nucleus error trace
  #define EOTD_TRACE             1  // EOTD log trace
  #define TRACE_FULL_NAME        0  // display full fct names after a PM/COM error

  #define L2_L3_SIMUL            1  // Layer 2 & Layer 3 simulated, main within NU_MAIN.C, trace possible.

  // Control algorithms...
  #define AFC_ALGO               1  // AFC algorithm.
  #define TOA_ALGO               1  // TOA algorithm.
  #define AGC_ALGO               1  // AGC algorithm.
  #define TA_ALGO                0  // TA (Timing Advance) algorithm.
  #undef VCXO_ALGO
  #define VCXO_ALGO              0  // VCXO algo
  #undef DCO_ALGO
  #define DCO_ALGO               0  // DCO algo (TIDE)
  #undef ORDER2_TX_TEMP_CAL
  #define ORDER2_TX_TEMP_CAL     0  // TX Temperature Compensation Algorithm selection


  #define FACCH_TEST             0  // FACCH test enabled.

  #define ADC_TIMER_ON           0  // Timer for ADC measurements
  #define AFC_ON                 1  // Enable of the Omega AFC module

  #define AUDIO_TASK             1  // Enable the L1 audio features
  #define AUDIO_SIMULATION       1  // Audio simulator for the audio tasks (works only with the new audio design i.e. AUDIO_TASK=1)
  #define AUDIO_L1_STANDALONE    0  // Flag to enable the audio simulator used with the L1 stand-alone (works only with the new audio design i.e. AUDIO_TASK=1)

  #define GTT_SIMULATION         1  // Gtt simulator for the gtt tasks (works only with if L1_GTT=1)
  #define TTY_SYNC_MCU           1  // TTY WORKAROUND BUG03401
  #define TTY_SYNC_MCU_2         1  // 
  #define L1_GTT_FIFO_TEST_ATOMIC 0 //
  #define NEW_WKA_PATCH          0
  #define OPTIMISED              1

  #define L1_RECOVERY            0  // L1 recovery

  #undef L1_GPRS
  #define L1_GPRS                1  // GPRS L1: MS supporting both Circuit Switched and Packet (GPRS) capabilities

  #undef AMR
  #define AMR                    1  // AMR version 1.0 supported

  #undef L1_12NEIGH
  #define L1_12NEIGH             1  // new L1-RR interface for 12 neighbour cells

  #undef L1_GTT
  #define L1_GTT                 1  // Enable Global Text Telephony feature for simulation

  #undef  OP_L1_STANDALONE
  #define OP_L1_STANDALONE       1  // Selection of code for L1 stand alone

  #undef  OP_RIV_AUDIO
  #define OP_RIV_AUDIO           0  // Selection of code for Riviera audio

  #undef OP_WCP
  #define OP_WCP                 0  // No WCP integration
//---------------------------------------------------------------------------------
// Test with H/W platform.
//---------------------------------------------------------------------------------
#elif (CODE_VERSION == NOT_SIMULATION)

  #define WA_PCTM_AGC_PARAMS 0 // to work by default with 4 parameters to calibration (compatible with PCTM if 1) 
  // Work around about Calypso RevA: the bus is floating (Cf PB01435)
  // (corrected with Calypso ReV B and Calypso C035)
  #if (CHIPSET == 7)
    #define W_A_CALYPSO_BUG_01435 1
  #else
    #define W_A_CALYPSO_BUG_01435 0
  #endif


  // for AMR thresolds definition CQ22226
  #define AMR_THRESHOLDS_WORKAROUND 1

  #if (L1_GTT==1)
    #define TTY_SYNC_MCU 1
    #define TTY_SYNC_MCU_2 1
    #define L1_GTT_FIFO_TEST_ATOMIC 0
    #define NEW_WKA_PATCH          0
    #define OPTIMISED              1
  #else
    #define TTY_SYNC_MCU_2 0
    #define L1_GTT_FIFO_TEST_ATOMIC 0
    #define TTY_SYNC_MCU 0
    #define NEW_WKA_PATCH          0
    #define OPTIMISED              0

  #endif
 
  // Traces...
  #define NUCLEUS_TRACE        0  // Nucleus error trace
  #define FLOWCHART            0  // Message sequence/flow chart trace.
  #define LOGFILE_TRACE        0  // trace in an output logfile
  #define TRACE_FULL_NAME      0  // display full fct names after a PM/COM error

  // Test Scenari...
  #define SCENARIO_FILE          0  // Test Scenario comes from input files.
  #define SCENARIO_MEM           1  //  // Test Scenario comes from RAM.

  #if (OP_L1_STANDALONE == 1)
    #define L2_L3_SIMUL            1  // Layer 2 & Layer 3 simulated, main within NU_MAIN.C, trace possible.
  #else
    #define L2_L3_SIMUL            0
  #endif

  // Control algorithms...
  #define AFC_ALGO               1  // AFC algorithm.
  //TOA Algorithm needs to be on for TestMode, otherwise no dedic test will be succesful!!!
  #define TOA_ALGO               1  // TOA algorithm.
  #define AGC_ALGO               1  // AGC algorithm.
  #define TA_ALGO                1  // TA (Timing Advance) algorithm.

  #define FACCH_TEST             0  // FACCH test enabled.

  #define ADC_TIMER_ON           0  // Timer for ADC measurements
  #define AFC_ON                 1  // Enable of the Omega AFC module

#if 0
  /* FreeCalypso: moved to config section above */
  #define AUDIO_TASK             1  // Enable the L1 audio features
#endif
  #define AUDIO_SIMULATION       0  // Audio simulator for the audio tasks (works only with the new audio design i.e. AUDIO_TASK=1)
  #if (OP_L1_STANDALONE == 1)
    #define AUDIO_L1_STANDALONE    1  // Flag to enable the audio simulator used with the L1 stand-alone (works only with the new audio design i.e. AUDIO_TASK=1)
  #else
    #define AUDIO_L1_STANDALONE    0
  #endif

  #define GTT_SIMULATION         0  // Gtt simulator for the gtt tasks (works only with if L1_GTT=1)

  #define OP_BT                  0  // Simulation of ISLAND (BLUETOOTH) sleep management 

  #define L1_RECOVERY            1  // L1 recovery


  #if (L1_GPRS == 1)
    #define RLC_VERSION            RLC_SCENARIO
    #if (RLC_VERSION == RLC_SCENARIO)
      #define RLC_DL_BLOCK_STAT    0  // Works with RLC_VERSION = RLC_SCENARIO
                                      // output stat on CRC error blocks
                                      // The user must enter the cs type and
                                      // the number of frames desired.
    #else
      #define RLC_DL_BLOCK_STAT    0  // Default value; Never change it
    #endif

    #if (OP_L1_STANDALONE == 1)
      #define DSP_BACKGROUND_TASKS     1 // Enable the TEST of DSP background.tasks
                                         // activated by a layer 3 message (BG_TASK_START (<task number>))
                                         // deactivated by a layer 3 message (BG_TASK_STOP (<task number>))
                                         // Warning : Works only with DSP>=31
    #else
      #define DSP_BACKGROUND_TASKS   0
    #endif

  #else
    #define DSP_BACKGROUND_TASKS     0
    #define RLC_DL_BLOCK_STAT        0  // Default value; Never change it
  #endif
#endif

// Audio tasks selection
//-----------------------

#if (AUDIO_TASK == 1)
  #define KEYBEEP          1  // Enable keybeep feature
  #define TONE             1  // Enable tone feature
  // Temporary modification for protocol stack compatibility - GSMLITE will be removed
  #if (OP_L1_STANDALONE == 1)
    #define GSMLITE 1
  #endif
  #if ((OP_L1_STANDALONE == 1) || (!GSMLITE))
    #define MELODY_E1        1  // Enable melody format E1 feature
    #define VOICE_MEMO       1  // Enable voice memorization feature

    #define FIR              1  // Enable FIR feature
    #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36)
      #define AUDIO_MODE       1  // Enable Audio mode feature
    #else
      #define AUDIO_MODE        0  // Disable Audio mode feature
    #endif
  #else
    #define MELODY_E1        0  // Disable melody format E1 feature
    #define VOICE_MEMO       0  // Disable voice memorization feature
    #if (MELODY_E2)
	    #define FIR              1  // Enable FIR feature  
	  #else
      #define FIR              0  // Disable FIR feature  
    #endif

    #define AUDIO_MODE       0  // Disable Audio mode feature
  #endif
  // Define CPORT for ESample only
  #if ((CHIPSET == 12) && ((DSP == 35) || (DSP == 36))) 
    #define L1_CPORT         1  // Enable cport feature
  #else
    #define L1_CPORT         0  // Disable cport feature
  #endif

#else
  #define KEYBEEP           0  // Enable keybeep feature
  #define TONE              0  // Enable tone feature
  #define MELODY_E1         0  // Enable melody format E1 feature
  #define VOICE_MEMO        0  // Enable voice memorization feature

  #define FIR               0  // Enable FIR feature
  #define AUDIO_MODE        0  // Enable Audio mode feature
  #define L1_CPORT          0  // Enable cport feature
#endif

#define L1_AUDIO_BACKGROUND_TASK (SPEECH_RECO | MELODY_E2) // audio background task is used by speech reco and melody_e2
#if (OP_RIV_AUDIO == 1)
  #define L1_AUDIO_DRIVER L1_VOICE_MEMO_AMR // Riviera audio driver (only Voice Memo AMR is available)
#endif


// Vocoder selections
//-------------------

#define FR        1            // Full Rate
#define FR_HR     2            // Full Rate + Half Rate
#define FR_EFR    3            // Full Rate + Enhanced Full Rate
#define FR_HR_EFR 4            // Full Rate + Half Rate + Enhanced Full Rate

// Standard (frequency plan) selections
//-------------------------------------

#define GSM             1            // GSM900.
#define GSM_E           2            // GSM900 Extended.
#define PCS1900         3            // PCS1900.
#define DCS1800         4            // DCS1800.
#define DUAL            5            // Dual Band (GSM900 + DCS 1800 bands)
#define DUALEXT         6            // Dual Band (E-GSM900 + DCS 1800 bands)
#define GSM850          7            // GSM850 Band
#define DUAL_US         8            // PCS1900 + GSM850

/*------------------------------------*/
/* Power Management                   */
/*------------------------------------*/
#define PWR_MNGT  1            // POWER management active if l1_config.pwr_mngt=1


/*---------------------------------------------------------------------------*/
/* DSP configurations                                                        */
/* ------------------                                                        */
/*  DSP      | FR| HR|EFR|14.4| SPEED   |12LA68|12LA68 |4L32|AEC| MCU/DSP    */
/* (version) |   |   |   |    |         |POLE80|POLE112|    |/NS| interface  */
/* ----------+---+---+---+----+---------+------+-------+----+---+----------  */
/*  0 (821)  | x |   |   |    | 39Mhz   |  x   |       |    |   | 1          */
/* ----------+---+---+---+----+---------+------+-------+----+---+----------  */
/*  1 (830)  | x |   |   |    | 39Mhz   | (1)  |       | x  |   | 1          */
/* ----------+---+---+---+----+---------+------+-------+----+---+----------  */
/*  2 (912)  | x | x |   |    | 58.5Mhz |  x   |       |    |   | 2          */
/* ----------+---+---+---+----+---------+------+-------+----+---+----------  */
/*  3 (10xx) | x |   | x | x  | 65Mhz   |  x   |       |    | x | 3          */
/* ----------+---+---+---+----+---------+------+-------+----|---+----------  */
/*  4 (11xx) | x | x | x | x  | 65Mhz   |  x   |  x (3)|    | x | 3          */
/* ----------+---+---+---+----+---------+------+-------+----+---+----------  */
/*  5 (830)  | x |   |   |    | 39Mhz   |  x   |       |    |   | 1          */
/* ----------+---+---+---+----+---------+------+-------+----+---+----------  */
/*  6 (11xx) | x | x | x | x  | 65Mhz   |  x   |  x (3)|    |(2)| 3          */
/* ----------+---+---+---+----+---------+------+-------+----+---+----------  */
/*                                                                           */
/*(1) this version can be loaded on a 12LA68/POLE80 but the RIF/DL problem is*/
/*    not corrected.                                                         */
/*                                                                           */
/*(2) AEC is disabled at DSP level but L1 must be compiled with MCU/DSP      */
/*    interface which support AEC, therefore AEC is defined as 1.            */
/*                                                                           */
/*(3) Pole112 include RIF DL correction. No patch is needed if this one only */
/*    include RIF/DL problem.                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#if   (DSP == 16 || DSP == 17)

/*  #define CLKMOD1    0x414e  // ...
  #define CLKMOD2    0x414e  // ...65 Mips
  #define CLKSTART   0x29    // ...65 Mips */

  #define CLKMOD1    0x4006  // ...
  #define CLKMOD2    0x4116  // ...65 Mips pll free
  #define CLKSTART   0x29    // ...65 Mips

/*  #define CLKMOD1     0x2116  //This settings force the DSP to never enteridle
  #define CLKMOD2     0x2116  //In this case the PLL will be always on. 39 Mips
  #define CLKSTART    0x25    // ...39 Mips */

  #define VOC        FR_HR_EFR // FR + HR + EFR.
  #define DATA14_4   1         // No 14.4 data allowed.
  #define AEC        1         // AEC/NS supported.
  #define MAP        3
  #define DSP_START  0x2000
  #define W_A_DSP1   0         // Work Around correcting pb in DSP: SACCH

  #define W_A_DSP_SR_BGD 0    // Work around about the DSP speech reco background task.

  /* DSP debug trace configuration */
  /*-------------------------------*/
  #if (MELODY_E2)
    // In case of the melody E2 the DSP trace must be disable because the
    // melody instrument waves are overlayed with DSP trace buffer

    // DSP debug trace API buufer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 7       // Real size is incremented by 1 for DSP write pointer.
  #else
    // DSP debug trace API buufer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 2047    // Real size is incremented by 1 for DSP write pointer.
  #endif

#elif   (DSP == 30)    // First GPRS.
  #define CLKMOD1    0x4006  // ...
  #define CLKMOD2    0x4116  // ...65 Mips pll free
  #define CLKSTART   0x29    // ...65 Mips

  #define VOC        FR_HR_EFR // FR + HR + EFR.
  #define DATA14_4   1         // No 14.4 data allowed.
  #define AEC        1         // AEC/NS not supported.
  #define MAP        3
  #define DSP_START  0x1F81
  #define W_A_DSP1   0         // Work Around correcting pb in DSP: SACCH
  #define ULYSSE      0

  #define W_A_DSP_SR_BGD 0    // Work around about the DSP speech reco background task.
#elif   (DSP == 31)    // ROM Code GPRS G0.
  #define CLKMOD1    0x4006  // ...
  #define CLKMOD2    0x4116  // ...65 Mips pll free
  #define CLKSTART   0x29    // ...65 Mips

  #define VOC        FR_HR_EFR // FR + HR + EFR (normaly FR_EFR : PBs).
  #define DATA14_4   1         // 14.4 data allowed.
  #define AEC        1         // AEC/NS not supported.
  #define MAP        3

  #define DSP_START  0x8763

  #define INSTALL_ADD            0x87c9 // Used to set gprs_install_address pointer
  #define INSTALL_ADD_WITH_PATCH 0x1352 // Used to set gprs_install_address pointer

  #define W_A_DSP1   0         // Work Around correcting pb in DSP: SACCH
  #define ULYSSE      0

  #define W_A_DSP_SR_BGD 0    // Work around about the DSP speech reco background task.
#elif   (DSP == 32)    // ROM Code GPRS G1.
  #define CLKMOD1    0x4006  // ...
  #define CLKMOD2    0x4116  // ...65 Mips pll free
  #define CLKSTART   0x29    // ...65 Mips

  #define VOC        FR_HR_EFR // FR + HR + EFR (normaly FR_EFR : PBs).
  #define DATA14_4   1         // 14.4 data allowed.
  #define AEC        1         // AEC/NS not supported.
  #define MAP        3

  #define DSP_START  0x8763

  #define INSTALL_ADD 0x87c9   // Used to set gprs_install_address pointer

  #define W_A_DSP1   0         // Work Around correcting pb in DSP: SACCH
  #define ULYSSE      0

  #define W_A_DSP_SR_BGD 0    // Work around about the DSP speech reco background task.
#elif   (DSP == 33)    // ROM Code GPRS.
  #define CLKMOD1    0x4006  // ...
  #define CLKMOD2    0x4116  // ...65 Mips pll free
  #define CLKSTART   0x29    // ...65 Mips
  #define C_PLL_CONFIG 0x154   // For VTCXO = 13 MHz and max DSP speed = 84.5 Mips
  #define VOC        FR_HR_EFR // FR + HR + EFR (normaly FR_EFR : PBs).
  #define AEC        1         // AEC/NS not supported.
  #if (OP_RIV_AUDIO == 0)
    #define L1_NEW_AEC 1
  #else
  // Available but not yet tuned with Riviera AUDIO    
  #define L1_NEW_AEC 0
  #endif
  #if ((L1_NEW_AEC) && (!AEC))
    // First undef the flag to avoid warnings at compilation time
    #undef AEC
    #define AEC 1
  #endif

  #define MAP        3

  #define DSP_START  0x7000

  #define INSTALL_ADD   0x7002 // Used to set gprs_install_address pointer

  #define W_A_DSP1   0         // Work Around correcting pb in DSP: SACCH
  #define ULYSSE      0

  #define W_A_DSP_SR_BGD 1    // Work around about the DSP speech reco background task.

  #if ( (CHIPSET != 12) && (CODE_VERSION == NOT_SIMULATION))

    #define W_A_DSP_IDLE3 1     // Work around to report DSP state to the ARM for Deep Sleep

                                // management.

								// DSP_IDLE3 is not supported in simulation

  #else
    #define W_A_DSP_IDLE3 0
  #endif

  // DSP software work-around config
  //  bit0 - Work-around to support CRTG.
  //  bit1 - DMA reset on critical DMA still running cases, refer to REQ01260.
  //  bit2 - Solve Read/Write BULDATA pointers Omega & Nausica issue, refer to BUG00650.
  //  bit3 - Solve IBUFPTRx reset IOTA issue, refer to BUG01911.

  #if    (ANALOG == 1)  // OMEGA / NAUSICA
    #define C_DSP_SW_WORK_AROUND 0x0006

  #elif  (ANALOG == 2)  // IOTA
    #define C_DSP_SW_WORK_AROUND 0x000E

  #elif  (ANALOG == 3)  // SYREN
    #define C_DSP_SW_WORK_AROUND 0x000E

  #endif

  /* DSP debug trace configuration */
  /*-------------------------------*/
  #if (MELODY_E2)
    // In case of the melody E2 the DSP trace must be disable because the
    // melody instrument waves are overlayed with DSP trace buffer

    // DSP debug trace API buufer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 7       // Real size is incremented by 1 for DSP write pointer.

    // DSP debug trace type config
    //             |<-------------- Features -------------->|<---------- Levels ----------->|
    // [15-8:UNUSED|7:TIMER|6:BURST|5:BUFFER|4:BUFFER HEADER|3:UNUSED|2:KERNEL|1:BASIC|0:ISR]
    #define C_DEBUG_TRACE_TYPE  0x0000  // Level = BASIC; Features = Timer + Buffer Header + Burst.

    #if (C_DEBUG_TRACE_TYPE != 0) && ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
      #define DSP_DEBUG_TRACE_ENABLE       1    // Enable DSP debug trace dumping capability
                                                // Currently not supported !
    #endif
  #else
    // DSP debug trace API buufer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 2047    // Real size is incremented by 1 for DSP write pointer.

    // DSP debug trace type config
    //             |<-------------- Features -------------->|<---------- Levels ----------->|
    // [15-8:UNUSED|7:TIMER|6:BURST|5:BUFFER|4:BUFFER HEADER|3:UNUSED|2:KERNEL|1:BASIC|0:ISR]
    #define C_DEBUG_TRACE_TYPE  0x0012  // Level = BASIC; Features = Buffer Header.

    #if (C_DEBUG_TRACE_TYPE != 0) && ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
    #define DSP_DEBUG_TRACE_ENABLE       1    // Enable DSP debug trace dumping capability (supported since patch 2090)
    #endif
  #endif
  /* d_error_status                */
  /*-------------------------------*/

  #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
    #define D_ERROR_STATUS_TRACE_ENABLE  1    // Enable d_error_status checking capability (supported since patch 2090)

    // masks to apply on d_error_status bit field for DSP patch 0x2061 or 0x2062
    #define DSP_DEBUG_GSM_MASK     0x0000
    #define DSP_DEBUG_GPRS_MASK    0x0f3d
  #endif

  #if DCO_ALGO
    // DCO type of scheduling
    #define C_CN_DCO_PARAM 0xA248
  #endif

#elif (DSP == 34)            // ROM Code GPRS AMR.
  #define CLKMOD1    0x4006  // ...
  #define CLKMOD2    0x4116  // ...65 Mips pll free
  #define CLKSTART   0x29    // ...65 Mips
  #define C_PLL_CONFIG 0x154   // For VTCXO = 13 MHz and max DSP speed = 84.5 Mips
  #define VOC        FR_HR_EFR // FR + HR + EFR (normaly FR_EFR : PBs).
  #define AEC        1         // AEC/NS not supported.
  #if (OP_RIV_AUDIO == 0)
    #define L1_NEW_AEC 1
  #else
  // Available but not yet tuned with Riviera AUDIO    
    #define L1_NEW_AEC 0
   #endif
  #if ((L1_NEW_AEC) && (!AEC))
    // First undef the flag to avoid warnings at compilation time
    #undef AEC
    #define AEC 1
  #endif
  #define MAP        3

  #define DSP_START  0x7000

  #define INSTALL_ADD   0x7002 // Used to set gprs_install_address pointer

  #define W_A_DSP1   0         // Work Around correcting pb in DSP: SACCH
  #define ULYSSE      0

  #define W_A_DSP_SR_BGD 1    // Work around about the DSP speech reco background task.

  #if ( (CHIPSET != 12) && (CODE_VERSION == NOT_SIMULATION))

    #define W_A_DSP_IDLE3 1     // Work around to report DSP state to the ARM for Deep Sleep

                                // management.

								// DSP_IDLE3 is not supported in simulation

  #else
    #define W_A_DSP_IDLE3 0
  #endif

  // DSP software work-around config
  //  bit0 - Work-around to support CRTG.
  //  bit1 - DMA reset on critical DMA still running cases, refer to REQ01260.
  //  bit2 - Solve Read/Write BULDATA pointers Omega & Nausica issue, refer to BUG00650.
  //  bit3 - Solve IBUFPTRx reset IOTA issue, refer to BUG01911.
  #if    (ANALOG == 1)  // OMEGA / NAUSICA
    #define C_DSP_SW_WORK_AROUND 0x0006

  #elif  (ANALOG == 2)  // IOTA
    #define C_DSP_SW_WORK_AROUND 0x000E

  #elif  (ANALOG == 3)  // SYREN
    #define C_DSP_SW_WORK_AROUND 0x000E

  #endif

  /* DSP debug trace configuration */
  /*-------------------------------*/
  #if (MELODY_E2)
    // In case of the melody E2 the DSP trace must be disable because the
    // melody instrument waves are overlayed with DSP trace buffer

    // DSP debug trace API buufer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 7       // Real size is incremented by 1 for DSP write pointer.

    // DSP debug trace type config
    //             |<-------------- Features -------------->|<---------- Levels ----------->|
    // [15-8:UNUSED|7:TIMER|6:BURST|5:BUFFER|4:BUFFER HEADER|3:UNUSED|2:KERNEL|1:BASIC|0:ISR]
    #define C_DEBUG_TRACE_TYPE  0x0000  // Level = BASIC; Features = Timer + Buffer Header + Burst.

    #if (C_DEBUG_TRACE_TYPE != 0) && ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
      #define DSP_DEBUG_TRACE_ENABLE       1    // Enable DSP debug trace dumping capability
                                                // Currently not supported !
    #endif
  #else
    // DSP debug trace API buufer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 2047    // Real size is incremented by 1 for DSP write pointer.

    // DSP debug trace type config
    //             |<-------------- Features -------------->|<---------- Levels ----------->|
    // [15-8:UNUSED|7:TIMER|6:BURST|5:BUFFER|4:BUFFER HEADER|3:UNUSED|2:KERNEL|1:BASIC|0:ISR]
    #define C_DEBUG_TRACE_TYPE  0x0012  // Level = BASIC; Features = Buffer Header.

    #if (C_DEBUG_TRACE_TYPE != 0) && ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
    #define DSP_DEBUG_TRACE_ENABLE       1    // Enable DSP debug trace dumping capability (supported since patch 2090)
    #endif

    // AMR trace
    #define C_AMR_TRACE_ID 55

  #endif
  /* d_error_status                */
  /*-------------------------------*/

  #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
    #define D_ERROR_STATUS_TRACE_ENABLE  1    // Enable d_error_status checking capability (supported since patch 2090)

    // masks to apply on d_error_status bit field
    #define DSP_DEBUG_GSM_MASK     0x0000
    #define DSP_DEBUG_GPRS_MASK    0x0f3d
  #endif

#elif (DSP == 35)            // ROM Code GPRS AMR.
  #define CLKMOD1    0x4006  // ...
  #define CLKMOD2    0x4116  // ...65 Mips pll free
  #define CLKSTART   0x29    // ...65 Mips
  #define C_PLL_CONFIG 0x154   // For VTCXO = 13 MHz and max DSP speed = 84.5 Mips
  #define VOC        FR_HR_EFR // FR + HR + EFR (normaly FR_EFR : PBs).
  #define AEC        1         // AEC/NS not supported.
  #if (OP_RIV_AUDIO == 0)
    #define L1_NEW_AEC 1
  #else
  // Available but not yet tuned with Riviera AUDIO    
    #define L1_NEW_AEC 0
  #endif
  #if ((L1_NEW_AEC) && (!AEC))
    // First undef the flag to avoid warnings at compilation time
    #undef AEC
    #define AEC 1
  #endif
  #define MAP        3

  #define FF_L1_TCH_VOCODER_CONTROL 1
  #define L1M_WAIT_DSP_RESTART_AFTER_VOCODER_ENABLE 1

  #define DSP_START  0x7000

  #define INSTALL_ADD   0x7002 // Used to set gprs_install_address pointer

  #define W_A_DSP1   0         // Work Around correcting pb in DSP: SACCH
  #define ULYSSE      0

  #define W_A_DSP_SR_BGD 1    // Work around about the DSP speech reco background task.

  #if ( (CHIPSET != 12) && (CODE_VERSION == NOT_SIMULATION))

    #define W_A_DSP_IDLE3 1     // Work around to report DSP state to the ARM for Deep Sleep

                                // management.

								// DSP_IDLE3 is not supported in simulation

  #else
    #define W_A_DSP_IDLE3 0
  #endif

  // DSP software work-around config
  //  bit0 - Work-around to support CRTG.
  //  bit1 - DMA reset on critical DMA still running cases, refer to REQ01260.
  //  bit2 - Solve Read/Write BULDATA pointers Omega & Nausica issue, refer to BUG00650.
  //  bit3 - Solve IBUFPTRx reset IOTA issue, refer to BUG01911.
  #if    (ANALOG == 1)  // OMEGA / NAUSICA
    #define C_DSP_SW_WORK_AROUND 0x0006

  #elif  (ANALOG == 2)  // IOTA
    #define C_DSP_SW_WORK_AROUND 0x000E

  #elif  (ANALOG == 3)  // SYREN
    #define C_DSP_SW_WORK_AROUND 0x000E

  #endif

  /* DSP debug trace configuration */
  /*-------------------------------*/
  #if (MELODY_E2)
    // In case of the melody E2 the DSP trace must be disable because the
    // melody instrument waves are overlayed with DSP trace buffer

    // DSP debug trace API buufer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 7       // Real size is incremented by 1 for DSP write pointer.

    // DSP debug trace type config
    //             |<-------------- Features -------------->|<---------- Levels ----------->|
    // [15-8:UNUSED|7:TIMER|6:BURST|5:BUFFER|4:BUFFER HEADER|3:UNUSED|2:KERNEL|1:BASIC|0:ISR]
    #define C_DEBUG_TRACE_TYPE  0x0000  // Level = BASIC; Features = Timer + Buffer Header + Burst.

    #if (C_DEBUG_TRACE_TYPE != 0) && ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
      #define DSP_DEBUG_TRACE_ENABLE       1    // Enable DSP debug trace dumping capability
                                                // Currently not supported !
    #endif
  #else
    // DSP debug trace API buufer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 2047    // Real size is incremented by 1 for DSP write pointer.

    // DSP debug trace type config
    //             |<-------------- Features -------------->|<---------- Levels ----------->|
    // [15-8:UNUSED|7:TIMER|6:BURST|5:BUFFER|4:BUFFER HEADER|3:UNUSED|2:KERNEL|1:BASIC|0:ISR]
    #define C_DEBUG_TRACE_TYPE  0x0012  // Level = BASIC; Features = Timer + Buffer Header + Burst.

    #if (C_DEBUG_TRACE_TYPE != 0) && ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
    #define DSP_DEBUG_TRACE_ENABLE       1    // Enable DSP debug trace dumping capability (supported since patch 2090)
    #endif

    // AMR trace
    #define C_AMR_TRACE_ID 55

  #endif
  /* d_error_status                */
  /*-------------------------------*/

  #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
    #define D_ERROR_STATUS_TRACE_ENABLE  1    // Enable d_error_status checking capability (supported since patch 2090)

    // masks to apply on d_error_status bit field for DSP patch 0x2061 or 0x2062
    #define DSP_DEBUG_GSM_MASK     0x08BD
    #define DSP_DEBUG_GPRS_MASK    0x0f3d
  #endif
#elif (DSP == 36)            // ROM Code GPRS AMR.
  #define CLKMOD1    0x4006  // ...
  #define CLKMOD2    0x4116  // ...65 Mips pll free
  #define CLKSTART   0x29    // ...65 Mips
  #define C_PLL_CONFIG 0x154   // For VTCXO = 13 MHz and max DSP speed = 84.5 Mips
  #define VOC        FR_HR_EFR // FR + HR + EFR (normaly FR_EFR : PBs).
  #define AEC        1         // AEC/NS not supported.
  #if (OP_RIV_AUDIO == 0)
    #define L1_NEW_AEC 1
  #else
  // Available but not yet tuned with Riviera AUDIO    
    #define L1_NEW_AEC 0
  #endif
  #if ((L1_NEW_AEC) && (!AEC))
    // First undef the flag to avoid warnings at compilation time
    #undef AEC
    #define AEC 1
  #endif
  #define MAP        3
  #undef  L1_AMR_NSYNC
  #define L1_AMR_NSYNC 1
  #define FF_L1_TCH_VOCODER_CONTROL 1
  #define L1M_WAIT_DSP_RESTART_AFTER_VOCODER_ENABLE 1

  #define DSP_START  0x7000

  #define INSTALL_ADD   0x7002 // Used to set gprs_install_address pointer

  #define W_A_DSP1   0         // Work Around correcting pb in DSP: SACCH
  #define ULYSSE      0

  #define W_A_DSP_SR_BGD 1    // Work around about the DSP speech reco background task.

  #if ( (CHIPSET != 12) && (CODE_VERSION == NOT_SIMULATION))

    #define W_A_DSP_IDLE3 1     // Work around to report DSP state to the ARM for Deep Sleep

                                // management.

								// DSP_IDLE3 is not supported in simulation

  #else
    #define W_A_DSP_IDLE3 0
  #endif

  // DSP software work-around config
  //  bit0 - Work-around to support CRTG.
  //  bit1 - DMA reset on critical DMA still running cases, refer to REQ01260.
  //  bit2 - Solve Read/Write BULDATA pointers Omega & Nausica issue, refer to BUG00650.
  //  bit3 - Solve IBUFPTRx reset IOTA issue, refer to BUG01911.
  #if    (ANALOG == 1)  // OMEGA / NAUSICA
    #define C_DSP_SW_WORK_AROUND 0x0006

  #elif  (ANALOG == 2)  // IOTA
    #define C_DSP_SW_WORK_AROUND 0x000E

  #elif  (ANALOG == 3)  // SYREN
    #define C_DSP_SW_WORK_AROUND 0x000E
  #endif

  // This workaround should be enabled only for H2-sample on full build config
  #if (OP_L1_STANDALONE==1)
    #define RAZ_VULSWITCH_REGAUDIO 0
  #endif

  /* DSP debug trace configuration */
  /*-------------------------------*/
  #if (MELODY_E2)
    // In case of the melody E2 the DSP trace must be disable because the
    // melody instrument waves are overlayed with DSP trace buffer

    // DSP debug trace API buufer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 7       // Real size is incremented by 1 for DSP write pointer.

    // DSP debug trace type config
    //             |<-------------- Features -------------->|<---------- Levels ----------->|
    // [15-8:UNUSED|7:TIMER|6:BURST|5:BUFFER|4:BUFFER HEADER|3:UNUSED|2:KERNEL|1:BASIC|0:ISR]
    #define C_DEBUG_TRACE_TYPE  0x0000  // Level = BASIC; Features = Timer + Buffer Header + Burst.

    #if (C_DEBUG_TRACE_TYPE != 0) && ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
      #define DSP_DEBUG_TRACE_ENABLE       1    // Enable DSP debug trace dumping capability
                                                // Currently not supported !
    #endif
  #else
    // DSP debug trace API buufer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 2047    // Real size is incremented by 1 for DSP write pointer.

    // DSP debug trace type config
    //             |<-------------- Features -------------->|<---------- Levels ----------->|
    // [15-8:UNUSED|7:TIMER|6:BURST|5:BUFFER|4:BUFFER HEADER|3:UNUSED|2:KERNEL|1:BASIC|0:ISR]
    #define C_DEBUG_TRACE_TYPE  0x0012  // Level = BASIC; Features = Buffer Header.

    #if (C_DEBUG_TRACE_TYPE != 0) && ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
    #define DSP_DEBUG_TRACE_ENABLE       1    // Enable DSP debug trace dumping capability (supported since patch 2090)
    #endif

    // AMR trace
    #define C_AMR_TRACE_ID 55

  #endif
  /* d_error_status                */
  /*-------------------------------*/

  #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
    #define D_ERROR_STATUS_TRACE_ENABLE  1    // Enable d_error_status checking capability (supported since patch 2090)

    // masks to apply on d_error_status bit field for DSP patch 0x2061 or 0x2062
    #define DSP_DEBUG_GSM_MASK     0x08BD
    #define DSP_DEBUG_GPRS_MASK    0x0f3d
  #endif
#endif // DSP

/*------------------------------------*/
/* Default value                      */
/*------------------------------------*/
#ifndef W_A_DSP1
  #define W_A_DSP1   0
#endif

#ifndef DATA14_4
  #define DATA14_4   0
#endif

#ifndef W_A_ITFORCE
  #define W_A_ITFORCE   0
#endif

#ifndef W_A_DSP_IDLE3
  #define W_A_DSP_IDLE3 0
#endif

#ifndef L1_NEW_AEC
  #define L1_NEW_AEC 0
#endif

#ifndef DSP_DEBUG_TRACE_ENABLE
  #define DSP_DEBUG_TRACE_ENABLE 0
#endif

#ifndef DEBUG_DEDIC_TCH_BLOCK_STAT
  #define DEBUG_DEDIC_TCH_BLOCK_STAT 0
#endif

#ifndef D_ERROR_STATUS_TRACE_ENABLE
  #define D_ERROR_STATUS_TRACE_ENABLE  0
#endif

#ifndef L1_GTT
  #define L1_GTT 0
  #define TTY_SYNC_MCU 0
  #define TTY_SYNC_MCU_2 0
  #define L1_GTT_FIFO_TEST_ATOMIC 0
  #define NEW_WKA_PATCH          0
  #define OPTIMISED              0
#endif

#ifndef L1_AMR_NSYNC
  #define L1_AMR_NSYNC 0
#endif

#ifndef FF_L1_TCH_VOCODER_CONTROL
  #define FF_L1_TCH_VOCODER_CONTROL 0
  #define L1M_WAIT_DSP_RESTART_AFTER_VOCODER_ENABLE 0 
#endif

/*------------------------------------*/
/* Download                           */
/*------------------------------------*/


/* Possible values for the download status */

#define LEAD_READY      1
#define BLOCK_READY     2
#define PROGRAM_DONE    3
#define PAGE_SELECTION  4


/************************************/
/* Options of compilation...        */
/************************************/

// Possible choice of hardware plateform.
#define GEMINI       1   // GEMINI chip (rom dsp code)
#define POLESTAR     2   // POLESTAR chip (no rom)

// Possible choice for DSP software setup.
#define NO_DWNLD         0
#define PATCH_DWNLD      1
#define DSP_DWNLD        2
#define PATCH_DSP_DWNLD  3

// MAC-S status reporting to Layer 1
#define MACS_STATUS     0   // MAC-S STATUS activated if set to 1


// Possible choice for dll_dcch_downlink interface (with FN or without FN)
#define SEND_FN_TO_L2_IN_DCCH 1 /* 0=without, 1=with FN parameter */

//---------------------------------------------------------------------------------

// Neighbor Cell RXLEV indication
#if ((OP_L1_STANDALONE==1) && (CODE_VERSION == NOT_SIMULATION))
 #define  L1_MPHC_RXLEV_IND_REPORT_SORT 1
#else
 #define  L1_MPHC_RXLEV_IND_REPORT_SORT 0
#endif

#endif /* __L1_CONFG_H__ */

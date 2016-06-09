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

// RLC functions Version possible choices
//------------------------------
#define       POLL_FORCED     0
#define       RLC_SCENARIO    1
#define       MODEM_FLOW      2

// possible choices for UART trace output
//------------------------------
#if (CHIPSET != 15)
  #define       MODEM_UART     0
  #define       IRDA_UART      1
  #if (CHIPSET == 12)
    #define     MODEM2_UART    2
  #endif
#else
  // There is only one UART in Locosto
  #define       MODEM_UART     0
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


  #undef FF_L1_IT_DSP_USF
  #define FF_L1_IT_DSP_USF       0
  #undef FF_L1_IT_DSP_DTX
#if (AMR == 1)
  #define FF_L1_IT_DSP_DTX     1 //it should be 1, sajal- temp made it 0 for build purpose
#else
  #define FF_L1_IT_DSP_DTX     0
#endif

  #define L1_DRP_IQ_SCALING    0

  // Test Scenari...
  #define SCENARIO_FILE          1  // Test Scenario comes from input files.
  #define SCENARIO_MEM           0  // Test Scenario comes from RAM.

  // In Simulation AUDIO_DEBUG Should be 0
  #define AUDIO_DEBUG 0

  // Traces...
  #undef TRACE_TYPE
  #define TRACE_TYPE             5
  #define LOGFILE_TRACE          1  // trace in an output logfile

  #define BURST_PARAM_LOG_ENABLE 0  // Burst Param Log Enable

  #define FLOWCHART              0  // Message sequence/flow chart trace.
  #define NUCLEUS_TRACE          0  // Nucleus error trace
  #define EOTD_TRACE             1  // EOTD log trace
  #define TRACE_FULL_NAME        0  // display full fct names after a PM/COM error

  #define L2_L3_SIMUL            1  // Layer 2 & Layer 3 simulated, main within NU_MAIN.C, trace possible.

  // Control algorithms...
  #define AFC_ALGO               1  // AFC algorithm.
#if (L1_SAIC != 0)
  #define TOA_ALGO               2  // TOA algorithm.
#else
  #define TOA_ALGO               1  // TOA algorithm.
#endif
  #define AGC_ALGO               1  // AGC algorithm.
  #define TA_ALGO                0  // TA (Timing Advance) algorithm.
  #undef VCXO_ALGO
  #define VCXO_ALGO              1  // VCXO algo
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
  #define TTY_SYNC_MCU           0  // TTY WORKAROUND BUG03401
  #define TTY_SYNC_MCU_2         0  //
  #define L1_GTT_FIFO_TEST_ATOMIC 0 //
  #define NEW_WKA_PATCH          0
  #define OPTIMISED              0

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

  #undef L1_DRP
  #define L1_DRP                0  // L1 supporting DRP interface

  #undef DRP_MEM_SIMULATION
  #define DRP_MEM_SIMULATION          0
//---------------------------------------------------------------------------------
// Test with H/W platform.
//---------------------------------------------------------------------------------

  #if (GSM_IDLE_RAM == 1)
    #define GSM_IDLE_RAM_DEBUG 0
  #endif

  #define AFC_BYPASS_MODE        0
 #define PWMEAS_IF_MODE_FORCE  0
// WA for OMAPS00099442 must be disabled in PC simulation
  #undef L1_FF_WA_OMAPS00099442
  #define L1_FF_WA_OMAPS00099442 0

#elif (CODE_VERSION == NOT_SIMULATION)

  #define L1_DRP_IQ_SCALING    1
  // In Target AUDIO_DEBUG could be turned ON to debug any AUDIO ON/OFF issues
  #define AUDIO_DEBUG 0

  #if (GSM_IDLE_RAM == 1)
    #if ((CHIPSET == 12) || (CHIPSET == 10))
      #define GSM_IDLE_RAM_DEBUG 1
    #else
      #define GSM_IDLE_RAM_DEBUG 0
    #endif
  #else
    #define GSM_IDLE_RAM_DEBUG 0
  #endif

//FreeCalypso: L1_VPM commented out, as I suspect it's a LoCosto-ism
//#define L1_VPM  1

  #if (OP_L1_STANDALONE == 1)
    #if (CHIPSET == 15)
      #if ((BOARD == 71) && (FLASH == 0))
        // Not possible in I-SAMPLE only RAM configuration as there will
        // not be enough memory space
        #define BURST_PARAM_LOG_ENABLE  0
      #else
        #define BURST_PARAM_LOG_ENABLE  1
      #endif
    #else
      #define BURST_PARAM_LOG_ENABLE  0
    #endif
  #else
    #define BURST_PARAM_LOG_ENABLE  0
  #endif

  // Work around about Calypso RevA: the bus is floating (Cf PB01435)
  // (corrected with Calypso ReV B and Calypso C035)
  #if (CHIPSET == 7)
    #define W_A_CALYPSO_BUG_01435 1
  #else
    #define W_A_CALYPSO_BUG_01435 0
  #endif

  #if (CHIPSET == 12) // Not needed for CHIPSET =15, as there is no extended page mode in Locosto
    #define W_A_CALYPSO_PLUS_SPR_19599 1
  #else
    #define W_A_CALYPSO_PLUS_SPR_19599 0
  #endif

  // for AMR thresolds definition CQ22226
  #define W_A_AMR_THRESHOLDS 1
  #define W_A_PCTM_RX_AGC_GLOBAL_PARAMS 1 // For support of PCTM

  #if (L1_GTT==1)
    #define TTY_SYNC_MCU 0
    #define TTY_SYNC_MCU_2 0
    #define L1_GTT_FIFO_TEST_ATOMIC 0
    #define NEW_WKA_PATCH          0
    #define OPTIMISED              0
  #else
    #define TTY_SYNC_MCU_2 0
    #define L1_GTT_FIFO_TEST_ATOMIC 0
    #define TTY_SYNC_MCU 0
    #define NEW_WKA_PATCH          0
    #define OPTIMISED              0

  #endif

/*
 * FreeCalypso: these FF_L1_IT_DSP_USF and FF_L1_IT_DSP_DTX features (?)
 * are new with the LoCosto L1 headers, i.e., not present in the Leonardo
 * headers.  I have no idea what they are, and I suspect they may likely
 * be something that won't work on our Calypso platform, so I'm disabling
 * them for now.
 */

  #undef FF_L1_IT_DSP_USF
#if 0 //(L1_GPRS == 1)
  #define FF_L1_IT_DSP_USF       1
#else
  #define FF_L1_IT_DSP_USF       0
#endif
  #undef FF_L1_IT_DSP_DTX
#if 0 //(AMR == 1)
  #define FF_L1_IT_DSP_DTX     1
#else
  #define FF_L1_IT_DSP_DTX     0
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
#if (L1_SAIC != 0)
  #define TOA_ALGO               2  // TOA algorithm.
#else
  #define TOA_ALGO               1  // TOA algorithm.
#endif
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

  #if ((RF_FAM == 60) || (RF_FAM == 61))
    #define L1_DRP                 1  // L1 supporting DRP interface
  #else
    #define L1_DRP                 0  // L1 supporting DRP interface
  #endif
  #define DRP_MEM_SIMULATION   0 // DRP memory simulation OFF by default

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
#define PWMEAS_IF_MODE_FORCE  1
// WA for OMAPS00099442 (OMAPS0010023 (N12.x), OMAPS000010022 (N5.x))
  // The problem is: When NW is lost due to reception gap or cell border range,
  // the MS will try to re-synchronize on the cell with the TPU timing aligned
  // with the timing of the cell. So the FB will start within the 92 bits of the TPU window and
  // will be missed. This issue is due to a limitation of the legacy FB demodulation algorithm
  // WA is to re-initialize the TPU with an arbitrary timing value
  #undef L1_FF_WA_OMAPS00099442
  #define L1_FF_WA_OMAPS00099442 1

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
  #if (CODE_VERSION == SIMULATION)
    #define L1_VOICE_MEMO       1
  #endif
  #if ((OP_L1_STANDALONE == 1) || (!GSMLITE))
    #define MELODY_E1        1  // Enable melody format E1 feature

    #if(L1_VOICE_MEMO == 1)
      #define VOICE_MEMO       1  // Enable voice memorization feature
    #else
      #define VOICE_MEMO       0
    #endif
    #define FIR              1  // Enable FIR feature
    #if (DSP >= 33)
      #define AUDIO_MODE       1  // Enable Audio mode feature
    #else
      #define AUDIO_MODE        0  // Disable Audio mode feature
    #endif
  #else
    #define MELODY_E1        0  // Disable melody format E1 feature
    #if(L1_VOICE_MEMO == 1)
      #define VOICE_MEMO       1  // Enable voice memorization feature
    #else
      #define VOICE_MEMO       0
    #endif
    #if (MELODY_E2)
      #define FIR            1  // Enable FIR feature
    #else
      #define FIR            0  // Disable FIR feature
    #endif
    #define AUDIO_MODE       0  // Disable Audio mode feature
  #endif


#else
  #define KEYBEEP           0  // Enable keybeep feature
  #define TONE              0  // Enable tone feature
  #define MELODY_E1         0  // Enable melody format E1 feature
  #define VOICE_MEMO        0 // Enable voice memorization feature
  #define FIR               0  // Enable FIR feature
  #define AUDIO_MODE        0  // Enable Audio mode feature
#endif

//FreeCalypso: LoCosto-ism below disabled
//#define L1_MIDI_BUFFER 1

/*
 * L1_CPORT appears in the Leonardo L1 headers, and is enabled only for
 * CHIPSET 12.  The LoCosto version doesn't have it at all.
 */
#define	L1_CPORT	0

#define L1_AUDIO_BACKGROUND_TASK (SPEECH_RECO | MELODY_E2) // audio background task is used by speech reco and melody_e2
#if (OP_RIV_AUDIO == 1)
  #define L1_AUDIO_DRIVER (L1_VOICE_MEMO_AMR | L1_EXT_AUDIO_MGT | L1_MP3) // Riviera audio driver (only Voice Memo AMR is available)
#endif


// Vocoder selections
//-------------------

#define FR        1            // Full Rate
#define FR_HR     2            // Full Rate + Half Rate
#define FR_EFR    3            // Full Rate + Enhanced Full Rate
#define FR_HR_EFR 4            // Full Rate + Half Rate + Enhanced Full Rate

// Standard (frequency plan) selections
//-------------------------------------
#if(L1_FF_MULTIBAND == 0) // std id is not used if multiband feature is enabled

#define GSM             1            // GSM900.
#define GSM_E           2            // GSM900 Extended.
#define PCS1900         3            // PCS1900.
#define DCS1800         4            // DCS1800.
#define DUAL            5            // Dual Band (GSM900 + DCS 1800 bands)
#define DUALEXT         6            // Dual Band (E-GSM900 + DCS 1800 bands)
#define GSM850          7            // GSM850 Band
#define DUAL_US         8            // PCS1900 + GSM850

#endif // L1_FF_MULTIBAND

/*------------------------------------*/
/* Power Management                   */
/*------------------------------------*/
#define PWR_MNGT  1            // POWER management active if l1_config.pwr_mngt=1

/*------------------------------------*/
/*    BT Audio                        */ 
/*------------------------------------*/
#if ((L1_MP3 == 1) || (L1_AAC == 1))
#if (OP_L1_STANDALONE == 0)
#if((PSP_STANDALONE == 1) || (DRP_FW_BUILD == 1))
#define L1_BT_AUDIO 0
#else
#define L1_BT_AUDIO 1
#endif
#else
#define L1_BT_AUDIO 0
#endif
#endif
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

    // DSP debug trace API buffer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 7       // Real size is incremented by 1 for DSP write pointer.
  #else
    // DSP debug trace API buffer config
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
  #define L1_NEW_AEC 1

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

  #if (CODE_VERSION == NOT_SIMULATION)
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

    // DSP debug trace API buffer config
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
    // DSP debug trace API buffer config
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
    #define DSP_DEBUG_GSM_MASK     0x08BD // L1_MCU-SPR-15852
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
  #define L1_NEW_AEC 1

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

  #if (CODE_VERSION == NOT_SIMULATION)
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

    // DSP debug trace API buffer config
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
    // DSP debug trace API buffer config
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
    #define DSP_DEBUG_GSM_MASK     0x08BD // L1_MCU-SPR-15852
    #define DSP_DEBUG_GPRS_MASK    0x0f3d
  #endif

#elif (DSP == 35)            // ROM Code GPRS AMR.
  #define CLKMOD1    0x4006  // ...
  #define CLKMOD2    0x4116  // ...65 Mips pll free
  #define CLKSTART   0x29    // ...65 Mips
  #define C_PLL_CONFIG 0x154   // For VTCXO = 13 MHz and max DSP speed = 84.5 Mips
  #define VOC        FR_HR_EFR // FR + HR + EFR (normaly FR_EFR : PBs).
  #define AEC        1         // AEC/NS not supported.
  #define L1_NEW_AEC 1

  #if ((L1_NEW_AEC) && (!AEC))
    // First undef the flag to avoid warnings at compilation time
    #undef AEC
    #define AEC 1
  #endif
  #define MAP        3

  #define FF_L1_TCH_VOCODER_CONTROL 1
  #define W_A_WAIT_DSP_RESTART_AFTER_VOCODER_ENABLE 1

  #define DSP_START  0x7000

  #define INSTALL_ADD   0x7002 // Used to set gprs_install_address pointer

  #define W_A_DSP1   0         // Work Around correcting pb in DSP: SACCH
  #define ULYSSE      0

  #define W_A_DSP_SR_BGD 1    // Work around about the DSP speech reco background task.

  #if (CODE_VERSION == NOT_SIMULATION)
    #if (CHIPSET != 12)
        #define W_A_DSP_IDLE3 1     // Work around to report DSP state to the ARM for Deep Sleep
                                    // management.
                                    // DSP_IDLE3 is not supported in simulation
    #else
      #define W_A_DSP_IDLE3 0     // Work around to report DSP state to the ARM for Deep Sleep
                                  // management.
                                  // DSP_IDLE3 is not supported in simulation
    #endif // CHIPSET 12
  #else
      #define W_A_DSP_IDLE3 0
  #endif

  #define W_A_DSP_PR20037 1

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

    // DSP debug trace API buffer config
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
    // DSP debug trace API buffer config
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
    #define DSP_DEBUG_GSM_MASK     0x08BD // L1_MCU-SPR-15852
    #define DSP_DEBUG_GPRS_MASK    0x0f3d
  #endif
#elif (DSP >= 36)				// ROM Code GPRS AMR.

  #if ((L1_PCM_EXTRACTION) && (SPEECH_RECO))
    #error "PCM extraction and Speech recognition not supported simultaneously"
  #endif

  #define CLKMOD1    0x4006  // ...
  #define CLKMOD2    0x4116  // ...65 Mips pll free
  #define CLKSTART   0x29    // ...65 Mips
  #define C_PLL_CONFIG 0x154   // For VTCXO = 13 MHz and max DSP speed = 84.5 Mips
  #define VOC        FR_HR_EFR // FR + HR + EFR (normaly FR_EFR : PBs).

#if 0
  /* what we got with LoCosto L1 headers */
  #define AEC        0         // AEC/NS not supported.
  #define L1_NEW_AEC 0
#else
  /* what we are used to from the Leonardo version */
  #define AEC        1         // AEC/NS not supported.
  #if (OP_RIV_AUDIO == 0)
    #define L1_NEW_AEC 1
  #else
  // Available but not yet tuned with Riviera AUDIO    
    #define L1_NEW_AEC 0
  #endif
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
  #define W_A_WAIT_DSP_RESTART_AFTER_VOCODER_ENABLE 1

  #define DSP_START  0x7000

  #define INSTALL_ADD   0x7002 // Used to set gprs_install_address pointer

  #define W_A_DSP1   0         // Work Around correcting pb in DSP: SACCH
  #define ULYSSE      0

  #define W_A_DSP_SR_BGD 1    // Work around about the DSP speech reco background task.

  #if (CODE_VERSION == NOT_SIMULATION)
    #if ((CHIPSET != 12) && (CHIPSET != 15))
      #define W_A_DSP_IDLE3 1     // Work around to report DSP state to the ARM for Deep Sleep
                                  // management.
                                  // DSP_IDLE3 is not supported in simulation
    #else  // CHIPSET 12
      #define W_A_DSP_IDLE3 0     // Work around to report DSP state to the ARM for Deep Sleep
                                  // management.
                                  // DSP_IDLE3 is not supported in simulation
    #endif // CHIPSET 12
  #else // CODE_VERSION
    #define W_A_DSP_IDLE3 0
  #endif

  #define W_A_DSP_PR20037 1

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

  #elif  (ANALOG == 11)  // TRITON
    #define C_DSP_SW_WORK_AROUND 0x000E

  #endif

  /* DSP debug trace configuration */
  /*-------------------------------*/
   // Note:
  // In case of melody E2, MP3, AAC or Dyn Dwnld ACTIVITY the DSP trace is automatically disabled
  // because the melody instrument waves are overlayed with DSP trace buffer (supported since patch 7c20)

    // DSP debug trace API buffer config
    #define C_DEBUG_BUFFER_ADD  0x17ff  // Address of DSP write pointer... data are just after.
    #define C_DEBUG_BUFFER_SIZE 2047       // Real size is incremented by 1 for DSP write pointer.

    // DSP debug trace type config
    //             |<-------------- Features -------------->|<---------- Levels ----------->|
    // [15-8:UNUSED|7:TIMER|6:BURST|5:BUFFER|4:BUFFER HEADER|3:UNUSED|2:KERNEL|1:BASIC|0:ISR]

    #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)// C_DEBUG_TRACE_TYPE  0x0012 changed from 0x0054 for DSP load reduce
      #define C_DEBUG_TRACE_TYPE  0x0012  // Level = KERNEL; Features = Timer, Burst, Buffer Header.
    #else
      #define C_DEBUG_TRACE_TYPE  0x0000  // Level = KERNEL; Features = Timer, Burst, Buffer Header.
    #endif


    #if (C_DEBUG_TRACE_TYPE != 0) && ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
      #define DSP_DEBUG_TRACE_ENABLE       1    // Enable DSP debug trace dumping capability
                                                // Currently not supported !
    #endif

    // AMR trace
    #define C_AMR_TRACE_ID 55


  /* d_error_status                */
  /*-------------------------------*/

  #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
    #define D_ERROR_STATUS_TRACE_ENABLE  1    // Enable d_error_status checking capability (supported since patch 2090)

    // masks to apply on d_error_status bit field for DSP patch 0x2061 or 0x2062
    #define DSP_DEBUG_GSM_MASK     0x08BD // L1_MCU-SPR-15852
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
  #define W_A_WAIT_DSP_RESTART_AFTER_VOCODER_ENABLE 0
  #define W_A_DSP_PR20037 0
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

/*
 * Possible choice for dll_dcch_downlink interface (with FN or without FN)
 * 0=without, 1=with FN parameter
 *
 * FreeCalypso note: the Leonardo version had this setting set to 1, i.e.,
 * 3 arguments to dll_dcch_downlink(). We don't have any source or even
 * header files for the Leonardo version of DL, but disassembly shows
 * that dll_dcch_downlink() does expect the FN parameter. The source for
 * DL from LoCosto also has a SEND_FN_TO_L2_IN_DCCH configurable setting,
 * and it is set to 1 in the dl.h local header. But here is the kicker:
 * the LoCosto version of this l1_confg.h header has the setting set to 0!
 *
 * I couldn't believe my eyes, so I disassembled the binary objects present
 * in the copy of the LoCosto source from scottn.us: yes, indeed that
 * code version contains an outright bug in that L1 does not pass the
 * 3rd argument (in ARM register r2), but DL expects it to be there.
 * (Thus DL is getting whatever "garbage" happens to be in r2 as the FN
 * parameter. I did not take the time to investigate what the downstream
 * effects are.)
 *
 * For FreeCalypso I'm setting SEND_FN_TO_L2_IN_DCCH to 1, both here
 * in L1 and in DL, where it was already set.
 */
#define SEND_FN_TO_L2_IN_DCCH 1

/*
 * FreeCalypso change: I'm disabling L1_CHECK_COMPATIBLE (a new "feature"
 * added with LoCosto version of L1, not present in the Leonardo version)
 * because l1_async.c fails to compile with it enabled.  Examination of
 * the code reveals that this "compatibility check" involves things
 * which we won't be enabling any time soon, if ever.
 */
#define L1_CHECK_COMPATIBLE 0    //Check L1A message compatiblity

//---------------------------------------------------------------------------------

#endif /* __L1_CONFG_H__ */

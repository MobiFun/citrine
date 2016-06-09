/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_TIME.H
 *
 *        Filename l1_time.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

// *********************************************************************
// *                                                                   *
// *        This file contains only RF independant defines.            *
// *                                                                   *
// *********************************************************************
// Remarks:
// --------
// PRG_TX is RF dependant, it is therefore provided within
// "l1_rf#.h".
// **************************************************************************
//
// measurements
// ------------
//
//                       |        +-----+
//                       |        | PW  |
//    -------------------|--------+     +--------------
//                  clk=offset    |     |
//                 (frame int.)  >|-----|<-PW_BURST_DURATION
//                       |        |     |
//    |      SYNTH_SETUP_TIME     |     |
//    |<--------------------------|<    |
//    |                  |        |
//                       |        |
//                      >|--------|<-PROVISION_TIME
//
//
// Normal Burst reception
// ----------------------
//
//                       |        +---------+
//                       |        |  RX WIN |
//  ---------------------|--------+         +----------
//                  clk=offset    |         |
//                 (frame int.)  >|---------|<-NB_BURST_DURATION_DL
//                       |        |         |
//    |      SYNTH_SETUP_TIME     |         |
//    |<--------------------------|<        |
//    |                  |        |
//                       |        |
//                      >|--------|<-PROVISION_TIME
//
//
// Normal Burst transmission
// -------------------------
//
//                            .
//                            +---------+
//                            |  TX WIN |
//  --------------------------+         +----------
//                            .         |
//                       clk=offset     |
//                            .         |
//                            .         |<--STOP_TX_**
//   |    SYNTH_SETUP_TIME    .
//   |<---------------------->.<--START_TX
//   |                        .
//
//
//
// Frequency Burst search in Dedicated TCH
// ---------------------------------------
//
//                       .        +-----------(...)-------------+
//                       .        |      FB search in TCH       |
//    -------------------.--------+                             +--------------
//                       .        |                             |
//           (FB26_ANCHORING_TIME)|                             |
//                       .        |                             |
//            SYNTH_SETUP_TIME    |                             |
//    |<------------------------->|                             |<-STOP_RX_FB26
//                       .        |
//                       .        |<-START_RX_FB26
//                       .        |
//                       .        |
//                      >.--------|<-PROVISION_TIME
//
//
// **************************************************************************


#define D_NSUBB_IDLE         296L                                                    // Nb of 48 samples window for FBNEW task.
#if (CODE_VERSION==SIMULATION)
  #define D_NSUBB_DEDIC        31L                                                     // Nb of 48 samples window for FB26 task.
#else
  #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 32) || (DSP == 36)
    #define D_NSUBB_DEDIC        30L                                                     // Nb of 48 samples window for FB26 task.
  #else
    #define D_NSUBB_DEDIC        31L                                                     // Nb of 48 samples window for FB26 task.
  #endif
#endif


#define IMM                  ( 5000L )                                               // Immediate command for TPU.
#define TN_WIDTH             ( 625L )
#define BP_DURATION          TN_WIDTH
#define TAIL_WIDTH           ( 3L * 4L )                                             // = 12
#define EXTENDED_TAIL_WIDTH  ( 8L * 4L )
#define TPU_CLOCK_RANGE      ( 5000L )
#define SWITCH_TIME          ( TPU_CLOCK_RANGE - EPSILON_SYNC )                      // = 4990, time for offset change.

#define PROVISION_TIME       ( 66L )
#define EPSILON_SYNC         ( 10L )                                                 // synchro change: max TOA shift=8qbits, 2qbits TPU scenario exec.
#define EPSILON_OFFS         (  2L )                                                 // offset change: 2qbits for TPU scenario exec.
#define EPSILON_MEAS         ( 20L )                                                 // margin kept between RX and PW meas or between PW meas
#define SERV_OFFS_REST_LOAD  (  1L )                                                 // 1qbit TPU scen exec. for serv. cell offset restore
#define TPU_SLEEP_LOAD       (  2L )                                                 // 2qbit TPU scen exec. for TPU sleep
#if (CODE_VERSION==SIMULATION)
  #define DL_ABB_DELAY         ( 32L )                                             // RX ABB filter delay
#else
  #define DL_ABB_DELAY         ( 32L + 4L)                                         // RX ABB filter delay
#endif

// DMA threshold used for sample acquisition by the DSP
#if (CODE_VERSION==SIMULATION)
  #define RX_DMA_THRES       (  1L )
#else
  #if (CHIPSET == 4) || (CHIPSET == 7)  || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11)
    #define RX_DMA_THRES       (  2L )
  #else
    #define RX_DMA_THRES       (  1L )
  #endif
#endif

// BDLENA durations are calculated for a DMA threshold of 1
// For a DMA threshold > 1 additional I/Q samples have to be acquired
// An increase of BDLENA length by 2qbit is sufficient to acquire one additional I/Q sample
// (ABB always outputs pairs of I/Q samples)
#define RX_DMA_DELAY  (RX_DMA_THRES - 1) * 2

#if (CODE_VERSION==SIMULATION)
  #define TULSET_DURATION    ( 16L )                                                 // Uplink power on setup time
  #define BULRUDEL_DURATION  ( 2L )
  #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
     // 16 qbits are added because the Calibration time is reduced of 4 GSM bit
     // due to a slow APC ramp of OMEGA (Cf. START_TX_NB)
     #define UL_VEGA_DELAY      ( TULSET_DURATION + BULRUDEL_DURATION +16L )         // = 18qbits, TX Vega delay
   #endif
#endif

#define SB_MARGIN            ( 23L * 4L )                                            // = 92
#define NB_MARGIN            (  3L * 4L )                                            // = 12
#define TA_MAX               ( 63L * 4L )                                            // = 252

#define SB_BURST_DURATION    ( TAIL_WIDTH + ( 142L * 4L) )                           // = 580, required for Demodulation
#define NB_BURST_DURATION_DL ( TAIL_WIDTH + ( 142L * 4L) )                           // = 580, required for Demodulation
#define PW_BURST_DURATION    ( 64L * 4L )                                            // = 256
#define RA_BURST_DURATION    ( EXTENDED_TAIL_WIDTH + TAIL_WIDTH + ( 77L * 4L ) )     // = 352 = 88*4
#define NB_BURST_DURATION_UL ( 2*TAIL_WIDTH + ( 142L * 4L) )                         // = 592 = 148 * 4

// PRG_TX has become a variable and will be substracted directly in the code
#define TIME_OFFSET_TX       ( PROVISION_TIME + (3L * TN_WIDTH))                  // = 1902, Offset difference for TX with TA=0.

//================================
// Definitions used by TPU drivers
//================================

// BENA durations...
//------------------
#define SB_ACQUIS_DURATION   ( SB_MARGIN + SB_BURST_DURATION + SB_MARGIN + DL_ABB_DELAY + RX_DMA_DELAY )    // = 796 + DMA delay
#define NB_ACQUIS_DURATION   ( NB_MARGIN + NB_BURST_DURATION_DL + NB_MARGIN + DL_ABB_DELAY + RX_DMA_DELAY ) // = 636 + DMA delay
#define PW_ACQUIS_DURATION   ( PW_BURST_DURATION + DL_ABB_DELAY + RX_DMA_DELAY )                            // = 288 + DMA delay
#define FB_ACQUIS_DURATION   ( ( D_NSUBB_IDLE  * 48L * 4L ) + ( 48L * 4L ) + DL_ABB_DELAY + RX_DMA_DELAY )  // = 57056 + DMA delay
#define FB26_ACQUIS_DURATION ( ( D_NSUBB_DEDIC * 48L * 4L ) + DL_ABB_DELAY + RX_DMA_DELAY)                 // = 5984 + DMA delay

#define START_RX_FB          ( PROVISION_TIME )                                                // = 66
#define START_RX_SB          ( PROVISION_TIME )                                                // = 66
#define START_RX_SNB         ( PROVISION_TIME )                                                // = 66
#define START_RX_PW_1        ( PROVISION_TIME )                                                // = 66
#define START_RX_FB26        ( PROVISION_TIME )                                                // = 66

#define START_TX_NB          ( 0L )
#define START_TX_RA          ( 0L )

#define STOP_RX_FB           ( (PROVISION_TIME + FB_ACQUIS_DURATION)   % TPU_CLOCK_RANGE )       // = 2122
#define STOP_RX_SB           ( (START_RX_SB    + SB_ACQUIS_DURATION)   % TPU_CLOCK_RANGE )       // = 862
#define STOP_RX_SNB          ( (START_RX_SNB   + NB_ACQUIS_DURATION)   % TPU_CLOCK_RANGE )       // = 702
#define STOP_RX_PW_1         ( (START_RX_PW_1  + PW_ACQUIS_DURATION)   % TPU_CLOCK_RANGE )       // = 354
#define STOP_RX_FB26         ( (START_RX_FB26  + FB26_ACQUIS_DURATION) % TPU_CLOCK_RANGE )     // = 4314


//================================
// Definitions used for GPRS
//================================

#if L1_GPRS
  #ifdef L1P_DRIVE_C

    // Window positions for RX normal burst reception durations
    const UWORD16 RX_DOWN_TABLE[8] =
    {
      PROVISION_TIME + NB_ACQUIS_DURATION,             //special case: only 1 RX, 151 IQ samples
      PROVISION_TIME + 2*BP_DURATION + DL_ABB_DELAY,  // 2 * 156.25 samples
      PROVISION_TIME + 3*BP_DURATION + DL_ABB_DELAY,  // 3 * 156.25 samples
      PROVISION_TIME + 4*BP_DURATION + DL_ABB_DELAY,  // 4 * 156.25 samples
      PROVISION_TIME + 5*BP_DURATION + DL_ABB_DELAY,  // 5 * 156.25 samples
      PROVISION_TIME + 6*BP_DURATION + DL_ABB_DELAY,  // 6 * 156.25 samples
      PROVISION_TIME + 7*BP_DURATION + DL_ABB_DELAY,  // 7 * 156.25 samples
      PROVISION_TIME + 8*BP_DURATION + DL_ABB_DELAY   // 8 * 156.25 samples
    };

    // Window positions for TX normal burst and PRACH transmission
    const UWORD16 TX_TABLE[8] =
    {
      0,
        BP_DURATION,
      2*BP_DURATION,
      3*BP_DURATION,
      4*BP_DURATION,
      5*BP_DURATION,
      6*BP_DURATION,
      7*BP_DURATION
    };

  #else

    extern UWORD16 RX_DOWN_TABLE[8];
    extern UWORD16 TX_TABLE[8];

  #endif
#endif

//===============================================
// New Definitions for new WIN-ID implementation
//===============================================

#define  BP_SPLIT_PW2                  5
#define  BP_SPLIT                     32
#define  FRAME_SPLIT          8*BP_SPLIT

// Load for TPU activity according to frame split
#define  PWR_LOAD    1 + PW_ACQUIS_DURATION / (BP_DURATION/BP_SPLIT)
#define  RX_LOAD     1 + NB_ACQUIS_DURATION / (BP_DURATION/BP_SPLIT)

#if L1_GPRS
  #ifdef L1P_DRIVE_C

    // RX split load in case of multislot
    const UWORD16 RX_SPLIT_TABLE[8] =
    {
      1 + (NB_ACQUIS_DURATION           ) / (BP_DURATION/BP_SPLIT),
      1 + (2*BP_DURATION + DL_ABB_DELAY) / (BP_DURATION/BP_SPLIT),
      1 + (3*BP_DURATION + DL_ABB_DELAY) / (BP_DURATION/BP_SPLIT),
      1 + (4*BP_DURATION + DL_ABB_DELAY) / (BP_DURATION/BP_SPLIT),
      1 + (5*BP_DURATION + DL_ABB_DELAY) / (BP_DURATION/BP_SPLIT),
      1 + (6*BP_DURATION + DL_ABB_DELAY) / (BP_DURATION/BP_SPLIT),
      1 + (7*BP_DURATION + DL_ABB_DELAY) / (BP_DURATION/BP_SPLIT),
      1 + (8*BP_DURATION + DL_ABB_DELAY) / (BP_DURATION/BP_SPLIT)
    };

  #else

    extern UWORD16 RX_SPLIT_TABLE[8];

  #endif
#endif


/****************** Revision Controle System Header ***********************
 *                      GSM Layer 1 software
 *              Copyright (c) Texas Instruments 1998
 *
 *        Filename tpudrv61.h
 *        Version  1.0
 *        Date     June 1st, 2005
 *
 ****************** Revision Controle System Header ***********************/
//FLEXIBLE ADC...
#undef VARIABLE_ADC_ON_TX
//VARIABLE_ADC_ON_TX- represents the number of symbols before TRF_T11 (the time where the PA is disabled) 
//the ADC conversion should be performed. VARIABLE_ADC_ON_TX- has a valid range from 14 to 155 symbols. 
//Hence, to use the fix, VARIABLE_ADC_ON_TX- must be defined to a value between 14 and 155, compile time. 
//This will cause the ADC conversion offset (TRF_T9) to be calculated by the following equation: 
//TRF_T9 = TRF_T11-VARIABLE_ADC_ON_TX*4
//The customer is expected to use a value of 76. If this compile flag is not set, the ADC conversion offset (TRF_T9)
// is set to a default value of 14 symbols before TXEN is disabled (i.e. the default timing is 40 qbit before the 
// TX down time anchor and 20 qb before the last tail bit).
#define VARIABLE_ADC_ON_TX     76

#if(L1_RF_KBD_FIX == 1)
// Keyboard disable config:
#define L1_KBD_DIS_RX_NB 1
#define L1_KBD_DIS_RX_SB 1
#define L1_KBD_DIS_RX_FB 1
#define L1_KBD_DIS_RX_FB26 1
#define L1_KBD_DIS_RX_MS 1
#define L1_KBD_DIS_TX_NB 1
#define L1_KBD_DIS_TX_RA 1

#define KBD_DISABLED 1

#endif/*(L1_RF_KBD_FIX == 1)*/

//Fixed declaration of script numbers
#define DRP_REG_ON     (0x0000)
#define DRP_TX_ON      (0x0001)
#define DRP_RX_ON      (0x0002)
#define DRP_TEMP_CONV  (0x0003)
#define DRP_ROC        (0x0004)
#define DRP_REG_OFF    (0x0007)
#define DRP_AFC        (0x000D)
#define DRP_IDLE       (0x000F)

#define SCRIPT_EN      (0x3080)

#define START_SCRIPT(script_nb) (SCRIPT_EN  | script_nb)

// IF settings
#define IF_100KHZ_DRP 0
#define IF_120KHZ_DRP 1

// Gain Compensation Enable/Disable
#define GAIN_COMP_DISABLE  (0x0)  //Default
#define GAIN_COMP_ENABLE   (0x1)

// AFE Gains Definition
#define AFE_LOW_GAIN      (0x0)       // 11 dB
#define AFE_HIGH_GAIN     (0x1)       // 38 dB

// ABE Gains Definition
#define ABE_0_DB          (0x0)
#define ABE_2_DB          (0x1)
#define ABE_5_DB          (0x2)
#define ABE_8_DB          (0x3)
#define ABE_11_DB         (0x4)
#define ABE_14_DB         (0x5)
#define ABE_17_DB         (0x6)
#define ABE_20_DB         (0x7)
#define ABE_23_DB         (0x8)

// Switched Cap Filter Corner Freq Defition
#define SCF_270KHZ 0
#define SCF_400KHZ 1

// Retiming definition
#define RETIM_DISABLE      (0x0000)
#define RETIM_TX_ONLY      (0x0001)
#define RETIM_RX_ONLY      (0x0002)
#define RETIM_FULL         (0x0003)

// IF settings for DSP
#define IF_100KHZ_DSP   2
#define IF_120KHZ_DSP 1

// DCO algo settings
#define DCO_IF_100KHZ      1
#define DCO_IF_0KHZ        2
#define DCO_IF_0KHZ_100KHZ 3
#define DCO_NONE           0


//--- Configuration values
//- Select the RF PG (x10), i.e. 10 for 1.0, 11 for 1.1 or 20 for 2.0
 // This is also used in l1_rf14h to select the SWAP_IQ


 //- Bit definitions for TST register programings, etc
#define BIT_0       0x000001
#define BIT_1       0x000002
#define BIT_2       0x000004
#define BIT_3       0x000008
#define BIT_4       0x000010
#define BIT_5       0x000020
#define BIT_6       0x000040
#define BIT_7       0x000080
#define BIT_8       0x000100
#define BIT_9       0x000200
#define BIT_10      0x000400
#define BIT_11      0x000800
#define BIT_12      0x001000
#define BIT_13      0x002000
#define BIT_14      0x004000
#define BIT_15      0x008000
#define BIT_16      0x010000
#define BIT_17      0x020000
#define BIT_18      0x040000
#define BIT_19      0x080000
#define BIT_20      0x100000
#define BIT_21      0x200000
#define BIT_22      0x400000
#define BIT_23      0x800000


//- Base REGISTER definitions -

//- RF signals connected to TSPACT -
#define RX_START     BIT_0     // RX_START of DRP2
#define TX_START     BIT_1     // TX_START of DRP2 modulator
#define START_APC    BIT_2     // Start of the APC module
#define LDO_EN       BIT_3     // Activation of the internal LDO inside the APC bloc
#if(L1_RF_KBD_FIX == 1)
#define KBD_DIS_TSPACT BIT_4     // Disable keyboard
#endif
#define APC_EN       BIT_5     // Enable of the APC module
#define START_ADC    BIT_0     // Activation of the Triton ADC
#define B3           BIT_3     // Control of the RFMD TX module
#define B1           BIT_5     // Control of the RFMD TX module
#define TX_EN        BIT_6     // Control of the RFMD TX module
#define B2           BIT_7     // Control of the RFMD TX module

#define TXM_SLEEP    (0)	// To avoid leakage during Deep-Seep

// DRP write register
#define OCP_DATA_MSB             0x05
#define OCP_DATA_LSB              0x04
#define OCP_ADDRESS_MSB       0x0B
#define OCP_ADDRESS_LSB        0x0A
#define OCP_ADDRESS_START   0x01

// TSPACT
#define REG_SPI_ACT_U  0x07
#define REG_SPI_ACT_L   0x06

// 3-band config
// RX_UP/DOWN and TX_UP/DOWN
#define RU_900     ( B1 | B3 )
#define RD_900     ( B1 | B3 )
#define TU_900     ( TX_EN | B1 | B3 )
#define TD_900     ( B1 | B3 )

#define RU_850     ( B3 )
#define RD_850     ( B3 )
#define TU_850     ( TX_EN | B3 )
#define TD_850     ( B3 )

#define RU_1800    ( B2 | B3 )
#define RD_1800    ( B2 | B3 )
#define TU_1800    ( TX_EN | B1 | B2 | B3 )
#define TD_1800    ( B2 | B3 )


#define RU_1900    ( B1 | B2 | B3 )
#define RD_1900    ( B1 | B2 | B3 )
#define TU_1900    ( TX_EN | B1 | B2 | B3 )
#define TD_1900    ( B1 | B2 | B3 )




//--- TIMINGS ----------------------------------------------------------

/*------------------------------------------*/
/*        Download delay values             */
/*------------------------------------------*/
// 1 qbit = 12/13 usec (~0.9230769), i.e. 200 usec is ~ 217 qbit (200 * 13 / 12)

#define T TPU_CLOCK_RANGE


// - TPU instruction into TSP timings ---
// 1 tpu instruction = 1 qbit
#define DLT_1     1      // 1 tpu instruction = 1 qbit
#define DLT_2     2      // 2 tpu instruction = 2 qbit
#define DLT_3     3      // 3 tpu instruction = 3 qbit
#define DLT_4     4      // 4 tpu instruction = 4 qbit

// - TPU command execution + serialization length ---
#define DLT_4B    5                   // 5*move


// - INIT (delta or DLT) timings ---
#define DLT_I1  5           // Time required to set EN high before RF_SER_OFF -> RF_SER_ON
#define DLT_I2  200         // Regulators turn on time
#define DLT_I3  5           // Time required to set RF_SER_ON
#define DLT_I4  110         // Regulator Turn-ON time


// - tdt & rdt ---
// MAX GSM (not GPRS) rdt and tdt values are...
//#define rdt       380 // MAX GSM rx delta timing
//#define tdt       400 // MAX GSM tx delta timing
// but current rdt and tdt values are...
#define rdt         0                   // rx delta timing
#define tdt         0                   // tx delta timing

// - RX timings ---
// - RX down:
// Flexible TPU Timings ....

/************************************/
/* Timing for TPU prog                            */
/************************************/
#if ( L1_TPU_DEV == 0)
   #define APC_RAMP_UP_TIME     20  // maximum time for ramp up
   #define APC_RAMP_DELAY            6    // minimum ramp up delay APCDEL
   #define APC_RAMP_DOWN_TIME   20  // maximum ramp down time
#endif


#if ( L1_TPU_DEV == 1)
  extern WORD16 rf_tx_tpu_timings[];
  extern WORD16 rf_rx_tpu_timings[];

  //define for TPU Dev Mode via the arrays so that they can be tweaked later for Rx & Tx



  #define TRF_T1 rf_tx_tpu_timings[0]
  #define TRF_T2 rf_tx_tpu_timings[1]
  #define TRF_T3 rf_tx_tpu_timings[2]
  #define TRF_T4 rf_tx_tpu_timings[3]
  #define TRF_T5 rf_tx_tpu_timings[4]
  #define TRF_T6  rf_tx_tpu_timings[5]
  #define TRF_T7  rf_tx_tpu_timings[6]
  #define TRF_T8  rf_tx_tpu_timings[7]
  #define TRF_T9  rf_tx_tpu_timings[8]
  #define TRF_T10 rf_tx_tpu_timings[9]
  #define TRF_T11  rf_tx_tpu_timings[10]
  #define TRF_T12  rf_tx_tpu_timings[11]
  #define TRF_T13 rf_tx_tpu_timings[12]
  #define TRF_T14 rf_tx_tpu_timings[13]
  #define TRF_T15 rf_tx_tpu_timings[14]
  #define TRF_T16  rf_tx_tpu_timings[15]
  #define TRF_T17 rf_tx_tpu_timings[16]
  #define TRF_T18  rf_tx_tpu_timings[17]
  #define TRF_T19  rf_tx_tpu_timings[18]
  #define TRF_T20 rf_tx_tpu_timings[19]
  #define TRF_T21 rf_tx_tpu_timings[20]
  #define TRF_T22 rf_tx_tpu_timings[21]
  #define TRF_T23 rf_tx_tpu_timings[22]
  #define TRF_T24 rf_tx_tpu_timings[23]
  #define TRF_T25 rf_tx_tpu_timings[24]
  #define TRF_T26 rf_tx_tpu_timings[25]
  #define TRF_T27 rf_tx_tpu_timings[26]
  #define TRF_T28 rf_tx_tpu_timings[27]
  #define TRF_T29 rf_tx_tpu_timings[28]
  #define TRF_T30 rf_tx_tpu_timings[29]
  #define TRF_T31 rf_tx_tpu_timings[30]
  #define TRF_T32 rf_tx_tpu_timings[31]


  #define TRF_R1 rf_rx_tpu_timings[0]
  #define TRF_R2 rf_rx_tpu_timings[1]
  #define TRF_R3 rf_rx_tpu_timings[2]
  #define TRF_R4 rf_rx_tpu_timings[3]
  #define TRF_R5 rf_rx_tpu_timings[4]
  #define TRF_R6  rf_rx_tpu_timings[5]
  #define TRF_R7  rf_rx_tpu_timings[6]
  #define TRF_R8  rf_rx_tpu_timings[7]
  #define TRF_R9  rf_rx_tpu_timings[8]
  #define TRF_R10 rf_rx_tpu_timings[9]
  #define TRF_R11  rf_rx_tpu_timings[10]
  #define TRF_R12  rf_rx_tpu_timings[11]
  #define TRF_R13 rf_rx_tpu_timings[12]
  #define TRF_R14 rf_rx_tpu_timings[13]
  #define TRF_R15 rf_rx_tpu_timings[14]
  #define TRF_R16  rf_rx_tpu_timings[15]
  #define TRF_R17 rf_rx_tpu_timings[16]
  #define TRF_R18  rf_rx_tpu_timings[17]
  #define TRF_R19  rf_rx_tpu_timings[18]
  #define TRF_R20 rf_rx_tpu_timings[19]
  #define TRF_R21 rf_rx_tpu_timings[20]
  #define TRF_R22 rf_rx_tpu_timings[21]
  #define TRF_R23 rf_rx_tpu_timings[22]
  #define TRF_R24 rf_rx_tpu_timings[23]
  #define TRF_R25 rf_rx_tpu_timings[24]
  #define TRF_R26 rf_rx_tpu_timings[25]
  #define TRF_R27 rf_rx_tpu_timings[26]
  #define TRF_R28 rf_rx_tpu_timings[27]
  #define TRF_R29 rf_rx_tpu_timings[28]
  #define TRF_R30 rf_rx_tpu_timings[29]
  #define TRF_R31 rf_rx_tpu_timings[30]
  #define TRF_R32 rf_rx_tpu_timings[31]

  //Flexi ABB Delays
  // The existing #defines in the code would have to be removed and the definitions done here.

  extern WORD16 rf_flexi_abb_delays[];



  #ifdef APC_RAMP_UP_TIME
    #undef APC_RAMP_UP_TIME
  #endif
  #define APC_RAMP_UP_TIME rf_flexi_abb_delays[1]  //default 32


  #ifdef DL_ABB_DELAY
    #undef DL_ABB_DELAY
  #endif
  #define DL_ABB_DELAY (rf_flexi_abb_delays[2])  //   ( 32L + 4L)


  #ifdef UL_ABB_DELAY
    #undef UL_ABB_DELAY
  #endif
  #define UL_ABB_DELAY rf_flexi_abb_delays[3] //12

  #ifdef UL_DELAY_1RF
    #undef UL_DELAY_1RF
  #endif
  #define UL_DELAY_1RF rf_flexi_abb_delays[4] //0

  #ifdef UL_DELAY_2RF
    #undef UL_DELAY_2RF
  #endif
  #define UL_DELAY_2RF rf_flexi_abb_delays[5] //0

  #ifdef APCDEL_DOWN
    #undef APCDEL_DOWN
  #endif
  #define APCDEL_DOWN rf_flexi_abb_delays[6]  //3//3

  #ifdef APCDEL_UP
    #undef APCDEL_UP
  #endif
  #define APCDEL_UP  rf_flexi_abb_delays[7] //24

  #ifdef GUARD_BITS
    #undef GUARD_BITS
  #endif
  #define GUARD_BITS rf_flexi_abb_delays[8] //7

  #ifdef SETUP_AFC_AND_RF
    #undef SETUP_AFC_AND_RF
  #endif
  #define SETUP_AFC_AND_RF rf_flexi_abb_delays[9] //6

  #ifdef SERV_OFFS_REST_LOAD
    #undef SERV_OFFS_REST_LOAD
  #endif
  #define SERV_OFFS_REST_LOAD rf_flexi_abb_delays[10]  // 1L

  #ifdef TA_MAX
    #undef TA_MAX
  #endif
  #define TA_MAX rf_flexi_abb_delays[11] // ( 63L * 4L )

  #ifdef PRG_TX
    #undef PRG_TX
  #endif
  #define PRG_TX rf_flexi_abb_delays[12] // (DL_DELAY_RF + UL_DELAY_2RF + (GUARD_BITS*4) + UL_DELAY_1RF + UL_ABB_DELAY)

  #ifdef EPSILON_OFFS
    #undef EPSILON_OFFS
  #endif
  #define EPSILON_OFFS rf_flexi_abb_delays[14] // 2L

  #ifdef EPSILON_MEAS
    #undef EPSILON_MEAS
  #endif
  #define EPSILON_MEAS rf_flexi_abb_delays[15]//20L

  #ifdef EPSILON_SYNC
    #undef EPSILON_SYNC
  #endif
  #define EPSILON_SYNC rf_flexi_abb_delays[16] //10L

  #ifdef APC_RAMP_DOWN_TIME
    #undef APC_RAMP_DOWN_TIME
  #endif
  #define APC_RAMP_DOWN_TIME rf_flexi_abb_delays[17] //32

  #ifdef APC_RAMP_DELAY
    #undef APC_RAMP_DELAY
  #endif
  #define APC_RAMP_DELAY rf_flexi_abb_delays[18] //6
#endif //L1_TPU_DEV
// End Flexible TPU  Timings, ABB DElays

#if ( L1_TPU_DEV == 0)
// - RX timings ---
// - RX down:
#if (L1_MADC_ON == 1)
#define TRF_R8   (PROVISION_TIME - 170 - DLT_4B - rdt )    // for doing MADC
#endif
// The times below are offsets to when BDLENA goes down
#define TRF_R7   (  2 - DLT_4B )            // Power down RF (idle script)
// TRF_R6 not use, warning timing TRF_R6 > TRF_R7
#define TRF_R6   (  -20 - DLT_4B)            // Disable RX Start and RF switch

// - RX up:
// The times below are offsets to when BDLENA goes high
// Burst data comes here
#define TRF_R5   (PROVISION_TIME -  19  - DLT_1   )    // Enable RX_START
#define TRF_R4   (PROVISION_TIME -  39 - DLT_1  - rdt )    // Set RF switch for RX in selected band
#if(L1_RF_KBD_FIX == 1)
#define TRF_R3_1 (PROVISION_TIME -  89 - DLT_1  - rdt )    // Disable keyboard
#endif
#define TRF_R3   (PROVISION_TIME - 190 - DLT_4B - rdt )    // RX_ON
#define TRF_R2   (PROVISION_TIME - 197 - DLT_4B - rdt )    // Select the AGC & LNA gains
#define TRF_R1   (PROVISION_TIME - 203 - DLT_4B - rdt )    // Set RX Synth channel



// - TX timings ---
// - TX down:
// The times below are offsets to when TXSTART goes down
#define TRF_T13 (    25 - DLT_1        )           //
#define TRF_T12 (    23 - DLT_4B        )           //
#define TRF_T11 (    16 - DLT_1        )     //
#define TRF_T10  (    0 - DLT_1        )     //
#ifdef VARIABLE_ADC_ON_TX
#define TRF_T9 (TRF_T11-VARIABLE_ADC_ON_TX*4)
#else
#define TRF_T9  ( -  40 - DLT_1        )     //
#endif

// - TX up:
// The times below are offsets to when TXSTART goes high
//burst data comes here
#define TRF_T8  (     18 - DLT_1        )     //
#define TRF_T7  (    12 - DLT_1        )     //
#define TRF_T6  (     6 - DLT_1        )     //
#define TRF_T5  (     0 - DLT_1        )     //
#define TRF_T4  ( - 100 - DLT_1        )     //
#if(L1_RF_KBD_FIX == 1)
#define TRF_T3_1 (-215 -DLT_1          )     // Disable keyboard
#endif
#define TRF_T3  ( - 225 - DLT_1        )     // Set APC_LDO enabled
#define TRF_T2  ( - 235 - DLT_4B - tdt )     // Power ON TX
#define TRF_T1  ( - 255 - DLT_4B - tdt )     // Set TX synth

#endif // Non TPU DEV




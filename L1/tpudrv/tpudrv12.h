/****************** Revision Controle System Header ***********************
 *                      GSM Layer 1 software                                
 *              Copyright (c) Texas Instruments 1998                      
 *                                                                        
 *        Filename tpudrv12.h
 *  Copyright 2003 (C) Texas Instruments  
 *                                                                        
 ****************** Revision Controle System Header ***********************/

//--- Configuration values
#define FEM_TEST            0                     // 1 => ENABLE the FEM_TEST mode
#define RF_VERSION          1                     // 1 or V1, 5 for V5, etc
#define SAFE_INIT_WA        0                     // 1 => ENABLE the "RITA safe init"
// TeST - Enable Main VCO buffer for test
#define MAIN_VCO_ACCESS_WA  0                     // 1 => ENABLE the Main VCO buffer

#if 0	// FreeCalypso
#include "rf.cfg"
#endif

//--- RITA PG declaration

#define R_PG_10 0
#define R_PG_13 1
#define R_PG_20 2 // For RFPG 2.2, use 2.0
#define R_PG_23 3

//--- PA declaration
#define PA_MGF9009 0
#define PA_RF3146 1
#define PA_RF3133 2
#define PA_PF08123B 3
#define PA_AWT6108 4

#if (RF_PA == PA_MGF9009 || RF_PA == PA_PF08123B)
  #define PA_CTRL_INT 0
#else
  #define PA_CTRL_INT 1
#endif

//- Select the RF PG (x10), i.e. 10 for 1.0, 11 for 1.1 or 20 for 2.0
// AlphaRF7 => "PG #1.3" for TPU purposes (not an official PC number)
// This is also used in l1_rf12.h to select the SWAP_IQ
#if   (RF_PG >= R_PG_20)
    // TeST - PLL2 WA activation => Set PLL2 Speed-up ON in RX
    #define PLL2_WA             0           // 0 => DISABLE the PLL2_WA (Rene's "Work-Around")
    #define ALPHA_RF7_WA        0           // 0 => DISABLE the Alpha RF7 work-arounds
#elif (RF_PG == R_PG_13)
    // TeST - PLL2 WA activation => Set PLL2 Speed-up ON in RX
    #define PLL2_WA             1           // 1 => ENABLE the PLL2_WA (Rene's "Work-Around")
    #define ALPHA_RF7_WA        1           // 1 => ENABLE the Alpha RF7 work-arounds
#else
    // TeST - PLL2 WA activation => Set PLL2 Speed-up ON in RX
    #define PLL2_WA             1           // 1 => ENABLE the PLL2_WA (Rene's "Work-Around")
    #define ALPHA_RF7_WA        1           // 1 => ENABLE the Alpha RF7 work-arounds
#endif

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

//--- TRF6151 definitions ------------------------------------------

//- BASE REGISTER definitions
#define REG_RX          0x000000    // MODE0
#define REG_PLL         0x000001    // MODE1
#define REG_PWR         0x000002    // MODE2
#define REG_CFG         0x000003    // MODE3

//- TeST REGISTER definitions => Used for WA only
// TeST - PLL2 WA => Define PLL2 TEST register
#define TST_PLL2        0x00001E    // MODE 14

// TeST - Enable Main VCO buffer for test => Define TST_VCO3 register
#define TST_VCO3        0x00000F    // MODE 15 (0*16+15*1)
#define TST_VCO4        0x000024    // MODE 36 (2*16+4*1)

// Alpha RF7 WA TeST registers
#define TST_LDO         0x000027    // MODE 39 (2*16+7*1)
#define TST_PLL1        0x00001D    // MODE 29 (1*16+13*1)
#define TST_TX2         0x000037    // MODE 55 (3*16+7*1)

// More Alpha RF7 WA TeST registers
#define TST_TX3         0x00003C    // MODE 61 (3*16+12*1)
#define TST_TX4         0x00003D    // MODE 61 (3*16+13*1)

// PG 2.1 WA TeST registers
#define TST_PLL3        0x00001F    // MODE 31 (1*16+15*1)
//  #define TST_PLL4        0x00002C    // MODE 44 (2*16+12*1)
#define TST_MISC       0x00003E    // MODE 62 (3*16+14*1) => Used for setting the VCXO current
#define TST_LO           0x00001C   // MODE 28 (1*16+12*1)

// Registers used to improve the Modulation Spectrum in DCS/PCS for PG2.1 V1
// UPDATE_SERIAL_REGISTER_COPY is a "dummy addres" that,
// when accessed, triggers the copy of the serial registers.
// This is necessary to switch into "manual operation mode"
#define UPDATE_SERIAL_INTERFACE_COPY    0x000007
#define TX_LOOP_MANUAL    BIT_3


//- REG_RX - MODE0
#define  BLOCK_DETECT_0       BIT_3    
#define  BLOCK_DETECT_1       BIT_4    
#define  RST_BLOCK_DETECT_0   BIT_5    
#define  RST_BLOCK_DETECT_1   BIT_6    
#define  READ_EN              BIT_7    
#define  RX_CAL_MODE          BIT_8    
#define  RF_GAIN             (BIT_10 | BIT_9)


//- REG_PLL - MODE1
//PLL_REGB
//PLL_REGA

//- REG_PWR - MODE2
#define  BANDGAP_MODE_OFF     0x0
#define  BANDGAP_MODE_ON_ENA  BIT_4
#define  BANDGAP_MODE_ON_DIS (BIT_4 | BIT_3)
#define  REGUL_MODE_ON        BIT_5    
// BIT[8..6]  band
#define  BAND_SELECT_GSM      BIT_6
#define  BAND_SELECT_DCS      BIT_7   
#define  BAND_SELECT_850_LO   BIT_8
#define  BAND_SELECT_850_HI  (BIT_8 | BIT_6)
#define  BAND_SELECT_PCS     (BIT_8 | BIT_7)   

#define  SYNTHE_MODE_OFF      0x0
#define  SYNTHE_MODE_RX       BIT_9
#define  SYNTHE_MODE_TX       BIT_10
#define  RX_MODE_OFF          0x0
#define  RX_MODE_A            BIT_11
#define  RX_MODE_B1           BIT_12
#define  RX_MODE_B2          (BIT_12 | BIT_11)
#define  TX_MODE_OFF          0x0
#define  TX_MODE_ON           BIT_13
#define  PACTRL_APC_OFF       0x0
#define  PACTRL_APC_ON        BIT_14
#define  PACTRL_APC_DIS       0x0
#define  PACTRL_APC_ENA       BIT_15    


//- REG_CFG - MODE3
// Common PA controller settings:
#define  PACTRL_TYPE_PWR      0x0
#define  PACTRL_TYPE_CUR      BIT_3
#define  PACTRL_IDIOD_30_UA   0x0
#define  PACTRL_IDIOD_300_UA  BIT_4 

// PA controller Clara-like (Power Sensing) settings:
  #define  PACTRL_VHOME_610_MV   (BIT_7 | BIT_5)
#define  PACTRL_VHOME_839_MV  (BIT_7 | BIT_5)
  #define  PACTRL_VHOME_1000_MV (BIT_6 | BIT_9)
  #define  PACTRL_VHOME_1600_MV (BIT_8 | BIT_5)
  #define  PACTRL_RES_OPEN       0x0
  #define  PACTRL_RES_150_K      BIT_10
  #define  PACTRL_RES_300_K      BIT_11
  #define  PACTRL_RES_NU        (BIT_10 | BIT_11)
  #define  PACTRL_CAP_0_PF       0x0
  #define  PACTRL_CAP_12_5_PF    BIT_12 
  #define  PACTRL_CAP_25_PF     (BIT_13 | BIT_12)
  #define  PACTRL_CAP_50_PF      BIT_13

  // PACTRL_CFG contains the configuration of the PACTRL that will
  // be put into the REG_CFG register at initialization time
  // WARNING - Do not forget to set the PACTRL_TYPE (PWR or CUR)
  //           in this #define!!!
#if (RF_PA == 0)      // MGF9009 (LCPA)
        #define  PACTRL_CFG \
                          PACTRL_IDIOD_300_UA      |  \
                          PACTRL_CAP_25_PF         |  \
                          PACTRL_VHOME_1000_MV     |  \
                          PACTRL_RES_300_K  
#elif (RF_PA == 1)  // 3146
  #define  PACTRL_CFG 0

#elif (RF_PA == 2)  // 3133
        #define  PACTRL_CFG 0  

#elif (RF_PA == 3)  // PF08123B
        #define  PACTRL_CFG \
                          PACTRL_TYPE_PWR     | \
                          PACTRL_CAP_50_PF    | \
                          PACTRL_RES_300_K    | \
                          PACTRL_VHOME_610_MV
#elif (RF_PA == 4)  // AWT6108
  #define  PACTRL_CFG 0
  #else
  #error  Unknown PA specifiec!
  #endif

// Temperature sensor
#define  TEMP_SENSOR_OFF      0x0
#define  TEMP_SENSOR_ON       BIT_14
// Internal Logic Init Disable
#define  ILOGIC_INIT_DIS      BIT_15
// ILOGIC_INIT_DIS must be ALWAYS set when programming the REG_CFG register
// It was introduced in PG 1.2
// For previous PGs this BIT was unused, so it can be safelly programmed
// for all PGs


// RF signals connected to TSPACT  [0..7]

#if CONFIG_TARGET_PIRELLI
#define	RF_RESET_LINE	BIT_5
#else
#define	RF_RESET_LINE	BIT_0
#endif

#define RF_SER_ON     RF_RESET_LINE
#define RF_SER_OFF    0      

#define TEST_TX_ON    0
#define TEST_RX_ON    0

#if CONFIG_TARGET_LEONARDO || CONFIG_TARGET_ESAMPLE

  // 4-band config (E-sample, P2, Leonardo)
  #define FEM_7         BIT_2     // act2
  #define FEM_8         BIT_1     // act1
  #define FEM_9         BIT_4     // act4

  #define PA_HI_BAND    BIT_3   // act3
  #define PA_LO_BAND    0
  #define PA_OFF        0

  #define FEM_PINS (FEM_7 | FEM_8 | FEM_9)

  #define FEM_OFF    ( FEM_PINS ^ 0 )

  #define FEM_SLEEP  ( 0 )  

  // This configuration is always inverted.

  // RX_UP/DOWN and TX_UP/DOWN
  #define RU_900     ( PA_OFF     | FEM_PINS ^ 0     )
  #define RD_900     ( PA_OFF     | FEM_PINS ^ 0     )
  #define TU_900     ( PA_LO_BAND | FEM_PINS ^ FEM_7 )
  #define TD_900     ( PA_OFF     | FEM_PINS ^ 0     )

  #define RU_850     ( PA_OFF     | FEM_PINS ^ FEM_9 )
  #define RD_850     ( PA_OFF     | FEM_PINS ^ 0     )
  #define TU_850     ( PA_LO_BAND | FEM_PINS ^ FEM_7 )
  #define TD_850     ( PA_OFF     | FEM_PINS ^ 0     )

  #define RU_1800    ( PA_OFF     | FEM_PINS ^ 0     )
  #define RD_1800    ( PA_OFF     | FEM_PINS ^ 0     )
  #define TU_1800    ( PA_HI_BAND | FEM_PINS ^ FEM_8 )
  #define TD_1800    ( PA_OFF     | FEM_PINS ^ 0     )

  #define RU_1900    ( PA_OFF     | FEM_PINS ^ 0     )
  #define RD_1900    ( PA_OFF     | FEM_PINS ^ 0     )
  #define TU_1900    ( PA_HI_BAND | FEM_PINS ^ FEM_8 )
  #define TD_1900    ( PA_OFF     | FEM_PINS ^ 0     )

#elif CONFIG_TARGET_GTAMODEM || CONFIG_TARGET_FCDEV3B

  // Openmoko's triband configuration is a bastardized version
  // of TI's quadband one from Leonardo/E-Sample

  #define FEM_7         BIT_2     // act2
  #define FEM_8         BIT_1     // act1
  #define FEM_9         BIT_4     // act4

  #define PA_HI_BAND    BIT_3   // act3
  #define PA_LO_BAND    0
  #define PA_OFF        0

  #define FEM_PINS (FEM_7 | FEM_8 | FEM_9)

  #define FEM_OFF    ( FEM_PINS ^ 0 )

  #define FEM_SLEEP  ( 0 )  

  // This configuration is always inverted.

  // RX_UP/DOWN and TX_UP/DOWN
  #define RU_900     ( PA_OFF     | FEM_PINS ^ 0     )
  #define RD_900     ( PA_OFF     | FEM_PINS ^ 0     )
  #define TU_900     ( PA_LO_BAND | FEM_PINS ^ FEM_9 )
  #define TD_900     ( PA_OFF     | FEM_PINS ^ 0     )

  #define RU_850     ( PA_OFF     | FEM_PINS ^ 0     )
  #define RD_850     ( PA_OFF     | FEM_PINS ^ 0     )
  #define TU_850     ( PA_LO_BAND | FEM_PINS ^ FEM_9 )
  #define TD_850     ( PA_OFF     | FEM_PINS ^ 0     )

  #define RU_1800    ( PA_OFF     | FEM_PINS ^ 0     )
  #define RD_1800    ( PA_OFF     | FEM_PINS ^ 0     )
  #define TU_1800    ( PA_HI_BAND | FEM_PINS ^ FEM_7 )
  #define TD_1800    ( PA_OFF     | FEM_PINS ^ 0     )

  #define RU_1900    ( PA_OFF     | FEM_PINS ^ FEM_8 )
  #define RD_1900    ( PA_OFF     | FEM_PINS ^ 0     )
  #define TU_1900    ( PA_HI_BAND | FEM_PINS ^ FEM_7 )
  #define TD_1900    ( PA_OFF     | FEM_PINS ^ 0     )

#elif CONFIG_TARGET_PIRELLI

  #define ANTSW_RX_PCS	BIT_4
  #define ANTSW_TX_HIGH	BIT_10
  #define ANTSW_TX_LOW	BIT_11

  #define PA_HI_BAND    BIT_3   // act3
  #define PA_LO_BAND    0
  #define PA_OFF        0

  #define PA_ENABLE	BIT_0

  // Pirelli uses a non-inverting buffer

  #define FEM_OFF    ( 0 )

  #define FEM_SLEEP  ( 0 )  

  // RX_UP/DOWN and TX_UP/DOWN (triband)
  #define RU_900     ( PA_OFF     | 0             )
  #define RD_900     ( PA_OFF     | 0             )
  #define TU_900     ( PA_LO_BAND | ANTSW_TX_LOW  )
  #define TD_900     ( PA_OFF     | 0             )

  #define RU_850     ( PA_OFF     | 0             )
  #define RD_850     ( PA_OFF     | 0             )
  #define TU_850     ( PA_LO_BAND | ANTSW_TX_LOW  )
  #define TD_850     ( PA_OFF     | 0             )

  #define RU_1800    ( PA_OFF     | 0             )
  #define RD_1800    ( PA_OFF     | 0             )
  #define TU_1800    ( PA_HI_BAND | ANTSW_TX_HIGH )
  #define TD_1800    ( PA_OFF     | 0             )

  #define RU_1900    ( PA_OFF     | ANTSW_RX_PCS  )
  #define RD_1900    ( PA_OFF     | 0             )
  #define TU_1900    ( PA_HI_BAND | ANTSW_TX_HIGH )
  #define TD_1900    ( PA_OFF     | 0             )

#elif CONFIG_TARGET_COMPAL

  #define PA_HI_BAND    BIT_8   // act8
  #define PA_LO_BAND    0
  #define PA_OFF        0

  #define PA_ENABLE	BIT_1

  // FEM control signals are active low
  #define FEM_PINS (BIT_6 | BIT_2)

  #define FEM_OFF    ( FEM_PINS ^ 0 )

  #define FEM_SLEEP  ( 0 )  

  #define FEM_TX_HIGH	BIT_6
  #if USE_TSPACT2_FOR_TXLOW
    #define FEM_TX_LOW	BIT_2
  #else
    #define FEM_TX_LOW	BIT_6
  #endif

  // RX_UP/DOWN and TX_UP/DOWN
  #define RU_900     ( PA_OFF     | FEM_PINS ^ 0           )
  #define RD_900     ( PA_OFF     | FEM_PINS ^ 0           )
  #define TU_900     ( PA_LO_BAND | FEM_PINS ^ FEM_TX_LOW  )
  #define TD_900     ( PA_OFF     | FEM_PINS ^ 0           )

  #define RU_850     ( PA_OFF     | FEM_PINS ^ 0           )
  #define RD_850     ( PA_OFF     | FEM_PINS ^ 0           )
  #define TU_850     ( PA_LO_BAND | FEM_PINS ^ FEM_TX_LOW  )
  #define TD_850     ( PA_OFF     | FEM_PINS ^ 0           )

  #define RU_1800    ( PA_OFF     | FEM_PINS ^ 0           )
  #define RD_1800    ( PA_OFF     | FEM_PINS ^ 0           )
  #define TU_1800    ( PA_HI_BAND | FEM_PINS ^ FEM_TX_HIGH )
  #define TD_1800    ( PA_OFF     | FEM_PINS ^ 0           )

  #define RU_1900    ( PA_OFF     | FEM_PINS ^ 0           )
  #define RD_1900    ( PA_OFF     | FEM_PINS ^ 0           )
  #define TU_1900    ( PA_HI_BAND | FEM_PINS ^ FEM_TX_HIGH )
  #define TD_1900    ( PA_OFF     | FEM_PINS ^ 0           )

#endif  // FreeCalypso target selection

#define TC1_DEVICE_ABB     TC1_DEVICE0  // TSPEN0
#if CONFIG_TARGET_PIRELLI
#define TC1_DEVICE_RF      TC1_DEVICE1  // TSPEN1
#else
#define TC1_DEVICE_RF      TC1_DEVICE2  // TSPEN2
#endif


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
#define SL_SU_DELAY2    DLT_3 // Needed to compile with old l1_rf12

// - Serialization timings ---
// The following values where calculated with Katrin Matthes...
//#define SL_7        3  // To send 7 bits to the ABB, 14*T (1/6.5MHz) are needed,
//                       // i.e. 14 / 6 qbit = 2.333 ~ 3 qbit
//#define SL_2B       6  // To send 2 bytes to the RF, 34*T (1/6.5MHz) are needed,
//                       // i.e. 34 / 6 qbit = 5.7 ~ 6 qbit
// ... while the following values are based on the HYP004.doc document
#define SL_7        2    // To send 7 bits to the ABB, 12*T (1/6.5MHz) are needed,
                         // i.e. 12 / 6 qbit = 2 qbit
#define SL_2B       4    // To send 2 bytes to the RF, 21*T (1/6.5MHz) are needed,
                         // i.e. 21 / 6 qbit = 3.5 ~ 4 qbit

// - TPU command execution + serialization length ---
#define DLT_1B    4                   // 3*move + serialization of 7 bits
#define DLT_2B    7                   // 4*move + serialization of 2 bytes
//#define DLT_1B    DLT_3 + SL_7      // 3*move + serialization of 7 bits
//#define DLT_2B    DLT_4 + SL_2B     // 4*move + serialization of 2 bytes


// - INIT (delta or DLT) timings ---
#define DLT_I1  5           // Time required to set EN high before RF_SER_OFF -> RF_SER_ON
#define DLT_I2  8           // Time required to set RF_SER_OFF
#define DLT_I3  5           // Time required to set RF_SER_ON
#define DLT_I4  110 // Regulator Turn-ON time


// - tdt & rdt ---
// MAX GSM (not GPRS) rdt and tdt values are...
//#define rdt       380 // MAX GSM rx delta timing
//#define tdt       400 // MAX GSM tx delta timing
// but current rdt and tdt values are...
#define rdt       0                   // rx delta timing
#define tdt       0                   // tx delta timing

// - RX timings ---
// - RX down:
// The times below are offsets to when BDLENA goes down
#define TRF_R10  (    0 - DLT_1B )    // disable BDLENA & BDLON -> power DOWN ABB (end of RX burst), needs DLT_1B to execute
#define TRF_R9   ( - 30 - DLT_2B )    // disable RF SWITCH, power DOWN Rita (go to Idle2 mode)

// - RX up:
// The times below are offsets to when BDLENA goes high
// Burst data comes here
#define TRF_R8   ( PROVISION_TIME -   0 - DLT_1B )          // enable BDLENA, disable BDLCAL (I/Q comes 32qbit later)
#define TRF_R7   ( PROVISION_TIME -   7 - DLT_1  )          // enable RF SWITCH
#define TRF_R6   ( PROVISION_TIME -  67 - DLT_1B )          // enable BDLCAL -> ABB DL filter init
#define TRF_R5   ( PROVISION_TIME -  72 - DLT_1B )          // enable BDLON -> power ON ABB DL path
#define TRF_R4   ( PROVISION_TIME -  76 - DLT_2B - rdt )    // power ON RX
#define TRF_R3   (PROVISION_TIME - 143 - DLT_2B - rdt )     // select the AGC & LNA gains + start DC offset calibration (stops automatically)
//l1dmacro_adc_read_rx() called here requires ~ 16 tpuinst
#define TRF_R2   (PROVISION_TIME - 198 - DLT_2B - rdt )     // set BAND + power ON RX Synth
#define TRF_R1   (PROVISION_TIME - 208 - DLT_2B - rdt )     // set RX Synth channel

// - TX timings ---
// - TX down:
// The times below are offsets to when BULENA goes down

#if (PA_CTRL_INT == 1)
#define TRF_T13     (    35 - DLT_1B )                      // right after, BULON low
#define TRF_T12_5   (    32 - DLT_2B )                      // Power OFF TX loop => power down RF.
#define TRF_T12_3   (    23 - DLT_1 )                      // Disable TXEN.
#endif

#if (PA_CTRL_INT == 0)
#define TRF_T13     (    35 - DLT_1B )                      // right after, BULON low
#define TRF_T12_2   (    32 - DLT_2B )                      // power down RF step 2
#define TRF_T12     (    18 - DLT_2B )                      // power down RF step 1
#endif

#define TRF_T11   (     0 - DLT_1B )                    // disable BULENA -> end of TX burst
#define TRF_T10_5 ( -  40 - DLT_1B )                    // ADC read

// - TX up:
// The times below are offsets to when BULENA goes high
//burst data comes here
#define TRF_T10_4 (    22 - DLT_1  )                   // enable RF SWITCH + TXEN
#define TRF_T10   (    17 - DLT_1  )                   // enable RF SWITCH

#if (PA_CTRL_INT == 0)
#define TRF_T9  (     8 - DLT_2B )                   // enable PACTRL
#endif

#define TRF_T8    ( -   0 - DLT_1B )                   // enable BULENA -> start of TX burst
#define TRF_T7    ( -  50 - DLT_1B - tdt )             // disable BULCAL -> stop ABB UL calibration
#define TRF_T6    ( - 130 - DLT_1B - tdt )             // enable BULCAL -> start ABB UL calibration
#define TRF_T5  ( - 158 - DLT_2B - tdt )             // power ON TX
#define TRF_T4    ( - 190 - DLT_1B - tdt )             // enable BULON -> power ON ABB UL path
// TRF_T3_MAN_1, TRF_T3_MAN_2 & TRF_T3_MAN_3 are only executed in DCS for PG 2.0 and above
#define TRF_T3_MAN_3 ( - 239 - DLT_2B - tdt )        // PG2.1: Set the right TX loop charge pump current for DCS & PCS
#define TRF_T3_MAN_2 ( - 249 - DLT_2B - tdt )        // PG2.1: Go into "TX Manual mode"
#define TRF_T3_MAN_1 ( - 259 - DLT_2B - tdt )        // PG2.1: IN DCS, use manual mode: Copy Serial Interface Registers for "Manual operation"
#define TRF_T3       ( - 259 - DLT_2B - tdt )        // PG2.1: In GSM & PCS go to "Automatic TX mode"
#define TRF_T2  ( - 269 - DLT_2B - tdt )             // PG2.0: set BAND + Power ON Main TX PLL + PACTRL ON
#define TRF_T1  ( - 279 - DLT_2B - tdt )             // set TX Main PLL channel


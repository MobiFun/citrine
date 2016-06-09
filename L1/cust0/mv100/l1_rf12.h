/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 *
 *        Filename l1_rf12.h
 *        Version  1.9
 *        Date     03/21/03
 *
 ************* Revision Controle System Header *************/

#if (OP_L1_STANDALONE == 1)
  // Define the correct enumeration of PA. Consult tpudrv12.h for the enumeration.
  #if ((BOARD == 40) || (BOARD == 41) || (BOARD == 45)) // EvaRita + D-sample or EvaConso
    #define PA 3
  #else
    #define PA 0
  #endif
#else
#include "rf.cfg"
//#define PA 3
#endif

#ifndef PA
  #error PA not defined.
#endif

#define RF_RITA_10 0x2030 // Check with TIDK

//#define RF_HW_BAND_EGSM
//#define RF_HW_BAND_DCS
#define RF_HW_BAND_PCS 0x4
#define RF_HW_BAND_DUAL_US  0x80
#define RF_HW_BAND_DUAL_EXT 0x20

//#define RF_HW_BAND_SUPPORT (0x0020 | RF_HW_BAND_PCS)  // radio_band_support E-GSM/DCS + PCS
// radio_band_support E-GSM/DCS + GSM850/PCS
#define RF_HW_BAND_SUPPORT (RF_HW_BAND_DUAL_EXT | RF_HW_BAND_DUAL_US)

/************************************/
/* SYNTHESIZER setup time...        */
/************************************/
#define RX_SYNTH_SETUP_TIME (PROVISION_TIME - TRF_R1)//RX Synthesizer setup time in qbit.
#define TX_SYNTH_SETUP_TIME (- TRF_T1)               //TX Synthesizer setup time in qbit.

/************************************/
/* time for TPU scenario ending...  */
/************************************/
//
// The following values are used to take into account any TPU activity AFTER
// BDLON (or BDLENA) down (for RX) and BULON down (for TX)
// - If there are no TPU commands after BDLON (or BDLENA) down and BULON down,
//   these defines must be ZERO
// - If there IS some TPU command after BDLON (or BDLENA) and BULON down,
//   these defines must be equal to the time difference (in qbits) between
//   the BDLON (or BDLENA) or BULON time and the last TPU command on
//   the TPU scenario
#define RX_TPU_SCENARIO_ENDING  0   // execution time of AFTER BDLENA down
#define TX_TPU_SCENARIO_ENDING  0   // execution time of AFTER BULON down

 
/******************************************************/
/* TXPWR configuration...                             */
/* Fixed TXPWR value when GSM management is disabled. */
/******************************************************/
#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
//  #define FIXED_TXPWR       0x3f12  // TXPWR=10, value=252
//#define FIXED_TXPWR       0x1952
  #define FIXED_TXPWR       0x1d12  // TXPWR=15
#endif


/************************************/
/* ANALOG delay (in qbits)          */
/************************************/
#define  DL_DELAY_RF      1   // time spent in the Downlink global RF chain by the modulated signal
#if (PA == 3)  // Hitachi
#define  UL_DELAY_1RF     5   // time spent in the first  uplink RF block
#else
#define  UL_DELAY_1RF     7   // time spent in the first  uplink RF block
#endif
#define  UL_DELAY_2RF     0   // time spent in the second uplink RF block
#if (ANALOG == 1)
  #define  UL_ABB_DELAY   6   // modulator input to output delay
#endif
#if ((ANALOG == 2) || (ANALOG == 3))
  #define  UL_ABB_DELAY   3   // modulator input to output delay
#endif

/************************************/
/* TX Propagation delay...          */
/************************************/
#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
  #define  PRG_TX       (DL_DELAY_RF + UL_DELAY_2RF + (GUARD_BITS*4) + UL_DELAY_1RF + UL_ABB_DELAY)   // = 40
#endif

/************************************/
/* Initial value for APC DELAY      */
/************************************/
#if (ANALOG == 1)
//#define APCDEL_DOWN     (32 - GUARD_BITS*4)          // minimum value: 2
  #define APCDEL_DOWN      2                           // minimum value: 2
  #define APCDEL_UP       (6+5)                        // minimum value: 6
#endif
#if (ANALOG == 2) || (ANALOG == 3)
//#define APCDEL_DOWN     (32 - GUARD_BITS*4)          // minimum value: 2
  #define APCDEL_DOWN     (2+0)                        // minimum value: 2
#if (PA == 3)  // Hitachi
#define APCDEL_UP       (6+1)                        // minimum value: 6
#else
  #define APCDEL_UP       (6+3+1)                        // minimum value: 6
#endif
                               // REMOVE // Jerome Modif for ARF7: (6+3) instead of (6+8)
#endif

#define GUARD_BITS 7

/************************************/
/* Initial value for AFC...         */
/************************************/
#define  EEPROM_AFC    ((150)*8)      // F13.3 required!!!!! (default : -952*8, initial deviation of -2400 forced)

#define  SETUP_AFC_AND_RF     6       // AFC converges in 2 frames and RF BAND GAP stable after 4 frames
                               // Rita (RF=12) LDO wakeup requires 6 frames

/************************************/
/* Baseband registers               */
/************************************/
#if (ANALOG == 1)  // Omega registers values will be programmed at 1st DSP communication interrupt
  #define  C_DEBUG1          0x0000          // Enable f_tx delay of 400000 cyc DEBUG
  #define  C_AFCCTLADD       0x002a | TRUE   // Value at reset
  #define  C_VBUR            0x418e | TRUE   // Uplink gain amp 0dB, Sidetone gain to mute
  #define  C_VBDR            0x098c | TRUE   // Downlink gain amp 0dB, Volume control 0 dB
  // RITA does not need an APCOFFSET because the PACTRL is internal:
  // REMOVE //#define  C_APCOFF          0x1016 | (0x3c << 6) | TRUE   // value at reset-Changed from 0x0016- CR 27.12
#if (PA == 3)  // Hitachi
  #define  C_APCOFF          0x1016 | (0x0 << 6) | TRUE
#else
  #define  C_APCOFF          0x1016 | (0x30 << 6) | TRUE
#endif
  #define  C_BULIOFF         0x3fc4 | TRUE   // value at reset
  #define  C_BULQOFF         0x3fc6 | TRUE   // value at reset
  #define  C_DAI_ON_OFF      0x0000          // value at reset
  #define  C_AUXDAC          0x0018 | TRUE   // value at reset
  #define  C_VBCR            0x02d0 | TRUE   // VULSWITCH=1, VDLAUX=1, VDLEAR=1
  // BULRUDEL will be initialized on rach only ....
  #define  C_APCDEL          (((APCDEL_DOWN-2)<<11) | ((APCDEL_UP-6)<<6) | 0x0004)
  #define  C_BBCTL           0x604c | TRUE   // OUTLEV1=OUTLEV1=SELVMID1=SELVMID0=1 for B-sample 'modified'
#endif
#if (ANALOG ==2)
  // IOTA registers values will be programmed at 1st DSP communication interrupt
  #define  C_DEBUG1          0x0001          // Enable f_tx delay of 400000 cyc DEBUG
  #define  C_AFCCTLADD       0x002a | TRUE   // Value at reset
  #define  C_VBUR            0x418e | TRUE   // No uplink mute, Side tone mute, PGA_UL 0dB
  #define  C_VBDR            0x098c | TRUE   // PGA_DL 0dB, Volume 0dB
  // RITA does not need an APCOFFSET because the PACTRL is internal:
  // REMOVE //#define  C_APCOFF          0x1016 | (0x3c << 6) | TRUE   // x2 slope 128
#if (PA == 3)  // Hitachi
  #define  C_APCOFF          0x1016 | (0x0 << 6) | TRUE   // x2 slope 128
#else
  #define  C_APCOFF          0x1016 | (0x30 << 6) | TRUE   // x2 slope 128
#endif
  #define  C_BULIOFF         0x3fc4 | TRUE   // value at reset
  #define  C_BULQOFF         0x3fc6 | TRUE   // value at reset
  #define  C_DAI_ON_OFF      0x0000          // value at reset
  #define  C_AUXDAC          0x0018 | TRUE   // value at reset
  #define  C_VBCR            0x02d0 | TRUE   // VULSWITCH=1, VDLAUX=1, VDLEAR=1
  #define  C_VBCR2           0x0016 | TRUE   // MICBIASEL=0, VDLHSO=0, MICAUX=0
  // BULRUDEL will be initialized on rach only ....
  #define  C_APCDEL          (((APCDEL_DOWN-2)<<11) | ((APCDEL_UP-6)<<6) | 0x0004)
  #define  C_APCDEL2         0x0034
  #define  C_BBCTL           0xB04c | TRUE   // Extenal DL calibration, Output common mode=1.35V
                                             // Monoslot, Vpp=8/15*Vref
  #define  C_BULGCAL         0x001c | TRUE   // IAG=0 dB, QAG=0 dB
#endif

#if (ANALOG == 3)
  // SYREN registers values will be programmed at 1st DSP communication interrupt
  #define  C_DEBUG1          0x0001          // Enable f_tx delay of 400000 cyc DEBUG
  #define  C_AFCCTLADD       0x002a | TRUE   // Value at reset
  #define  C_VBUR            0x1E6<<6 | VBUCTRL | TRUE   //  Side tone mute, PGA_UL 0dB
  #define  C_VBDR            0x026<<6 | VBDCTRL | TRUE   // PGA_DL 0dB, Volume 0dB
#if (PA == 3)  // Hitachi
  #define  C_APCOFF          0x1016 | (0x0 << 6) | TRUE   // x2 slope 128
#else
  #define  C_APCOFF          0x1016 | (0x30 << 6) | TRUE   // x2 slope 128
#endif
#define  C_BULIOFF         0x3fc4 | TRUE   // value at reset
  #define  C_BULQOFF         0x3fc6 | TRUE   // value at reset
  #define  C_DAI_ON_OFF      0x0000          // value at reset
  #define  C_AUXDAC          (0x00<<6) | 0x18 | TRUE   // value at reset
  #define  C_VBCR            (0x108<<6) | 0x10 | TRUE   // VULSWITCH=1 AUXI 28,2 dB
  #define  C_VBCR2           (0x01<<6) | 0x16 | TRUE   // HSMIC on, SPKG gain @ 2,5dB
  // BULRUDEL will be initialized on rach only ....
  #define  C_APCDEL          (((APCDEL_DOWN-2)<<11) | ((APCDEL_UP-6)<<6) | 0x0004)
  #define  C_APCDEL2         0x0034
  #define  C_BBCTL           0xB04c | TRUE   // Internal autocalibration, Output common mode=1.35V
                                             // Monoslot, Vpp=8/15*Vref
  #define  C_BULGCAL         0x001c | TRUE   // IAG=0 dB, QAG=0 dB

  #define  C_VBPOP           (0x4)<<6 | 0x14 | TRUE     // HSOAUTO enabled only
  #define  C_VAUDINITD       2                          // vaud_init_delay init 2 frames
  #define  C_VAUDCR          (0x0)<<6   | 0x1e | TRUE   // Init to zero
  #define  C_VAUOCR          (0x155)<<6 | VAUOCTRL | TRUE   // Speech on all outputs
  #define  C_VAUSCR          (0x0)<<6   | 0x20 | TRUE   // Init to zero
  #define  C_VAUDPLL         (0x0)<<6   | 0x24 | TRUE   // Init to zero

#endif


/************************************/
/* Automatic frequency compensation */
/************************************/
/********************* C_Psi_sta definition *****************************/
/* C_Psi_sta = (2*pi*Fr)        / (N * Fb)                              */
/*      (1)  = (2*pi*V*ppm*0.9) / (N*V*Fb)                              */
/*           regarding Vega V/N   = 2.4/4096                            */
/*           regarding VCO  ppm/V = 16 / 1   (average slope of the VCO) */
/*      (1)  = (2*pi*2.4*16*0.9) / (4096*1*270.83)                      */
/*           = 0.000195748                                              */
/* C_Psi_sta_inv = 1/C_Psi_sta = 5108                                   */
/************************************************************************/
#define  C_Psi_sta_inv    12902L    // (1/C_Psi_sta)
#define  C_Psi_st         4L        // C_Psi_sta * 0.8 F0.16
#define  C_Psi_st_32      266313L  // F0.32
#define  C_Psi_st_inv     16128L   // (1/C_Psi_st)

#if (VCXO_ALGO == 1)
// Linearity parameters
  #define  C_AFC_DAC_CENTER     ((-1242)*8)
  #define  C_AFC_DAC_MIN        ((-2000)*8)
  #define  C_AFC_DAC_MAX        ((1419)*8)

  #define  C_AFC_SNR_THR        2560     //  1/0.4    * 2**10
#endif

typedef struct
{
  WORD16  eeprom_afc;
  UWORD32 psi_sta_inv;
  UWORD32 psi_st;
  UWORD32 psi_st_32;
  UWORD32 psi_st_inv;

  #if (VCXO_ALGO)
  // VCXO adjustment parameters
  // Parameters used when assuming linearity
  WORD16  dac_center;
  WORD16  dac_min;
  WORD16  dac_max;
    WORD16  snr_thr;
  #endif
}
T_AFC_PARAMS;

/************************************/
/* Swap IQ definitions...           */
/************************************/
/* 0=No Swap, 1=Swap RX only, 2=Swap TX only, 3=Swap RX and TX */
#if RF_PG==10
    // PG 1.0 -> 1 (Swap RX only)
    // GSM 850 => TX is ALWAYS swapped compared to GSM 900
    #define  SWAP_IQ_GSM    1
    #define  SWAP_IQ_DCS    1
    #define  SWAP_IQ_PCS    1
    #define  SWAP_IQ_GSM850 3   // Swap TX compared to GSM 900
#else
    // All PG versions ABOVE 1.0 -> 0 (No Swap)
    // GSM 850 => TX is ALWAYS swapped compared to GSM 900
    #define  SWAP_IQ_GSM    0
    #define  SWAP_IQ_DCS    0
    #define  SWAP_IQ_PCS    0
    #define  SWAP_IQ_GSM850 2   // Swap TX compared to GSM 900
#endif

/************************************/
/************************************/
// typedef
/************************************/
/************************************/

/*************************************************************/
/* Define structure for apc of TX Power                 ******/
/*************************************************************/
typedef struct
{ // pcm-file "rf/tx/level.gsm|dcs"
  UWORD16 apc;            // 0..31
  UWORD8  ramp_index;     // 0..RF_TX_RAMP_SIZE
  UWORD8  chan_cal_index; // 0..RF_TX_CHAN_CAL_TABLE_SIZE
}
T_TX_LEVEL;

/************************************/
/* Automatic Gain Control           */
/************************************/
/* Define structure for sub-band definition of TX Power ******/
typedef struct
   {
   UWORD16  upper_bound;    //highest physical arfcn of the sub-band
   WORD16    agc_calib;      // AGC  for each TXPWR
   }T_RF_AGC_BAND;

/************************************/
/*       Ramp definitions           */
/************************************/
#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
  typedef struct
  {
    UWORD8  ramp_up     [16];  // Ramp-up profile
    UWORD8  ramp_down   [16];  // Ramp-down profile
  }
  T_TX_RAMP;
#endif


// RF structure definition
//========================

// Number of bands supported
#define GSM_BANDS 2

#define MULTI_BAND1     0
#define MULTI_BAND2     1
// RF table sizes
#define RF_RX_CAL_CHAN_SIZE 10 // number of AGC sub-bands
#define RF_RX_CAL_TEMP_SIZE 11 // number of temperature ranges

#define RF_TX_CHAN_CAL_TABLE_SIZE  4 // channel calibration table size
#define RF_TX_NUM_SUB_BANDS        8 // number of sub-bands in channel calibration table
#define RF_TX_LEVELS_TABLE_SIZE   32 // level table size
#define RF_TX_RAMP_SIZE           16 // number of ramp definitions
#define RF_TX_CAL_TEMP_SIZE        5 // number of temperature ranges

#define AGC_TABLE_SIZE            20
#define MIN_AGC_INDEX              6

#define TEMP_TABLE_SIZE          131 // number of elements in ADC->temp conversion table


// RX parameters and tables
//-------------------------

// AGC parameters and tables
typedef struct
{
  UWORD16 low_agc_noise_thr;
  UWORD16 high_agc_sat_thr;
  UWORD16 low_agc;
  UWORD16 high_agc;
  UWORD8  il2agc_pwr[121];
  UWORD8  il2agc_max[121];
  UWORD8  il2agc_av[121];
}
T_AGC;

// Calibration parameters
typedef struct
{
  UWORD16 g_magic;
  UWORD16 lna_att;
  UWORD16 lna_switch_thr_low;
  UWORD16 lna_switch_thr_high;
}
T_RX_CAL_PARAMS;

// RX temperature compensation
typedef struct
{
  WORD16 temperature;
  WORD16 agc_calib;
}
T_RX_TEMP_COMP;

// RF RX structure
typedef struct
{
  T_AGC              agc;
}
T_RF_RX; //common

// RF RX structure
typedef struct
{
  T_RX_CAL_PARAMS rx_cal_params;
  T_RF_AGC_BAND   agc_bands[RF_RX_CAL_CHAN_SIZE];
  T_RX_TEMP_COMP  temp[RF_RX_CAL_TEMP_SIZE];
}
T_RF_RX_BAND;


// TX parameters and tables
//-------------------------

// TX temperature compensation
typedef struct
{
  WORD16 temperature;
  #if (ORDER2_TX_TEMP_CAL==1)
  WORD16 a;
  WORD16 b;
  WORD16 c;
  #else
  WORD16 apc_calib;
  #endif
}
T_TX_TEMP_CAL;

// Ramp up and ramp down delay
typedef struct
{
  UWORD16 up;
  UWORD16 down;
}
T_RAMP_DELAY;

typedef struct
{
  UWORD16 arfcn_limit;
  WORD16  chan_cal;
}
T_TX_CHAN_CAL;

// RF TX structure
typedef struct
{
  T_RAMP_DELAY   ramp_delay;
  UWORD8         guard_bits;  // number of guard bits needed for ramp up
  UWORD8         prg_tx;
}
T_RF_TX; //common

// RF TX structure
typedef struct
{
  T_TX_LEVEL           levels[RF_TX_LEVELS_TABLE_SIZE];
  T_TX_CHAN_CAL        chan_cal_table[RF_TX_CHAN_CAL_TABLE_SIZE][RF_TX_NUM_SUB_BANDS];
  T_TX_RAMP            ramp_tables[RF_TX_RAMP_SIZE];
  T_TX_TEMP_CAL        temp[RF_TX_CAL_TEMP_SIZE];
}
T_RF_TX_BAND;

// band structure
typedef struct
{
  T_RF_RX_BAND  rx;
  T_RF_TX_BAND  tx;
  UWORD8        swap_iq;
}
T_RF_BAND;

// RF structure
typedef struct
{
  // common for all bands
  UWORD16      rf_revision;
  UWORD16      radio_band_support;
  T_RF_RX      rx;
  T_RF_TX      tx;
  T_AFC_PARAMS afc;
}
T_RF;

/************************************/
/*       MADC definitions           */
/************************************/
// Omega: 5 external channels if touch screen not used, 3 otherwise
enum ADC_INDEX {
   ADC_VBAT,
   ADC_VCHARG,
   ADC_ICHARG,
   ADC_VBACKUP,
   ADC_BATTYP,
   ADC_BATTEMP,
   ADC_ADC3,     // name of this ??
   ADC_RFTEMP,
   ADC_ADC4,
   ADC_INDEX_END // ADC_INDEX_END must be the end of the enums
};

typedef struct
{
    WORD16 converted[ADC_INDEX_END]; // converted
    UWORD16 raw[ADC_INDEX_END];       // raw from ADC
}
T_ADC;

/************************************/
/*       MADC calibration           */
/************************************/
typedef struct
{
   UWORD16 a[ADC_INDEX_END];
   WORD16 b[ADC_INDEX_END];
}
T_ADCCAL;

// Conversion table: ADC value -> temperature
typedef struct
{
    UWORD16 adc;  // ADC reading is 10 bits
    WORD16  temp; // temp is in approx. range -30..+80
}
T_TEMP;

typedef struct
{
    char *name;
    void *addr;
    int  size;
}
T_CONFIG_FILE;

typedef struct
{
  char      *name;         // name of ffs file suffix
  T_RF_BAND *addr;         // address to default flash structure
  UWORD16   max_carrier;  // max carrier
  UWORD16   max_txpwr;    // max tx power
}
T_BAND_CONFIG;

typedef struct
{
  UWORD8  band[GSM_BANDS]; // index to band address
  UWORD8  txpwr_tp; // tx power turning point
  UWORD16 first_arfcn;  // first index
}
T_STD_CONFIG;
enum GSMBAND_DEF
{
  BAND_NONE,
  BAND_EGSM900,
  BAND_DCS1800,
  BAND_PCS1900,
  BAND_GSM850,
  // put new bands here
  BAND_GSM900  //last entry
};

/************************************/
/* ABB (Omega) Initialization       */
/************************************/

#if ((ANALOG == 1) || (ANALOG == 2))
  #define ABB_TABLE_SIZE 16
#elif (ANALOG == 3)
  #define ABB_TABLE_SIZE 22
#endif

// Note that this translation is probably not needed at all. But until L1 is
// (maybe) changed to simply initialize the ABB from a table of words, we
// use this to make things more easy-readable.
#if (ANALOG == 1)
  enum ABB_REGISTERS {
    ABB_AFCCTLADD = 0,
    ABB_VBUR,
    ABB_VBDR,
    ABB_BBCTL,
    ABB_APCOFF,
    ABB_BULIOFF,
    ABB_BULQOFF,
    ABB_DAI_ON_OFF,
    ABB_AUXDAC,
    ABB_VBCR,
    ABB_APCDEL
  };
#elif (ANALOG == 2)
  enum ABB_REGISTERS {
    ABB_AFCCTLADD = 0,
    ABB_VBUR,
    ABB_VBDR,
    ABB_BBCTL,
    ABB_BULGCAL,
    ABB_APCOFF,
    ABB_BULIOFF,
    ABB_BULQOFF,
    ABB_DAI_ON_OFF,
    ABB_AUXDAC,
    ABB_VBCR,
    ABB_VBCR2,
    ABB_APCDEL,
    ABB_APCDEL2
  };
#elif (ANALOG == 3)
  enum ABB_REGISTERS {
    ABB_AFCCTLADD = 0,
    ABB_VBUR,
    ABB_VBDR,
    ABB_BBCTL,
    ABB_BULGCAL,
    ABB_APCOFF,
    ABB_BULIOFF,
    ABB_BULQOFF,
    ABB_DAI_ON_OFF,
    ABB_AUXDAC,
    ABB_VBCR,
    ABB_VBCR2,
    ABB_APCDEL,
    ABB_APCDEL2,
    ABB_VBPOP,
    ABB_VAUDINITD,
    ABB_VAUDCR,
    ABB_VAUOCR,
    ABB_VAUSCR,
    ABB_VAUDPLL
  };
#endif

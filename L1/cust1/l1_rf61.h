/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 *
 *        Filename l1_rf61.h
 *        Version  1.0
 *        Date     June 16th, 2005
 *
 ************* Revision Controle System Header *************/

#ifndef __L1_RF_H__
#define __L1_RF_H__

#if (ANLG_FAM == 11)
#include "bspTwl3029_Madc.h"
#endif

#define RF_LOCOSTO 0x2050 // Check with TIDK

//LNA Specific BAND Index Settings
#define RF_QUADBAND 0 //Default Setting 850, EGSM,DCS,PCS
#define RF_EU_TRIBAND 1 //  EGSM,DCS,PCS
#define RF_EU_DUALBAND 2 //  EGSM,DCS
#define RF_US_TRIBAND 3  // 850,DCS,PCS
#define RF_US_DUALBAND 4 //850, PCS
#define RF_PCS1900_900_DUALBAND 5 // EGSM, PCS
#define RF_DCS1800_850_DUALBAND 6 //850, DCS

#define RF_BAND_SYSTEM_INDEX RF_QUADBAND //for other PCB's Please redefine here.
#define RF_LNA_MASK 0x0003 // Reuse last 2 bits from

//End LNA Changes

//#define RF_HW_BAND_EGSM
//#define RF_HW_BAND_DCS
#define RF_HW_BAND_PCS 0x4
#define RF_HW_BAND_DUAL_US  0x80
#define RF_HW_BAND_DUAL_EXT 0x20
#define RF_HW_BAND_SUPPORT (RF_HW_BAND_DUAL_EXT)

/************************************/
/* SYNTHESIZER setup time... */
/************************************/
#define RX_SYNTH_SETUP_TIME (PROVISION_TIME - TRF_R1) //RX Synthesizer setup time in qbit.
#define TX_SYNTH_SETUP_TIME (- TRF_T1)               //TX Synthesizer setup time in qbit.

/************************************/
/* Time for TPU scenario ending...  */
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
#define RX_TPU_SCENARIO_ENDING  6   // execution time of AFTER BDLENA down	   last value : 0
#define TX_TPU_SCENARIO_ENDING  0   // execution time of AFTER BULON down	   last value : 14


/******************************************************/
/* TXPWR configuration...                             */
/* Fixed TXPWR value when GSM management is disabled. */
/******************************************************/
#if ( ANLG_FAM == 11)
  #define FIXED_TXPWR       0x5C // TXPWR=500 for the moment...change later TEMP_FIX
#endif


/************************************/
/* RF  delay (in qbits)             */
/************************************/
#define  DL_DELAY_RF      0   // time spent in the Downlink global RF chain by the modulated signal
#define  UL_DELAY_1RF     0   // time spent in the first  uplink RF block
#define  UL_DELAY_2RF     0   // time spent in the second uplink RF block

#define  UL_ABB_DELAY   12   // 12 for RF output DELAY

#define GUARD_BITS 5

/************************************/
/* TX Propagation delay...          */
/************************************/
#define  PRG_TX       (DL_DELAY_RF + UL_DELAY_2RF + (GUARD_BITS*4) + UL_DELAY_1RF + UL_ABB_DELAY)   // = 50

/************************************/
/* Initial value for APC DELAY     */
/************************************/
#define APCDEL_DOWN     (0)                        // To add this value, the setup delay minimum value: 2	  last value : 0
//SG #define APCDEL_UP       (12)
#define APCDEL_UP       (0)                        //To add this value, the setup delay minimum value: 6
#define C_APCDEL1       (((APCDEL_DOWN & 0x1f)<<5)  |(APCDEL_UP & 0x1f) )
#define C_APCDEL2       (((APCDEL_DOWN & 0x3e0)) |((APCDEL_UP>>5) & 0x1f) )


/************************************/
/* Initial value for AFC...         */
/************************************/
#define  EEPROM_AFC    ((150)*8)    // F14.2 required!!!!! (default : -952*8, initial deviation of -2400 forced)

#if(CHIPSET == 15)
	#define  SETUP_AFC_AND_RF     0
#else
	#define  SETUP_AFC_AND_RF     6       // AFC converges in 2 frames and RF BAND GAP stable after 4 frames
#endif
#define  C_DRP_DCXO_XTAL_DSP_ADDRESS   0x7F80  // DRP DCXO XTAL DSP address

/************************************/
/* DCO Registers Initialization      */
/************************************/
#define C_DCO_SAMPLES_PER_SYMBOL  1
/* The basic frequency table in DSP is approximately 2.115KHz (26MHZ/(512*24))
 * The FCW value needs to intended IF Frequency / 2.115KHZ.
 * Hence since IF is 100KFZ, FCW = 100 / 2.115 ~= 47 */
#define C_DCO_FCW                 47

/***************************************************************
 * Threshold between LIF_100 KHZ (at high power levels) and
 * LIF_120KHz (at low power level)
 * the threshold defintion for GSM and GPRS is as given below
 * ************************************************************ */
#define C_IF_ZERO_LOW_THRESHOLD_GSM   180  // if IL < -90dbm use Zero IF else Low IF STd L1 format
#define C_IF_ZERO_LOW_THRESHOLD_GPRS   180  // if IL < -90dbm use Zero IF else Low IF STd L1 format

/* Since CSF filter coefficients are changed in accordance with GENIE
 * algorithm, below are values programmed in DRP to select HW/SW CSF filter */
#define CSF_CWL_HARDWARE_FILTER_64TAP      0x0100
#define CSF_CWL_PROGRAMMABLE_FILTER_64TAP  0x0000

/* Below are the attenuation losses to be compensated for LIF_100KHZ and LIF_120KHZ Path
 * !!!! Note: The below attenuations are applicable when ROC is enabled  !!!! */
#define SCF_ATTENUATION_LIF_100KHZ  (0) /* value of 0 represents 0dB attenuation as per L1 F7.1 format */
#define SCF_ATTENUATION_LIF_120KHZ  (1) /* value of 1 represents 0.5dB attenuation */
#define SCF_ATTENUATION_ZIF         (0) /* No attenuation when ZIF mode is used */


/************************************/
/* DRP Retiming Related             */
/************************************/
#define C_RETIMING_DISABLED 0x0
#define C_RETIMING_ENABLED  0x1

#define C_RETIMING_CONFIG C_RETIMING_DISABLED

/*************************************/
/*      DRP WRAPPER Initialization   */
/*************************************/
#define C_APCCTRL2             0x05C8   // BGEN=1, APCOFF=64, APC_LDO_EN=0, MODE=0
// SG #define C_APCCTRL2             0x01FC
#define DRP_DBB_RX_IRQ_COUNT   16

#define TTY_L1_STANDALONE 0

/************************************/
/* Triton audio initialize          */
/************************************/
#if (ANLG_FAM == 11)
#if (TTY_L1_STANDALONE == 1)
  #define  C_VULGAIN       0x10 // -12dB
//  #define  C_VULGAIN       0x09 // voice uplink gain set to 7dB
  #define  C_SIDETONE      0x0d // -23dB
//  #define  C_SIDETONE      0x0f // side tone set to MUTE
  #define  C_VDLGAIN       0x4c //
//  #define  C_VDLGAIN       0x2c // set PGA gain to 2dB
  #define  C_CTRL1         0x30 // MicBias = 1, DL Filter Bypass
//  #define  C_CTRL1         0x00 // VSYNC = 1, reset Dig Modulator, VSP, Digital filter
  #define  C_CTRL2         0x10 //
//  #define  C_CTRL2         0x00 // reset value
  #define  C_CTRL3         0x25 // MIC IN (diff), MICIP, MICIN. Amplifier: MIC AMP, Gain = 25.6 dB, SPKAMP = +2.5 dB
  #define  C_CTRL4         0x00 // reset value
  #define  C_CTRL5         0x28 // SAMP FREQ = 48Khz, EARAMP = 1 dB
  #define  C_CTRL6         0x00 // reset value
  #define  C_POPAUTO       0x01 // POPmode set to/Automatic mode
  #define  C_OUTEN1        0x12 // Audio left/Audio Mono, Audio Right/ Audio mono
  #define  C_OUTEN2        0x05 // EAR= voice speech, AUXO = voice speech
  #define  C_OUTEN3        0x01 // SPK = voice speech
  #define  C_AULGA         0x00 // reset value
  #define  C_AURGA         0x00 // reset value
#else
  #define  C_VULGAIN       0x09 // voice uplink gain set to 7dB
  #define  C_SIDETONE      0x06 // side tone set to MUTE
  #define  C_VDLGAIN       0x2c // set PGA gain to 2dB
  #define  C_CTRL1         0x00 // VSYNC = 1, reset Dig Modulator, VSP, Digital filter
  #define  C_CTRL2         0x00 // reset value
  #define  C_CTRL3         0x21 // MIC IN (diff), MICIP, MICIN. Amplifier: MIC AMP, Gain = 25.6 dB, SPKAMP = +2.5 dB
  #define  C_CTRL4         0x00 // reset value
  #define  C_CTRL5         0x28 // SAMP FREQ = 48Khz, EARAMP = 1 dB
  #define  C_CTRL6         0x00 // reset value
  #define  C_POPAUTO       0x01 // POPmode set to/Automatic mode
  #define  C_OUTEN1        0x24 // Audio left/Audio Mono, Audio Right/ Audio mono
  #define  C_OUTEN2        0x01 // EAR= voice speech, AUXO = voice speech
  #define  C_OUTEN3        0x01 // SPK = voice speech
  #define  C_AULGA         0x00 // reset value
  #define  C_AURGA         0x00 // reset value
#endif   //(L1_GTT == 1)
#endif   //(ANLG_FAM == 11)

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
#define  C_Psi_sta_inv    6142L		// 12902L    // (1/C_Psi_sta)
#define  C_Psi_st         9L		// 4L        // C_Psi_sta * 0.8 F0.16
#define  C_Psi_st_32      559386L	// 266313L  // F0.32
#define  C_Psi_st_inv     7678L		// 16128L   // (1/C_Psi_st)

#if (VCXO_ALGO == 1)
// Linearity parameters
  #define  C_AFC_DAC_CENTER     ((4264))	//((-1242)*8)
  #define  C_AFC_DAC_MIN        ((-11128))	//((-2000)*8)
  #define  C_AFC_DAC_MAX        ((19656))	//((1419)*8)
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
#define  SWAP_IQ_GSM    0
#define  SWAP_IQ_DCS    0
#define  SWAP_IQ_PCS    0
#define  SWAP_IQ_GSM850 0   // Swap TX compared to GSM 900

#define LNA_OFF (1)
#define LNA_ON (0)
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
   UWORD16   upper_bound;    //highest physical arfcn of the sub-band
   WORD16    agc_calib;      // AGC  for each TXPWR
   } T_RF_AGC_BAND;

/************************************/
/*       Ramp definitions           */
/************************************/

  typedef struct
  {
    UWORD8 ramp_up     [20];  // Ramp-up profile
    UWORD8 ramp_down   [20];  // Ramp-down profile
  }
  T_TX_RAMP;


// RF structure definition
//========================
#if (L1_FF_MULTIBAND == 0)
// Number of bands supported
#define GSM_BANDS 2

#define MULTI_BAND1     0
#define MULTI_BAND2     1
#else
#endif

// RF table sizes
#define RF_RX_CAL_CHAN_SIZE 10 // number of AGC sub-bands
#define RF_RX_CAL_TEMP_SIZE 11 // number of temperature ranges

#define RF_TX_CHAN_CAL_TABLE_SIZE  4 // channel calibration table size
#define RF_TX_NUM_SUB_BANDS        8 // number of sub-bands in channel calibration table
#define RF_TX_LEVELS_TABLE_SIZE   32 // level table size
#define RF_TX_RAMP_SIZE           16 // number of ramp definitions
#define RF_TX_CAL_TEMP_SIZE        5 // number of temperature ranges

#define AGC_TABLE_SIZE            24
#define MIN_AGC_INDEX              0  // size step for AGC table

#define TEMP_TABLE_SIZE          131 // number of elements in ADC->temp conversion table
#if (REL99 && FF_PRF)
  #define MAX_UPLINK_TIME_SLOT     4 // max number of time slot in uplink
#endif


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
  #if (REL99 && FF_PRF)
    T_TX_LEVEL         levels_power_reduction[MAX_UPLINK_TIME_SLOT];
  #endif
  T_TX_CHAN_CAL  chan_cal_table[RF_TX_CHAN_CAL_TABLE_SIZE][RF_TX_NUM_SUB_BANDS];
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
// TRITON: 5 external channels
enum ADC_INDEX {
  ADC_ADIN1,
  ADC_ADIN2,
  ADC_ADIN3,
  ADC_BATT_TYPE,
  ADC_BTEMP,
  ADC_USBVBUS   ,
  ADC_VBKP,
  ADC_ICHG,
  ADC_VCHG,
  ADC_VBAT,
  ADC_HOTDIE   ,
  ADC_RFTEMP,
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

#if (L1_FF_MULTIBAND == 0)

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
#endif // L1_FF_MULTIBAND == 0

/************************************/
/* ABB (TRITON) Initialization       */
/************************************/

/* ABB_TABLE_SIZE definitions for Triton */
#if (ANLG_FAM == 11)
   #define ABB_TABLE_SIZE 15
#endif

/* ABB enum definitions for TRITON */
#if (ANLG_FAM == 11)
  enum ABB_REGISTERS {
    ABB_VULGAIN = 0,
    ABB_VDLGAIN,
    ABB_SIDETONE,
    ABB_CTRL1,
    ABB_CTRL2,
    ABB_CTRL3,
    ABB_CTRL4,
    ABB_CTRL5,
    ABB_CTRL6,
    ABB_POPAUTO,
    ABB_OUTEN1,
    ABB_OUTEN2,
    ABB_OUTEN3,
    ABB_AULGA,
    ABB_AURGA
  };
#endif

/*************************************/
/*      DRP WRAPPER Table            */
/*************************************/
 #define DRP_WRAPPER_TABLE_SIZE 3

enum DRP_WRAPPER_REGISTERS {
    DRP_WRAPPER_APCCTRL2 = 0,
    DRP_WRAPPER_APCDEL1,
    DRP_WRAPPER_APCDEL2
};

extern T_RF_BAND rf_band[];
#endif //__L1_RF_H

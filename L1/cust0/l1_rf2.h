/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 *
 *        Filename l1_rf2.h
 *  Copyright 2003 (C) Texas Instruments  
 * 
 ************* Revision Controle System Header *************/

#ifndef __L1_RF_H__
#define __L1_RF_H__

/************************************/
/************************************/
//  # define 
/************************************/
/************************************/
/* SYNTHESIZER setup time...        */
/************************************/

#define RX_SYNTH_SETUP_TIME          215L  // Synthesizer setup time in quarter bit.
#define TX_SYNTH_SETUP_TIME          270L  // Synthesizer setup time in quarter bit.

/************************************/
/* time for TPU scenario ending...  */
/************************************/

#define RX_TPU_SCENARIO_ENDING      (4-3)  // execution time of BDLENA down
                                           // minus serialization time
#define TX_TPU_SCENARIO_ENDING      (4-3)  // execution time of BULON down
                                           // minus serialization time

/************************************/
/* TXPWR configuration...           */
/************************************/

#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
  #define FIXED_TXPWR       ((0x1FF << 6) | AUXAPC | FALSE)            // TXPWR=15
//  #define FIXED_TXPWR       ((0xFF << 6) | AUXAPC | FALSE)
#endif

/************************************/
/* TX Propagation delay...          */
/************************************/

#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
//  #define  PRG_TX                ( 52L ) 
  #define  PRG_TX                ( 8L) 
#endif

/************************************/
/*(ANALOG)delay (in qbits)          */
/************************************/

#define  UL_ABB_DELAY   0   // modulator input to output delay

/************************************/
/* Initial value for AFC...         */
/************************************/

#define  EEPROM_AFC    ((-952-2400)*8)        // F13.3 required!!!!! (default : -952*8, initial deviation of -2400 forced)

#define  SETUP_AFC_AND_RF            2        // time to have a stable output of the AFC and RF Band Gap(in Frames)
                                              // !! minimum Value : 1 Frame due to the fact there is no
                                              // hisr() in the first wake-up frame !!!!

#if (ANALOG == 1)
  /************************************/
  /* Omega power on...                */
  /************************************/
  // Omega registers values will be programmed at 1st DSP communication interrupt
  #define  C_DEBUG1          (0x0000                  | FALSE)      // Enable f_tx delay of 400000 cyc DEBUG 
  #define  C_AFCCTLADD      ((0x000 << 6) | AFCCTLADD | TRUE )      // Value at reset
  #define  C_VBUCTRL        ((0x0C9 << 6) | VBUCTRL   | TRUE )      // Uplink gain amp 3 dB, Sidetone gain to -17 dB
  #define  C_VBDCTRL        ((0x006 << 6) | VBDCTRL   | TRUE )      // Downlink gain amp 0dB, Volume control -12 dB
  #define  C_BBCTRL         ((0x000 << 6) | BBCTRL    | TRUE )      // value at reset
  #define  C_APCOFF         ((0x000 << 6) | APCOFF    | TRUE )
  #define  C_BULIOFF        ((0x0FF << 6) | BULIOFF   | TRUE )      // value at reset
  #define  C_BULQOFF        ((0x0FF << 6) | BULQOFF   | TRUE )      // value at reset
  #define  C_DAI_ON_OFF       0x0000                                // value at reset
  #define  C_AUXDAC         ((0x000 << 6) | AUXDAC    | TRUE )      // value at reset
  #define  C_VBCTRL         ((0x00B << 6) | VBCTRL    | TRUE )      // VULSWITCH=0, VDLAUX=1, VDLEAR=1 

  // BULRUDEL will be initialized on rach only ....
  #define  C_APCDEL1        ((0x000 << 6) | APCDEL1   | FALSE) 
#endif

#if (ANALOG == 2)
  /************************************/
  /* Iota power on...                 */
  /************************************/
  // Iota registers values will be programmed at 1st DSP communication interrupt
  #define  C_DEBUG1          (0x0000                  | FALSE)      // Enable f_tx delay of 400000 cyc DEBUG 
  #define  C_AFCCTLADD      ((0x000 << 6) | AFCCTLADD | TRUE )      // Value at reset
  #define  C_VBUCTRL        ((0x0C9 << 6) | VBUCTRL   | TRUE )      // Uplink gain amp 3 dB, Sidetone gain to -17 dB
  #define  C_VBDCTRL        ((0x006 << 6) | VBDCTRL   | TRUE )      // Downlink gain amp 0dB, Volume control -12 dB
  #define  C_BBCTRL         ((0x000 << 6) | BBCTRL    | TRUE )      // value at reset
  #define  C_BULGCAL        ((0x000 << 6) | BULGCAL   | TRUE )      // IAG=0 dB, QAG=0 dB
  #define  C_APCOFF         ((0x000 << 6) | APCOFF    | TRUE )      // value at reset
  #define  C_BULIOFF        ((0x0FF << 6) | BULIOFF   | TRUE )      // value at reset
  #define  C_BULQOFF        ((0x0FF << 6) | BULQOFF   | TRUE )      // value at reset
  #define  C_AUXDAC         ((0x000 << 6) | AUXDAC    | TRUE )      // value at reset
  #define  C_DAI_ON_OFF       0x0000                                // value at reset
  #define  C_VBCTRL1        ((0x00B << 6) | VBCTRL1   | TRUE )      // VULSWITCH=1, VDLAUX=1, VDLEAR=1 
  #define  C_VBCTRL2        ((0x000 << 6) | VBCTRL2   | TRUE )      // MICBIASEL=0, VDLHSO=0, MICAUX=0
  
// BULRUDEL will be initialized on rach only ....
  #define  C_APCDEL1        ((0x000 << 6) | APCDEL1   | TRUE )
  #define  C_APCDEL2        ((0x000 << 6) | APCDEL2   | TRUE )
#endif

#if (ANALOG == 3)
  // SYREN registers values will be programmed at 1st DSP communication interrupt
  #define  C_DEBUG1          (0x0000                  | FALSE)      // Enable f_tx delay of 400000 cyc DEBUG 
  #define  C_AFCCTLADD      ((0x000 << 6) | AFCCTLADD | TRUE )      // Value at reset
  #define  C_VBUCTRL        ((0x0C9 << 6) | VBUCTRL   | TRUE )      // Side tone -17 dB, PGA_UL 3 dB
  #define  C_VBDCTRL        ((0x006 << 6) | VBDCTRL   | TRUE )      // PGA_DL 0dB, Volume -12 dB
  #define  C_BBCTRL         ((0x000 << 6) | BBCTRL    | TRUE )      // Internal autocalibration, Output common mode=1.35V
                                                                    // Monoslot, Vpp=8/15*Vref 
  #define  C_BULGCAL        ((0x000 << 6) | BULGCAL   | TRUE )      // IAG=0 dB, QAG=0 dB
  #define  C_APCOFF         ((0x000 << 6) | APCOFF    | TRUE )      // value at reset
  #define  C_BULIOFF        ((0x0FF << 6) | BULIOFF   | TRUE )      // value at reset
  #define  C_BULQOFF        ((0x0FF << 6) | BULQOFF   | TRUE )      // value at reset
  #define  C_DAI_ON_OFF       0x0000                                // value at reset
  #define  C_AUXDAC         ((0x000 << 6) | AUXDAC    | TRUE )      // value at reset
  #define  C_VBCTRL1        ((0x108 << 6) | VBCTRL1   | TRUE )      // VULSWITCH=1 AUXI 28,2 dB
  #define  C_VBCTRL2        ((0x001 << 6) | VBCTRL2   | TRUE )      // HSMIC on, SPKG gain @ 2,5dB

  // BULRUDEL will be initialized on rach only ....
  #define  C_APCDEL1        ((0x000 << 6) | APCDEL1   | TRUE )
  #define  C_APCDEL2        ((0x000 << 6) | APCDEL2   | TRUE )

  #define  C_VBPOP          ((0x004 << 6) | VBPOP     | TRUE )      // HSOAUTO enabled only
  #define  C_VAUDINITD        2                                     // vaud_init_delay init 2 frames
  #define  C_VAUDCTRL       ((0x000 << 6) | VAUDCTRL  | TRUE )      // Init to zero
  #define  C_VAUOCTRL       ((0x155 << 6) | VAUOCTRL  | TRUE )      // Speech on all outputs
  #define  C_VAUSCTRL       ((0x000 << 6) | VAUSCTRL  | TRUE )      // Init to zero
  #define  C_VAUDPLL        ((0x000 << 6) | VAUDPLL   | TRUE )      // Init to zero

  // SYREN registers values programmed by L1 directly through SPI (ABB_on)

  #define  C_BBCFG            0x44                                  // Syren Like BDLF Filter - DC OFFSET removal OFF

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

#define  C_Psi_sta_inv    9307L    // (1/C_Psi_sta)
#define  C_Psi_st         6        // C_Psi_sta * 0.8 F0.16
#define  C_Psi_st_32      369173L  // F0.32
#define  C_Psi_st_inv     11634L   // (1/C_Psi_st)

typedef struct
{
  WORD16  eeprom_afc;
  UWORD32 psi_sta_inv;
  UWORD32 psi_st;
  UWORD32 psi_st_32;
  UWORD32 psi_st_inv;
}
T_AFC_PARAMS;

/************************************/
/* Swap IQ definitions...           */
/************************************/
/* 0=No Swap, 1=Swap RX only, 2=Swap TX only, 3=Swap RX and TX */

#define  SWAP_IQ_GSM     0
#define  SWAP_IQ_DCS     0
#define  SWAP_IQ_PCS     0  // not supported by rf2   
#define  SWAP_IQ_GSM850  0  // not supported by rf2  

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
  UWORD16 apc;             // 0..31
  UWORD8  ramp_index;      // 0..RF_TX_RAMP_SIZE
  UWORD8  chan_cal_index;  // 0..RF_TX_CHAN_CAL_TABLE_SIZE
} 
T_TX_LEVEL;

/************************************/
/* Automatic Gain Control           */
/************************************/
/* Define structure for sub-band definition of TX Power ******/ 
typedef struct 
   {
   UWORD16  upper_bound;       // highest physical arfcn of the sub-band 
   WORD16    agc_calib;        // AGC  for each TXPWR
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

enum RfRevision {
    RF_IGNORE      = 0x0000,
    RF_SL2         = 0x1000,
    RF_GAIA_20X    = 0x2000,
    RF_GAIA_20A    = 0x2001,
    RF_GAIA_20B    = 0x2002,
    RF_ATLAS_20B   = 0x2020,
    RF_PASCAL_20   = 0x2030
};

// Number of bands supported
#define GSM_BANDS 2

#define MULTI_BAND1               0
#define MULTI_BAND2               1
// RF table sizes
#define RF_RX_CAL_CHAN_SIZE       9  // number of AGC sub-bands
#define RF_RX_CAL_TEMP_SIZE      11  // number of temperature ranges

#define RF_TX_CHAN_CAL_TABLE_SIZE 4  // channel calibration table size 
#define RF_TX_NUM_SUB_BANDS       8  // number of sub-bands in channel calibration table
#define RF_TX_LEVELS_TABLE_SIZE  32  // level table size  
#define RF_TX_RAMP_SIZE          15  // number of ramp definitions
#define RF_TX_CAL_TEMP_SIZE       5  // number of temperature ranges

#define AGC_TABLE_SIZE            1

#define TEMP_TABLE_SIZE         131  // number of elements in ADC->temp conversion table


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
  T_AGC           agc;
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
   ADC_RFTEMP,
   ADC_ADC3,
   ADC_ADC4,
   ADC_INDEX_END // ADC_INDEX_END must be the end of the enums
};

typedef struct
{
    WORD16 converted[ADC_INDEX_END]; // converted
    UWORD16 raw[ADC_INDEX_END];      // raw from ADC
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
  UWORD16   max_carrier;   // max carrier 
  UWORD16   max_txpwr;     // max tx power
}
T_BAND_CONFIG;

typedef struct
{
  UWORD8  band[GSM_BANDS]; // index to band address
  UWORD8  txpwr_tp;        // tx power turning point  
  UWORD16 first_arfcn;     // first index
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
#endif

#if (ANALOG == 3)
  #define ABB_TABLE_SIZE 22
#endif

// Note that this translation is probably not needed at all. But until L1 is
// (maybe) changed to simply initialize the ABB from a table of words, we
// use this to make things more easy-readable.
#if (ANALOG == 1)
  enum ABB_REGISTERS {
    ABB_AFCCTLADD = 0,
    ABB_VBUCTRL,
    ABB_VBDCTRL,
    ABB_BBCTRL,
    ABB_APCOFF,
    ABB_BULIOFF,
    ABB_BULQOFF,
    ABB_DAI_ON_OFF,
    ABB_AUXDAC,
    ABB_VBCTRL,
    ABB_APCDEL1
  };
#elif (ANALOG == 2)
  enum ABB_REGISTERS {
    ABB_AFCCTLADD = 0,
    ABB_VBUCTRL,
    ABB_VBDCTRL,
    ABB_BBCTRL,
    ABB_BULGCAL,
    ABB_APCOFF,
    ABB_BULIOFF,
    ABB_BULQOFF,
    ABB_DAI_ON_OFF,
    ABB_AUXDAC,
    ABB_VBCTRL1,
    ABB_VBCTRL2,
    ABB_APCDEL1,
    ABB_APCDEL2
  };
#elif (ANALOG == 3)
  enum ABB_REGISTERS {
    ABB_AFCCTLADD = 0,
    ABB_VBUCTRL,
    ABB_VBDCTRL,
    ABB_BBCTRL,
    ABB_BULGCAL,
    ABB_APCOFF,
    ABB_BULIOFF,
    ABB_BULQOFF,
    ABB_DAI_ON_OFF,
    ABB_AUXDAC,
    ABB_VBCTRL1,
    ABB_VBCTRL2,
    ABB_APCDEL1,
    ABB_APCDEL2,
    ABB_VBPOP,
    ABB_VAUDINITD,
    ABB_VAUDCTRL,
    ABB_VAUOCTRL,
    ABB_VAUSCTRL,
    ABB_VAUDPLL
  };
#endif
#endif

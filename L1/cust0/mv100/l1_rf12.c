#if (OP_L1_STANDALONE == 1)
  // Define the correct enumeration of PA. Consult tpudrv12.h for the enumeration.
  #if ((BOARD == 40) || (BOARD == 41) || (BOARD == 45)) // EvaRita + D-sample or EvaConso
    #define PA 3
  #else
    #define PA 0
  #endif
#else  
#include "rf.cfg"
//#define PA 3 // Hitachi
#endif

T_RF rf =
{
  RF_RITA_10,           //RF revision
  RF_HW_BAND_SUPPORT,   // radio_band_support E-GSM/DCS + PCS

  { //RX structure
    { //AGC structure
      140,  // low_agc_noise_thr;
      110,  // high_agc_sat_thr;
        6,  // low_agc;
       34,  // high_agc;
      //IL2AGC tables
      {  // below is: il2agc_pwr[121];
        //           il2agc_max[121];
        //           il2agc_av[121];
        // il2agc_pwr
        // Note this is shared between PCN and EGSM.
    14,          /*  EGSM_MAX  IL=0 */
    14,          /*  EGSM_MAX  IL=-1 */
    14,          /*  EGSM_MAX  IL=-2 */
    14,          /*  EGSM_MAX  IL=-3 */
    14,          /*  EGSM_MAX  IL=-4 */
    14,          /*  EGSM_MAX  IL=-5 */
    14,          /*  EGSM_MAX  IL=-6 */
    14,          /*  EGSM_MAX  IL=-7 */
    14,          /*  EGSM_MAX  IL=-8 */
    14,          /*  EGSM_MAX  IL=-9 */
    14,          /*  EGSM_MAX  IL=-10 */
    14,          /*  EGSM_MAX  IL=-11 */
    14,          /*  EGSM_MAX  IL=-12 */
    14,          /*  EGSM_MAX  IL=-13 */
    14,          /*  EGSM_MAX  IL=-14 */
    14,          /*  EGSM_MAX  IL=-15 */
    14,          /*  EGSM_MAX  IL=-16 */
    14,          /*  EGSM_MAX  IL=-17 */
    14,          /*  EGSM_MAX  IL=-18 */
    14,          /*  EGSM_MAX  IL=-19 */
    14,          /*  EGSM_MAX  IL=-20 */
    14,          /*  EGSM_MAX  IL=-21 */
    14,          /*  EGSM_MAX  IL=-22 */
    14,          /*  EGSM_MAX  IL=-23 */
    14,          /*  EGSM_MAX  IL=-24 */
    14,          /*  EGSM_MAX  IL=-25 */
    14,          /*  EGSM_MAX  IL=-26 */
    14,          /*  EGSM_MAX  IL=-27 */
    14,          /*  EGSM_MAX  IL=-28 */
    14,          /*  EGSM_MAX  IL=-29 */
    14,          /*  EGSM_MAX  IL=-30 */
    14,          /*  EGSM_MAX  IL=-31 */
    14,          /*  EGSM_MAX  IL=-32 */
    14,          /*  EGSM_MAX  IL=-33 */
    14,          /*  EGSM_MAX  IL=-34 */
    14,          /*  EGSM_MAX  IL=-35 */
    14,          /*  EGSM_MAX  IL=-36 */
    14,          /*  EGSM_MAX  IL=-37 */
    14,          /*  EGSM_MAX  IL=-38 */
    14,          /*  EGSM_MAX  IL=-39 */
    14,          /*  EGSM_MAX  IL=-40 */
    14,          /*  EGSM_MAX  IL=-41 */
    14,          /*  EGSM_MAX  IL=-42 */
    14,          /*  EGSM_MAX  IL=-43 */
    14,          /*  EGSM_MAX  IL=-44 */
    14,          /*  EGSM_MAX  IL=-45 */
    14,          /*  EGSM_MAX  IL=-46 */
    14,          /*  EGSM_MAX  IL=-47 */
    14,          /*  EGSM_MAX  IL=-48 */
    14,          /*  EGSM_MAX  IL=-49 */
    14,          /*  EGSM_MAX  IL=-50 */
    14,          /*  EGSM_MAX  IL=-51 */
    14,          /*  EGSM_MAX  IL=-52 */
    14,          /*  EGSM_MAX  IL=-53 */
    14,          /*  EGSM_MAX  IL=-54 */
    16,          /*  EGSM_MAX  IL=-55 */
    16,          /*  EGSM_MAX  IL=-56 */
    18,          /*  EGSM_MAX  IL=-57 */
    18,          /*  EGSM_MAX  IL=-58 */
    20,          /*  EGSM_MAX  IL=-59 */
    20,          /*  EGSM_MAX  IL=-60 */
    22,          /*  EGSM_MAX  IL=-61 */
    22,          /*  EGSM_MAX  IL=-62 */
    24,          /*  EGSM_MAX  IL=-63 */
    24,          /*  EGSM_MAX  IL=-64 */
    26,          /*  EGSM_MAX  IL=-65 */
    26,          /*  EGSM_MAX  IL=-66 */
    28,          /*  EGSM_MAX  IL=-67 */
    28,          /*  EGSM_MAX  IL=-68 */
    30,          /*  EGSM_MAX  IL=-69 */
    30,          /*  EGSM_MAX  IL=-70 */
    32,          /*  EGSM_MAX  IL=-71 */
    32,          /*  EGSM_MAX  IL=-72 */
    34,          /*  EGSM_MAX  IL=-73 */
    34,          /*  EGSM_MAX  IL=-74 */
    36,          /*  EGSM_MAX  IL=-75 */
    36,          /*  EGSM_MAX  IL=-76 */
    38,          /*  EGSM_MAX  IL=-77 */
    38,          /*  EGSM_MAX  IL=-78 */
    40,          /*  EGSM_MAX  IL=-79 */
    40,          /*  EGSM_MAX  IL=-80 */
    40,          /*  EGSM_MAX  IL=-81 */
    40,          /*  EGSM_MAX  IL=-82 */
    40,          /*  EGSM_MAX  IL=-83 */
    40,          /*  EGSM_MAX  IL=-84 */
    40,          /*  EGSM_MAX  IL=-85 */
    40,          /*  EGSM_MAX  IL=-86 */
    40,          /*  EGSM_MAX  IL=-87 */
    40,          /*  EGSM_MAX  IL=-88 */
    40,          /*  EGSM_MAX  IL=-89 */
    40,          /*  EGSM_MAX  IL=-90 */
    40,          /*  EGSM_MAX  IL=-91 */
    40,          /*  EGSM_MAX  IL=-92 */
    40,          /*  EGSM_MAX  IL=-93 */
    40,          /*  EGSM_MAX  IL=-94 */
    40,          /*  EGSM_MAX  IL=-95 */
    40,          /*  EGSM_MAX  IL=-96 */
    40,          /*  EGSM_MAX  IL=-97 */
    40,          /*  EGSM_MAX  IL=-98 */
    40,          /*  EGSM_MAX  IL=-99 */
    40,          /*  EGSM_MAX  IL=-100 */
    40,          /*  EGSM_MAX  IL=-101 */
    40,          /*  EGSM_MAX  IL=-102 */
    40,          /*  EGSM_MAX  IL=-103 */
    40,          /*  EGSM_MAX  IL=-104 */
    40,          /*  EGSM_MAX  IL=-105 */
    40,          /*  EGSM_MAX  IL=-106 */
    40,          /*  EGSM_MAX  IL=-107 */
    40,          /*  EGSM_MAX  IL=-108 */
    40,          /*  EGSM_MAX  IL=-109 */
    40,          /*  EGSM_MAX  IL=-110 */
    40,          /*  EGSM_MAX  IL=-111 */
    40,          /*  EGSM_MAX  IL=-112 */
    40,          /*  EGSM_MAX  IL=-113 */
    40,          /*  EGSM_MAX  IL=-114 */
    40,          /*  EGSM_MAX  IL=-115 */
    40,          /*  EGSM_MAX  IL=-116 */
    40,          /*  EGSM_MAX  IL=-117 */
    40,          /*  EGSM_MAX  IL=-118 */
    40,          /*  EGSM_MAX  IL=-119 */
    40           /*  EGSM_MAX  IL=-120 */
      },
      { // il2agc_max
        // Note this is shared between PCN and EGSM.
    14,          /*  EGSM_MAX  IL=0 */
    14,          /*  EGSM_MAX  IL=-1 */
    14,          /*  EGSM_MAX  IL=-2 */
    14,          /*  EGSM_MAX  IL=-3 */
    14,          /*  EGSM_MAX  IL=-4 */
    14,          /*  EGSM_MAX  IL=-5 */
    14,          /*  EGSM_MAX  IL=-6 */
    14,          /*  EGSM_MAX  IL=-7 */
    14,          /*  EGSM_MAX  IL=-8 */
    14,          /*  EGSM_MAX  IL=-9 */
    14,          /*  EGSM_MAX  IL=-10 */
    14,          /*  EGSM_MAX  IL=-11 */
    14,          /*  EGSM_MAX  IL=-12 */
    14,          /*  EGSM_MAX  IL=-13 */
    14,          /*  EGSM_MAX  IL=-14 */
    14,          /*  EGSM_MAX  IL=-15 */
    14,          /*  EGSM_MAX  IL=-16 */
    14,          /*  EGSM_MAX  IL=-17 */
    14,          /*  EGSM_MAX  IL=-18 */
    14,          /*  EGSM_MAX  IL=-19 */
    14,          /*  EGSM_MAX  IL=-20 */
    14,          /*  EGSM_MAX  IL=-21 */
    14,          /*  EGSM_MAX  IL=-22 */
    14,          /*  EGSM_MAX  IL=-23 */
    14,          /*  EGSM_MAX  IL=-24 */
    14,          /*  EGSM_MAX  IL=-25 */
    14,          /*  EGSM_MAX  IL=-26 */
    14,          /*  EGSM_MAX  IL=-27 */
    14,          /*  EGSM_MAX  IL=-28 */
    14,          /*  EGSM_MAX  IL=-29 */
    14,          /*  EGSM_MAX  IL=-30 */
    14,          /*  EGSM_MAX  IL=-31 */
    14,          /*  EGSM_MAX  IL=-32 */
    14,          /*  EGSM_MAX  IL=-33 */
    14,          /*  EGSM_MAX  IL=-34 */
    14,          /*  EGSM_MAX  IL=-35 */
    14,          /*  EGSM_MAX  IL=-36 */
    14,          /*  EGSM_MAX  IL=-37 */
    14,          /*  EGSM_MAX  IL=-38 */
    14,          /*  EGSM_MAX  IL=-39 */
    14,          /*  EGSM_MAX  IL=-40 */
    14,          /*  EGSM_MAX  IL=-41 */
    14,          /*  EGSM_MAX  IL=-42 */
    14,          /*  EGSM_MAX  IL=-43 */
    14,          /*  EGSM_MAX  IL=-44 */
    14,          /*  EGSM_MAX  IL=-45 */
    14,          /*  EGSM_MAX  IL=-46 */
    14,          /*  EGSM_MAX  IL=-47 */
    14,          /*  EGSM_MAX  IL=-48 */
    14,          /*  EGSM_MAX  IL=-49 */
    14,          /*  EGSM_MAX  IL=-50 */
    14,          /*  EGSM_MAX  IL=-51 */
    14,          /*  EGSM_MAX  IL=-52 */
    14,          /*  EGSM_MAX  IL=-53 */
    14,          /*  EGSM_MAX  IL=-54 */
    16,          /*  EGSM_MAX  IL=-55 */
    16,          /*  EGSM_MAX  IL=-56 */
    18,          /*  EGSM_MAX  IL=-57 */
    18,          /*  EGSM_MAX  IL=-58 */
    20,          /*  EGSM_MAX  IL=-59 */
    20,          /*  EGSM_MAX  IL=-60 */
    22,          /*  EGSM_MAX  IL=-61 */
    22,          /*  EGSM_MAX  IL=-62 */
    24,          /*  EGSM_MAX  IL=-63 */
    24,          /*  EGSM_MAX  IL=-64 */
    26,          /*  EGSM_MAX  IL=-65 */
    26,          /*  EGSM_MAX  IL=-66 */
    28,          /*  EGSM_MAX  IL=-67 */
    28,          /*  EGSM_MAX  IL=-68 */
    30,          /*  EGSM_MAX  IL=-69 */
    30,          /*  EGSM_MAX  IL=-70 */
    32,          /*  EGSM_MAX  IL=-71 */
    32,          /*  EGSM_MAX  IL=-72 */
    34,          /*  EGSM_MAX  IL=-73 */
    34,          /*  EGSM_MAX  IL=-74 */
    36,          /*  EGSM_MAX  IL=-75 */
    36,          /*  EGSM_MAX  IL=-76 */
    38,          /*  EGSM_MAX  IL=-77 */
    38,          /*  EGSM_MAX  IL=-78 */
    40,          /*  EGSM_MAX  IL=-79 */
    40,          /*  EGSM_MAX  IL=-80 */
    40,          /*  EGSM_MAX  IL=-81 */
    40,          /*  EGSM_MAX  IL=-82 */
    40,          /*  EGSM_MAX  IL=-83 */
    40,          /*  EGSM_MAX  IL=-84 */
    40,          /*  EGSM_MAX  IL=-85 */
    40,          /*  EGSM_MAX  IL=-86 */
    40,          /*  EGSM_MAX  IL=-87 */
    40,          /*  EGSM_MAX  IL=-88 */
    40,          /*  EGSM_MAX  IL=-89 */
    40,          /*  EGSM_MAX  IL=-90 */
    40,          /*  EGSM_MAX  IL=-91 */
    40,          /*  EGSM_MAX  IL=-92 */
    40,          /*  EGSM_MAX  IL=-93 */
    40,          /*  EGSM_MAX  IL=-94 */
    40,          /*  EGSM_MAX  IL=-95 */
    40,          /*  EGSM_MAX  IL=-96 */
    40,          /*  EGSM_MAX  IL=-97 */
    40,          /*  EGSM_MAX  IL=-98 */
    40,          /*  EGSM_MAX  IL=-99 */
    40,          /*  EGSM_MAX  IL=-100 */
    40,          /*  EGSM_MAX  IL=-101 */
    40,          /*  EGSM_MAX  IL=-102 */
    40,          /*  EGSM_MAX  IL=-103 */
    40,          /*  EGSM_MAX  IL=-104 */
    40,          /*  EGSM_MAX  IL=-105 */
    40,          /*  EGSM_MAX  IL=-106 */
    40,          /*  EGSM_MAX  IL=-107 */
    40,          /*  EGSM_MAX  IL=-108 */
    40,          /*  EGSM_MAX  IL=-109 */
    40,          /*  EGSM_MAX  IL=-110 */
    40,          /*  EGSM_MAX  IL=-111 */
    40,          /*  EGSM_MAX  IL=-112 */
    40,          /*  EGSM_MAX  IL=-113 */
    40,          /*  EGSM_MAX  IL=-114 */
    40,          /*  EGSM_MAX  IL=-115 */
    40,          /*  EGSM_MAX  IL=-116 */
    40,          /*  EGSM_MAX  IL=-117 */
    40,          /*  EGSM_MAX  IL=-118 */
    40,          /*  EGSM_MAX  IL=-119 */
    40           /*  EGSM_MAX  IL=-120 */
        },
        { // il2agc_av
          // Note this is shared between PCN and EGSM.
    14,          /*  EGSM_MAX  IL=0 */
    14,          /*  EGSM_MAX  IL=-1 */
    14,          /*  EGSM_MAX  IL=-2 */
    14,          /*  EGSM_MAX  IL=-3 */
    14,          /*  EGSM_MAX  IL=-4 */
    14,          /*  EGSM_MAX  IL=-5 */
    14,          /*  EGSM_MAX  IL=-6 */
    14,          /*  EGSM_MAX  IL=-7 */
    14,          /*  EGSM_MAX  IL=-8 */
    14,          /*  EGSM_MAX  IL=-9 */
    14,          /*  EGSM_MAX  IL=-10 */
    14,          /*  EGSM_MAX  IL=-11 */
    14,          /*  EGSM_MAX  IL=-12 */
    14,          /*  EGSM_MAX  IL=-13 */
    14,          /*  EGSM_MAX  IL=-14 */
    14,          /*  EGSM_MAX  IL=-15 */
    14,          /*  EGSM_MAX  IL=-16 */
    14,          /*  EGSM_MAX  IL=-17 */
    14,          /*  EGSM_MAX  IL=-18 */
    14,          /*  EGSM_MAX  IL=-19 */
    14,          /*  EGSM_MAX  IL=-20 */
    14,          /*  EGSM_MAX  IL=-21 */
    14,          /*  EGSM_MAX  IL=-22 */
    14,          /*  EGSM_MAX  IL=-23 */
    14,          /*  EGSM_MAX  IL=-24 */
    14,          /*  EGSM_MAX  IL=-25 */
    14,          /*  EGSM_MAX  IL=-26 */
    14,          /*  EGSM_MAX  IL=-27 */
    14,          /*  EGSM_MAX  IL=-28 */
    14,          /*  EGSM_MAX  IL=-29 */
    14,          /*  EGSM_MAX  IL=-30 */
    14,          /*  EGSM_MAX  IL=-31 */
    14,          /*  EGSM_MAX  IL=-32 */
    14,          /*  EGSM_MAX  IL=-33 */
    14,          /*  EGSM_MAX  IL=-34 */
    14,          /*  EGSM_MAX  IL=-35 */
    14,          /*  EGSM_MAX  IL=-36 */
    14,          /*  EGSM_MAX  IL=-37 */
    14,          /*  EGSM_MAX  IL=-38 */
    14,          /*  EGSM_MAX  IL=-39 */
    14,          /*  EGSM_MAX  IL=-40 */
    14,          /*  EGSM_MAX  IL=-41 */
    14,          /*  EGSM_MAX  IL=-42 */
    14,          /*  EGSM_MAX  IL=-43 */
    14,          /*  EGSM_MAX  IL=-44 */
    14,          /*  EGSM_MAX  IL=-45 */
    14,          /*  EGSM_MAX  IL=-46 */
    14,          /*  EGSM_MAX  IL=-47 */
    14,          /*  EGSM_MAX  IL=-48 */
    14,          /*  EGSM_MAX  IL=-49 */
    14,          /*  EGSM_MAX  IL=-50 */
    14,          /*  EGSM_MAX  IL=-51 */
    14,          /*  EGSM_MAX  IL=-52 */
    14,          /*  EGSM_MAX  IL=-53 */
    14,          /*  EGSM_MAX  IL=-54 */
    16,          /*  EGSM_MAX  IL=-55 */
    16,          /*  EGSM_MAX  IL=-56 */
    18,          /*  EGSM_MAX  IL=-57 */
    18,          /*  EGSM_MAX  IL=-58 */
    20,          /*  EGSM_MAX  IL=-59 */
    20,          /*  EGSM_MAX  IL=-60 */
    22,          /*  EGSM_MAX  IL=-61 */
    22,          /*  EGSM_MAX  IL=-62 */
    24,          /*  EGSM_MAX  IL=-63 */
    24,          /*  EGSM_MAX  IL=-64 */
    26,          /*  EGSM_MAX  IL=-65 */
    26,          /*  EGSM_MAX  IL=-66 */
    28,          /*  EGSM_MAX  IL=-67 */
    28,          /*  EGSM_MAX  IL=-68 */
    30,          /*  EGSM_MAX  IL=-69 */
    30,          /*  EGSM_MAX  IL=-70 */
    32,          /*  EGSM_MAX  IL=-71 */
    32,          /*  EGSM_MAX  IL=-72 */
    34,          /*  EGSM_MAX  IL=-73 */
    34,          /*  EGSM_MAX  IL=-74 */
    36,          /*  EGSM_MAX  IL=-75 */
    36,          /*  EGSM_MAX  IL=-76 */
    38,          /*  EGSM_MAX  IL=-77 */
    38,          /*  EGSM_MAX  IL=-78 */
    40,          /*  EGSM_MAX  IL=-79 */
    40,          /*  EGSM_MAX  IL=-80 */
    40,          /*  EGSM_MAX  IL=-81 */
    40,          /*  EGSM_MAX  IL=-82 */
    40,          /*  EGSM_MAX  IL=-83 */
    40,          /*  EGSM_MAX  IL=-84 */
    40,          /*  EGSM_MAX  IL=-85 */
    40,          /*  EGSM_MAX  IL=-86 */
    40,          /*  EGSM_MAX  IL=-87 */
    40,          /*  EGSM_MAX  IL=-88 */
    40,          /*  EGSM_MAX  IL=-89 */
    40,          /*  EGSM_MAX  IL=-90 */
    40,          /*  EGSM_MAX  IL=-91 */
    40,          /*  EGSM_MAX  IL=-92 */
    40,          /*  EGSM_MAX  IL=-93 */
    40,          /*  EGSM_MAX  IL=-94 */
    40,          /*  EGSM_MAX  IL=-95 */
    40,          /*  EGSM_MAX  IL=-96 */
    40,          /*  EGSM_MAX  IL=-97 */
    40,          /*  EGSM_MAX  IL=-98 */
    40,          /*  EGSM_MAX  IL=-99 */
    40,          /*  EGSM_MAX  IL=-100 */
    40,          /*  EGSM_MAX  IL=-101 */
    40,          /*  EGSM_MAX  IL=-102 */
    40,          /*  EGSM_MAX  IL=-103 */
    40,          /*  EGSM_MAX  IL=-104 */
    40,          /*  EGSM_MAX  IL=-105 */
    40,          /*  EGSM_MAX  IL=-106 */
    40,          /*  EGSM_MAX  IL=-107 */
    40,          /*  EGSM_MAX  IL=-108 */
    40,          /*  EGSM_MAX  IL=-109 */
    40,          /*  EGSM_MAX  IL=-110 */
    40,          /*  EGSM_MAX  IL=-111 */
    40,          /*  EGSM_MAX  IL=-112 */
    40,          /*  EGSM_MAX  IL=-113 */
    40,          /*  EGSM_MAX  IL=-114 */
    40,          /*  EGSM_MAX  IL=-115 */
    40,          /*  EGSM_MAX  IL=-116 */
    40,          /*  EGSM_MAX  IL=-117 */
    40,          /*  EGSM_MAX  IL=-118 */
    40,          /*  EGSM_MAX  IL=-119 */
    40           /*  EGSM_MAX  IL=-120 */
      }
    },
  },
  {
    {0, 0},     // ramp up and down delays
    GUARD_BITS, // number of guard bits needed for ramp up
    PRG_TX      // propagation delay PRG_TX
  },
  { //AFC parameters
    EEPROM_AFC,
    C_Psi_sta_inv,     // (1/C_Psi_sta)
    C_Psi_st,          // C_Psi_sta * 0.8 F0.16
    C_Psi_st_32,       // F0.32
    C_Psi_st_inv       // (1/C_Psi_st)

#if (VCXO_ALGO==1)
     ,C_AFC_DAC_CENTER,      // VCXO startup parameter - best guess
      C_AFC_DAC_MIN,         // VCXO startup parameter - 15ppm
      C_AFC_DAC_MAX,         // VCXO startup parameter + 15ppm
      C_AFC_SNR_THR         // snr - Default threshold value
#endif
  }
};

T_RF_BAND rf_band[GSM_BANDS]; //uninitialised rf struct for bands

const T_RF_BAND rf_900 =
{
  { //RX structure
     //T_RX_CAL_PARAMS rx_cal_params
    {
        193,      //g_magic
         40,      //lna_gain_max * 2
         40,      //lna_th_high
         44       //lna_th_low
    },
    { //T_RF_AGC_BAND   agc_bands[RF_RX_CAL_CHAN_SIZE];
     // Remark: ARFCN=0 (GSM-E) is maintained by 1st GSM subbband.
                     // upper_bound, agc_calib
      {  10,  0},    // sub-band1 up to arfcn =  10, Agc calibration = 0db
      {  30,  0},    // sub-band2 up to arfcn =  30, Agc calibration = 0db
      {  51,  0},    // sub-band3 up to arfcn =  51, Agc calibration = 0db
      {  71,  0},    // etc.
      {  90,  0},    //
      { 112,  0},    //
      { 124,  0},    //
      { 991,  0},    //
      { 992,  0},    //
      {1023,  0},    //
    },
    { //RX temperature compensation
      { -15 ,  0  },
      {  -5 ,  0  },
      {   6 ,  0  },
      {  16 ,  0  },
      {  25 ,  0  },
      {  35 ,  0  },
      {  45 ,  0  },
      {  56 ,  0  },
      {  66 ,  0  },
      {  75 ,  0  },
      { 100 ,  0  }
    }
  },
  { //TX structure
    {// gsm900 T_LEVEL_TX
#if (PA == 3)  // Hitachi
    {550,  0, 0}, // 0
    {550,  0, 0}, // 1
    {550,  0, 0}, // 2
    {550,  0, 0}, // 3
    {550,  0, 0}, // 4
    {560,  0, 0}, // 5 Highest power
    {510,  1, 0}, // 6
    {460,  2, 0}, // 7
    {400,  3, 1}, // 8
    {325,  4, 1}, // 9
    {280,  5, 1}, // 10
    {230,  6, 1}, // 11
    {195,  7, 1}, // 12
    {175,  8, 2}, // 13
    {158,  9, 2},  // 14
    {140, 10, 2}, // 15
    {130, 11, 2}, // 16
    {120, 12, 3}, // 17
    {115, 13, 3}, // 18
    {110, 14, 3}, // 19 Lowest power
    { 43, 14, 0}, // 20
    { 43, 14, 0}, // 21
    { 43, 14, 0}, // 22
    { 43, 14, 0}, // 23
    { 43, 14, 0}, // 24
    { 43, 14, 0}, // 25
    { 43, 14, 0}, // 26
    { 43, 14, 0}, // 27
    { 43, 14, 0}, // 28
    { 43, 14, 0}, // 29
    { 43, 14, 0}, // 30
    { 43, 14, 0}, // 31
#else
      { 673, 0, 0  }, // 0
      { 673, 0, 0  }, // 1
      { 673, 0, 0  }, // 2
      { 673, 0, 0  }, // 3
      { 673, 0, 0  }, // 4
      { 673, 0, 0  }, // 5 Highest power
      { 478, 1, 0  }, // 6
      { 389, 2, 0  }, // 7
      { 318, 3, 0  }, // 8
      { 260, 4, 0  }, // 9
      { 217, 5, 0  }, // 10
      { 180, 6, 0  }, // 11
      { 151, 7, 0  }, // 12
      { 128, 8, 0  }, // 13
      { 109, 9, 0  }, // 14
      { 94, 10, 0  }, // 15
      { 86, 11, 0  }, // 16
      { 78, 12, 0  }, // 17
      { 72, 13, 0  }, // 18
      { 67, 14, 0  }, // 19 Lowest power
      { 67, 14, 0  }, // 20
      { 67, 14, 0  }, // 21
      { 67, 14, 0  }, // 22
      { 67, 14, 0  }, // 23
      { 67, 14, 0  }, // 24
      { 67, 14, 0  }, // 25
      { 67, 14, 0  }, // 26
      { 67, 14, 0  }, // 27
      { 67, 14, 0  }, // 28
      { 67, 14, 0  }, // 29
      { 67, 14, 0  }, // 30
      { 67, 14, 0  }, // 31
#endif
    },
    {// Channel Calibration Tables
      {// arfcn, tx_chan_cal
#if (PA == 3)  // Hitachi
        {   21, 128 }, // Calibration Table 0
        {   41, 128 },
        {   62, 128 },
        {   82, 128 },
        {  103, 128 },
        {  124, 129 },
        {  885, 128 },
        { 1023, 128 }
        },
#else
        {   40, 128 }, // Calibration Table 0
        {   80, 128 },
        {  124, 128 },
        {  586, 128 },
        {  661, 128 },
        {  736, 128 },
        {  885, 128 },
        { 1023, 128 }
        },
#endif
      {// arfcn, tx_chan_cal
        {   21, 128 }, // Calibration Table 1
        {   41, 128 },
        {   62, 128 },
        {   82, 128 },
        {  103, 128 },
        {  124, 128 },
        {  885, 128 },
        { 1023, 128 }
        },
      {// arfcn, tx_chan_cal
        {   21, 128 }, // Calibration Table 2
        {   41, 128 },
        {   62, 128 },
        {   82, 128 },
        {  103, 128 },
        {  124, 128 },
        {  885, 128 },
        { 1023, 128 }
        },
      {// arfcn, tx_chan_cal
        {   21, 128 }, // Calibration Table 3
        {   41, 128 },
        {   62, 128 },
        {   82, 128 },
        {  103, 128 },
        {  124, 128 },
        {  885, 128 },
        { 1023, 128 }
      }
    },
    { // GSM Power Ramp Values
#if (PA == 3)  // Hitachi
     {
      {// Ramp-Up      #0 profile - Power Level 5
        0,0,6,0,11,7,1,0,0,11,0,26,23,22,16,5 
      },
      {// Ramp-Down    #0 profile
        0,5,7,16,26,24,30,6,0,14,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #1 profile - Power Level 6
        0,4,8,0,0,0,25,0,0,0,0,17,30,19,25,0
      },
      {// Ramp-Down    #1 profile
        0,7,19,24,28,25,23,2,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #2 profile - Power Level 7
        0,4,8,0,0,0,25,0,0,0,0,17,30,19,25,0
      },
      {// Ramp-Down    #2 profile
        0,7,19,24,28,25,23,2,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #3 profile - Power Level 8
        5,9,11,3,0,4,16,0,1,0,0,7,18,24,12,18
      },
      {// Ramp-Down    #3 profile
        0,9,16,23,23,21,18,9,3,2,0,0,0,2,1,1
      },
     },
     {
      {// Ramp-Up      #4 profile - Power Level 9
        5,0,18,17,0,8,0,0,0,3,0,14,21,21,15,6
      },
      {// Ramp-Down    #4 profile
        0,8,12,31,24,20,19,7,4,2,1,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #5 profile - Power Level 10
        5,10,11,20,0,7,0,0,14,0,0,0,15,17,22,7
      },
      {// Ramp-Down    #5 profile
        0,7,17,24,27,20,18,11,4,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #6 profile - Power Level 11
        0,11,1,8,30,0,0,0,0,28,0,1,14,14,12,9
      },
      {// Ramp-Down    #6 profile
        0,7,16,19,27,26,19,7,4,2,1,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #7 profile - Power Level 12
        0,0,12,0,2,0,21,26,0,0,0,24,9,3,20,11
      },
      {// Ramp-Down    #7 profile
        0,6,17,21,28,23,19,7,4,2,1,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #8 profile - Power Level 13
        5,0,0,26,31,16,0,0,0,0,0,0,0,31,13,6
      },
      {// Ramp-Down    #8 profile
        0,15,14,20,22,24,19,9,2,2,1,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #9 profile - Power Level 14
        5,10,9,0,4,3,10,10,23,2,7,13,4,12,11,5
      },
      {// Ramp-Down    #9 profile
        0,4,19,28,24,20,19,7,4,2,1,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #10 profile - Power Level 15
        5,0,10,9,4,4,12,13,14,15,19,13,7,2,1,0
      },
      {// Ramp-Down    #10 profile
        0,3,18,29,25,20,19,7,4,2,1,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #11 profile - Power Level 16
        5,10,9,6,14,7,13,11,13,10,15,5,4,2,4,0
      },
      {// Ramp-Down    #11 profile
        0,0,13,24,31,27,19,7,4,2,1,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #12 profile - Power Level 17
        5,10,9,11,15,6,13,9,19,31,0,0,0,0,0,0
      },
      {// Ramp-Down    #12 profile
        0,0,4,31,31,29,19,7,4,2,1,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #13 profile - Power Level 18
        5,10,9,9,6,27,31,31,0,0,0,0,0,0,0,0
      },
      {// Ramp-Down    #13 profile
        0,0,4,31,29,25,25,7,4,2,1,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #14 profile - Power Level 19
        5,10,20,31,31,31,0,0,0,0,0,0,0,0,0,0
      },
      {// Ramp-Down    #14 profile
        0,0,2,27,31,30,24,7,4,2,1,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #15 profile - Power Level 19
        5,10,20,31,31,31,0,0,0,0,0,0,0,0,0,0
      },
      {// Ramp-Down    #15 profile
        0,0,2,27,31,30,24,7,4,2,1,0,0,0,0,0
      },
     },
    },
#else
{ { 0,0,0,17,0,0,0,0,0,11,31,31,31,7,0,0  },        // Ramp-Up       #0 profile - Power Level 5
{   0,18,31,31,31,17,0,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,6,17,0,0,0,0,30,31,25,9,10,0  },        // Ramp-Up       #0 profile - Power Level 6
{   0,19,23,26,31,15,0,14,0,0,0,0,0,0,0,0  }, },    // Ramp-Down  #0 profile
{ { 0,0,0,0,25,0,0,0,0,10,31,31,31,0,0,0  },        // Ramp-Up        #0 profile - Power Level 7
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down     # profile
{ { 0,0,0,0,10,17,0,0,0,7,31,31,31,1,0,0  },        // Ramp-Up       #0 profile - Power Level 8
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 0,0,0,0,28,0,0,0,0,7,31,31,31,0,0,0  },         // Ramp-Up       #0 profile - Power Level 9
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 0,0,0,0,11,0,31,0,0,0,12,31,31,12,0,0  },      // Ramp-Up       #0 profile - Power Level 10
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 0,0,0,31,0,0,18,0,0,9,23,30,17,0,0,0  },         // Ramp-Up       #0 profile - Power Level 11
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },       // Ramp-Down   #0 profile
{ { 0,0,31,0,0,0,19,0,0,0,19,31,23,5,0,0  },         // Ramp-Up      #0 profile - Power Level 12
{   0,31,31,31,21,14,0,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down    #0 profile
{ { 0,0,0,0,0,0,31,25,0,0,14,9,31,18,0,0  },        // Ramp-Up       #0 profile - Power Level 13
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down     #0 profile
{ { 0,0,4,31,0,0,0,0,31,31,0,22,9,0,0,0  },         // Ramp-Up        #0 profile - Power Level 14
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down     #0 profile
{ { 0,0,0,0,31,5,0,11,31,31,0,19,0,0,0,0  },        // Ramp-Up      #0 profile - Power Level 15
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down     #0 profile
{ { 0,0,0,0,0,0,0,31,31,31,31,4,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 16
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },    // Ramp-Down    #0 profile
{ { 0,0,0,0,0,4,31,31,31,31,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 17
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },    // Ramp-Down    #0 profile
{ { 0,0,0,0,0,31,31,31,31,4,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 18
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },    // Ramp-Down    #0 profile
{ { 0,0,0,0,4,31,31,31,31,0,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 19
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  } } },    // Ramp-Down    #0 profile
#endif
    { //TX temperature compensation
      #if (ORDER2_TX_TEMP_CAL==1)
      { -11,  0,  0,  0 },
      {  +9,  0,  0,  0 },
      { +39,  0,  0,  0 },
      { +59,  0,  0,  0 },
      { 127,  0,  0,  0 }
      #else
      { -11,  0 },
      {  +9,  0 },
      { +39,  0 },
      { +59,  0 },
      { 127,  0 }
      #endif
    },
  },
  //IQ swap
  SWAP_IQ_GSM,
};

const T_RF_BAND rf_1800 =
{
  { //RX structure
    { //T_RX_CAL_PARAMS rx_cal_params
      188,      //g_magic
       40,      //lna gain * 2
       40,      //lna_th_high
       44       //lna_th_low
    },
    { //T_RF_AGC_BAND   agc_bands[RF_RX_CAL_CHAN_SIZE];
     /*--------------*/
     /*-- DCS band --*/
     /*--------------*/
      { 548,  0},     //
      { 622,  0},     //
      { 680,  0},     //
      { 745,  0},     //
      { 812,  0},     //
      { 860,  0},     //
      { 885,  0},     //
      { 991,  0},     //
      { 992,  0},     //
      {1023,  0},     //
    },
    { //RX temperature compensation
      { -15 ,  0  },
      {  -5 ,  0  },
      {   6 ,  0  },
      {  16 ,  0  },
      {  25 ,  0  },
      {  35 ,  0  },
      {  45 ,  0  },
      {  56 ,  0  },
      {  66 ,  0  },
      {  75 ,  0  },
      { 100 ,  0  }
    }
  },
  { //TX structure
    {// dcs1800 T_LEVEL_TX
#if (PA == 3)  // Hitachi
    {720, 0, 0}, // 0 Highest power
    {637, 1, 0}, // 1
    {570, 2, 0}, // 2
    {470, 3, 1}, // 3
    {390, 4, 1}, // 4
    {328, 5, 1}, // 5
    {277, 6, 1}, // 6
    {238, 7, 1}, // 7
    {205, 8, 2}, // 8
    {178, 9, 2}, // 9
    {158, 10, 2}, // 10
    {140, 11, 2}, // 11
    {133, 12, 2}, // 12
    {125, 13, 3}, // 13
    {118, 14, 3}, // 14
    {114, 15, 3}, // 15 Lowest power
    {61, 15, 0}, // 16
    {61, 15, 0}, // 17
    {61, 15, 0}, // 18
    {61, 15, 0}, // 19
    {61, 15, 0}, // 20
    {61, 15, 0}, // 21
    {61, 15, 0}, // 22
    {61, 15, 0}, // 23    {61, 15, 0}, // 24
    {61, 15, 0}, // 25
    {61, 15, 0}, // 26
    {61, 15, 0}, // 27
    {61, 15, 0}, // 28
    {750, 0, 0}, // 29 Highest power
    {750, 0, 0}, // 30 Highest power
    {750, 0, 0}, // 31 Highest power
#else
      { 918, 0, 0  }, // 0 Highest power
      { 616, 1, 0  }, // 1
      { 500, 2, 0  }, // 2
      { 411, 3, 0  }, // 3
      { 339, 4, 0  }, // 4
      { 280, 5, 0  }, // 5
      { 231, 6, 0  }, // 6
      { 194, 7, 0  }, // 7
      { 165, 8, 0  }, // 8
      { 143, 9, 0  }, // 9
      { 122, 10, 0  }, // 10
      { 109, 11, 0  }, // 11
      { 96, 12, 0  }, // 12
      { 85, 13, 0  }, // 13
      { 80, 14, 0  }, // 14
      { 75, 15, 0  }, // 15 Lowest power
      { 75, 15, 0  }, // 16
      { 75, 15, 0  }, // 17
      { 75, 15, 0  }, // 18
      { 75, 15, 0  }, // 19
      { 75, 15, 0  }, // 20
      { 75, 15, 0  }, // 21
      { 75, 15, 0  }, // 22
      { 75, 15, 0  }, // 23
      { 75, 15, 0  }, // 24
      { 75, 15, 0  }, // 25
      { 75, 15, 0  }, // 26
      { 75, 15, 0  }, // 27
      { 75, 15, 0  }, // 28
      { 754, 0, 0  }, // 29
      { 754, 0, 0  }, // 30
      { 754, 0, 0  }, // 31
#endif
    },
    {// Channel Calibration Tables
      {// arfcn, tx_chan_cal
        {  554, 126 }, // Calibration Table 0
        {  722, 128 },
        {  746, 128 },
        {  774, 128 },
        {  808, 128 },
        {  851, 134 },
        {  870, 134 },
        {  885, 136 }
      },
      {
        {  554, 128 }, // Calibration Table 1
        {  722, 128 },
        {  746, 128 },
        {  774, 128 },
        {  808, 128 },
        {  851, 128 },
        {  870, 128 },
        {  885, 128 }
      },
      {// arfcn, tx_chan_cal
        {  554, 128 }, // Calibration Table 2
        {  722, 128 },
        {  746, 128 },
        {  774, 128 },
        {  808, 128 },
        {  851, 128 },
        {  870, 128 },
        {  885, 128 }
      },
      {// arfcn, tx_chan_cal
        {  554, 128 }, // Calibration Table 3
        {  722, 128 },
        {  746, 128 },
        {  774, 128 },
        {  808, 128 },
        {  851, 128 },
        {  870, 128 },
        {  885, 128 }
      }
    },
     { // DCS Power Ramp Values
#if (PA == 3)  // Hitachi
      {
      {// Ramp-Up      #0 profile - Power Level 0
        0,0,0,10,16,0,0,0,6,0,0,0,19,31,31,15
      },
      {// Ramp-Down    #0 profile
        6,13,28,26,22,19,6,2,6,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #1 profile - Power Level 1
        0,0,0,0,12,2,0,12,0,0,0,11,24,24,31,12
      },
      {// Ramp-Down    #1 profile
        6,16,23,28,22,19,6,2,6,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #2 profile - Power Level 2
        0,0,0,6,0,8,0,15,0,2,0,10,22,27,16,22
      },
      {// Ramp-Down    #2 profile
        4,7,26,29,31,23,2,0,6,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #3 profile - Power Level 3
        0,0,0,18,0,0,0,16,0,0,0,10,29,31,22,2
      },
      {// Ramp-Down    #3 profile
        8,12,19,20,23,20,14,6,4,2,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #4 profile - Power Level 4
        0,0,0,16,0,3,2,1,23,0,0,8,23,31,19,2
      },
      {// Ramp-Down    #4 profile
        0,3,19,24,31,12,20,15,4,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #5 profile - Power Level 5
        0,0,0,21,0,0,0,1,31,0,0,0,17,30,20,8
      },
      {// Ramp-Down    #5 profile
        3,14,26,31,20,17,6,11,0,0,0,0,0,0,0,0,
      },
     },
     {
      {// Ramp-Up      #6 profile - Power Level 6
        0,0,0,15,0,0,11,2,24,6,9,0,19,31,10,1
      },
      {// Ramp-Down    #6 profile
        5,20,31,23,6,17,26,0,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #7 profile - Power Level 7
        0,0,0,15,0,0,11,2,18,0,22,0,7,0,31,22
      },
      {// Ramp-Down    #7 profile
        0,5,20,31,23,6,17,26,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #8 profile - Power Level 8
        1,0,0,14,0,11,0,29,0,0,9,14,13,6,27,4
      },
      {// Ramp-Down    #8 profile
        2,19,27,25,31,16,8,0,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #9 profile - Power Level 9
        0,0,0,22,0,0,20,20,7,11,0,15,5,0,28,0
      },
      {// Ramp-Down    #9 profile
        0,2,23,31,31,31,0,0,10,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #10 profile - Power Level 10
        0,0,0,22,0,0,20,20,7,11,0,15,5,0,28,0
      },
      {// Ramp-Down    #10 profile
        0,0,25,31,31,31,0,0,10,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #11 profile - Power Level 11
       0,0,22,0,0,20,20,7,11,0,15,5,0,28,0,0
      },
      {// Ramp-Down    #11 profile
       0,0,20,31,31,31,5,0,10,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #12 profile - Power Level 12
        0,0,0,4,31,31,4,11,0,11,2,17,6,11,0,0
      },
      {// Ramp-Down    #12 profile
        0,0,27,30,31,31,9,0,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #13 profile - Power Level 13
        0,4,31,31,4,11,0,11,2,17,6,11,0,0,0,0
      },
      {// Ramp-Down    #13 profile
        0,0,0,27,30,31,31,9,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #14 profile - Power Level 14
        0,0,0,0,31,31,31,19,16,0,0,0,0,0,0,0

      },
      {// Ramp-Down    #14 profile
        0,0,2,31,31,31,31,2,0,0,0,0,0,0,0,0

      },
     },
     {
      {// Ramp-Up      #15 profile - Power Level 15
        0,0,0,0,31,31,31,19,16,0,0,0,0,0,0,0
      },
      {// Ramp-Down    #15 profile
        0,0,2,31,31,31,31,2,0,0,0,0,0,0,0,0
      },
     },
    },
#else
{ { 0,0,0,19,0,0,0,0,0,8,31,31,31,8,0,0  },           // Ramp-Up       #0 profile - Power Level 0
{   12,19,23,24,28,15,0,0,7,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,19,0,0,0,4,0,0,8,31,31,31,4,0,0  },           // Ramp-Up       #0 profile - Power Level 1
{   0,11,27,30,29,31,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 0,0,0,26,0,0,0,0,0,22,23,26,21,10,0,0  },        // Ramp-Up       #0 profile - Power Level 2
{   0,31,27,28,24,18,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 0,0,0,0,27,0,0,0,0,0,19,30,31,21,0,0  },         // Ramp-Up       #0 profile - Power Level 3
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },       // Ramp-Down   #0 profile
{ { 0,0,0,30,0,0,0,0,0,13,31,30,24,0,0,0  },        // Ramp-Up       #0 profile - Power Level 4
{   0,18,30,31,31,18,0,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,6,31,0,0,0,0,6,31,30,24,0,0,0  },         // Ramp-Up       #0 profile - Power Level 5
{   0,13,30,31,31,23,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 31,0,0,0,7,0,0,11,0,0,26,30,23,0,0,0  },       // Ramp-Up       #0 profile - Power Level 6
{   0,13,30,31,31,23,0,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,31,0,0,0,0,22,0,0,0,21,30,24,0,0,0  },        // Ramp-Up       #0 profile - Power Level 7
{   0,13,30,31,31,23,0,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,0,9,31,31,0,0,0,26,31,0,0,0  },        // Ramp-Up       #0 profile - Power Level 8
{   0,14,31,31,31,21,0,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,8,31,31,0,0,0,0,27,31,0,0,0  },        // Ramp-Up       #0 profile - Power Level 9
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,5,31,0,0,0,28,14,0,0,0,28,22,0,0,0  },        // Ramp-Up       #0 profile - Power Level 10
{   0,15,31,31,29,22,0,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,0,0,16,27,31,31,23,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 11
{   0,0,15,31,31,27,24,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,0,16,27,31,31,23,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 12
{   0,0,14,31,31,27,25,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,16,31,25,31,25,0,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 13
{   0,0,17,30,31,25,25,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,0,31,31,31,31,4,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 14
{   0,0,18,31,22,30,27,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,31,31,31,31,4,0,0,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 15
{   0,0,23,31,31,31,12,0,0,0,0,0,0,0,0,0  }, }, },     // Ramp-Down   #0 profile
#endif
    { //TX temperature compensation
      #if (ORDER2_TX_TEMP_CAL==1)
      { -11,  0,  0,  0 },
      {  +9,  0,  0,  0 },
      { +39,  0,  0,  0 },
      { +59,  0,  0,  0 },
      { 127,  0,  0,  0 }
      #else
      { -11,  0 },
      {  +9,  0 },
      { +39,  0 },
      { +59,  0 },
      { 127,  0 }
      #endif
    },
  },
  //IQ swap
  SWAP_IQ_DCS
};

//copy from gsm900
const T_RF_BAND rf_850 =
{
  { //RX structure
     //T_RX_CAL_PARAMS rx_cal_params
    {
#if ((BOARD == 40) || (BOARD == 41) || (BOARD == 45)) // EvaRita + D-sample or EvaConso
        193,      //g_magic
#else
        181,      //g_magic
#endif
         40,      //lna_gain_max * 2
         40,      //lna_th_high
         44       //lna_th_low
    },
    { //T_RF_AGC_BAND   agc_bands[RF_RX_CAL_CHAN_SIZE];
     // Remark: ARFCN=0 (GSM-E) is maintained by 1st GSM subbband.
                     // upper_bound, agc_calib
      {  10,  0},    // sub-band1 up to arfcn =  10, Agc calibration = 0db
      {  30,  0},    // sub-band2 up to arfcn =  30, Agc calibration = 0db
      {  51,  0},    // sub-band3 up to arfcn =  51, Agc calibration = 0db
      {  71,  0},    // etc.
      {  90,  0},    //
      { 112,  0},    //
      { 124,  0},    //
      { 991,  0},    //
      { 992,  0},    //
      {1023,  0},    //
    },
    { //RX temperature compensation
      { -15 ,  0  },
      {  -5 ,  0  },
      {   6 ,  0  },
      {  16 ,  0  },
      {  25 ,  0  },
      {  35 ,  0  },
      {  45 ,  0  },
      {  56 ,  0  },
      {  66 ,  0  },
      {  75 ,  0  },
      { 100 ,  0  }
    }
  },
  { //TX structure
    {// gsm850 T_LEVEL_TX
#if (PA == 3)  // Hitachi
      {560, 0, 0}, // 0
      {560, 0, 0}, // 1
      {560, 0, 0}, // 2
      {560, 0, 0}, // 3
      {560, 0, 0}, // 4
      {560, 0, 0}, // 5 Highest power
      {437, 1, 0}, // 6
      {355, 2, 0}, // 7
      {291, 3, 0}, // 8
      {237, 4, 0}, // 9
      {194, 5, 0}, // 10
      {160, 6, 0}, // 11
      {132, 7, 0}, // 12
      {110, 8, 0}, // 13
      {92, 9, 0}, // 14
      {77, 10, 0}, // 15
      {67, 11, 0}, // 16
      {59, 12, 0}, // 17
      {52, 13, 0}, // 18
      {46, 14, 0}, // 19 Lowest power
      {46, 14, 0}, // 20
      {46, 14, 0}, // 21
      {46, 14, 0}, // 22
      {46, 14, 0}, // 23
      {46, 14, 0}, // 24
      {46, 14, 0}, // 25
      {46, 14, 0}, // 26
      {46, 14, 0}, // 27
      {46, 14, 0}, // 28
      {46, 14, 0}, // 29
      {46, 14, 0}, // 30
      {46, 14, 0}, // 31
#else
      { 616,  0,  0 }, // 0
      { 616,  0,  0 }, // 1
      { 616,  0,  0 }, // 2
      { 616,  0,  0 }, // 3
      { 616,  0,  0 }, // 4
      { 616,  0,  0 }, // 5 Highest pwr
      { 453,  1,  0 }, // 6
      { 371,  2,  0 }, // 7
      { 309,  3,  0 }, // 8
      { 256,  3,  0 }, // 9
      { 216,  5,  0 }, // 10
      { 182,  6,  0 }, // 11
      { 155,  7,  0 }, // 12
      { 134,  8,  0 }, // 13
      { 116,  9,  0 }, // 14
      { 103, 10,  0 }, // 15
      { 89, 11,  0 }, // 16
      { 87, 12,  0 }, // 17
      { 80, 13,  0 }, // 18
      { 75, 14,  0 }, // 19  lowest pwr
      { 75, 14,  0 }, // 20
      { 75, 14,  0 }, // 21
      { 75, 14,  0 }, // 22
      { 75, 14,  0 }, // 23
      { 75, 14,  0 }, // 24
      { 75, 14,  0 }, // 25
      { 75, 14,  0 }, // 26
      { 75, 14,  0 }, // 27
      { 75, 14,  0 }, // 28
      { 75, 14,  0 }, // 29
      { 75, 14,  0 }, // 30
      { 75, 14,  0 }  // 31
#endif
    },
    {// Channel Calibration Tables
      {// arfcn, tx_chan_cal
        {   40, 128 }, // Calibration Table 0
        {   80, 128 },
        {  124, 128 },
        {  586, 128 },
        {  661, 128 },
        {  736, 128 },
        {  885, 128 },
        { 1023, 128 }
        },
      {// arfcn, tx_chan_cal
        {  554, 130 }, // Calibration Table 1
        {  722, 128 },
        {  746, 129 },
        {  774, 131 },
        {  808, 132 },
        {  851, 134 },
        {  870, 138 },
        { 885, 140 }
      },
      {// arfcn, tx_chan_cal
        {   40, 128 }, // Calibration Table 2
        {   80, 128 },
        {  124, 128 },
        {  586, 128 },
        {  661, 128 },
        {  736, 128 },
        {  885, 128 },
        { 1023, 128 }
      },
      {// arfcn, tx_chan_cal
        {   40, 128 }, // Calibration Table 3
        {   80, 128 },
        {  124, 128 },
        {  586, 128 },
        {  661, 128 },
        {  736, 128 },
        {  885, 128 },
        { 1023, 128 }
      }
    },
    { // gsm850 Power Ramp Values
#if (PA == 3)  // Hitachi
      {
        {// Ramp-Up      #0 profile - Power Level 5
          8,0,0,0,0,0,6,0,
          0,6,18,29,23,21,17,0
        },
        {// Ramp-Down    #0 profile
          0,12,19,31,31,18,17,0,
          0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #1 profile - Power Level 6
          0,0,3,3,1,4,0,3,
          6,5,15,31,31,9,12,5
        },
        {// Ramp-Down    #1 profile
          9,23,25,31,25,15,0,0,
          0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #2 profile - Power Level 7
          1,1,4,0,4,2,5,2,
          4,2,12,19,31,18,15,8
        },
        {// Ramp-Down    #2 profile
          9,30,31,31,16,11,0,0,
          0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #3 profile - Power Level 8
          2,1,3,2,4,5,4,4,
          5,10,11,5,15,20,22,15
        },
        {// Ramp-Down    #3 profile
          8,12,16,31,31,9,7,6,
          3,3,2,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #4 profile - Power Level 9
          0,4,3,1,0,4,0,2,
          10,13,0,0,31,31,26,3
        },
        {// Ramp-Down    #4 profile
          8,9,28,31,31,7,5,2,
          7,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #5 profile - Power Level 10
          0,0,0,5,0,3,4,6,
          18,11,1,0,31,31,15,3
        },
        {// Ramp-Down    #5 profile
          7,11,31,31,31,17,0,0,
          0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #6 profile - Power Level 11
          0,0,7,4,3,5,1,6,
          2,17,5,1,15,27,25,10
        },
        {// Ramp-Down    #6 profile
          6,14,27,31,29,7,4,3,
          7,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #7 profile - Power Level 12
          0,2,5,9,1,4,5,6,
          9,14,8,1,31,19,10,4
        },
        {// Ramp-Down    #7 profile
          9,5,31,31,31,5,5,6,
          0,3,2,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #8 profile - Power Level 13
          0,0,0,12,4,9,9,13,
          11,10,9,3,18,12,11,7
        },
        {// Ramp-Down    #8 profile
          8,9,28,31,27,8,5,4,
          8,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #9 profile - Power Level 14
          0,0,0,6,11,11,10,8,
          7,5,5,5,19,29,12,0
        },
        {// Ramp-Down    #9 profile
          8,18,31,31,31,9,0,0,
          0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #10 profile - Power Level 15
          0,0,0,0,0,0,9,31,
          31,31,12,5,2,0,3,4
        },
        {// Ramp-Down    #10 profile
          4,18,31,31,31,13,0,0,
          0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #11 profile - Power Level 16
          0,0,0,0,0,0,27,31,
          31,31,3,0,1,2,0,2
        },
        {// Ramp-Down    #11 profile
          3,9,30,31,31,24,0,0,
          0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #12 profile - Power Level 17
          0,0,0,0,0,5,31,31,
          30,31,0,0,0,0,0,0
        },
        {// Ramp-Down    #12 profile
          6,8,31,31,19,20,12,1,
          0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #13 profile - Power Level 18
          0,0,0,0,18,17,31,31,
          17,14,0,0,0,0,0,0
        },
        {// Ramp-Down    #13 profile
          3,7,29,31,31,27,0,0,
          0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #14 profile - Power Level 19
          0,0,0,9,12,15,30,31,          17,14,0,0,0,0,0,0
        },
        {// Ramp-Down    #14 profile
          0,0,7,20,26,31,31,13,
          0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #15 profile - Power Level 19
          0,0,0,9,12,15,30,31,
          17,14,0,0,0,0,0,0
        },
        {// Ramp-Down    #15 profile
          0,0,7,20,26,31,31,13,
          0,0,0,0,0,0,0,0
        },
      },
    },
#else
{ { 0,0,0,25,0,0,7,0,0,18,31,31,16,0,0,0  },        // Ramp-Up       #0 profile - Power Level 5
{   10,14,21,22,31,20,0,0,0,10,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,29,0,0,2,0,0,31,31,31,4,0,0  },        // Ramp-Up       #0 profile - Power Level 6
{   0,9,31,31,31,26,0,0,0,0,0,0,0,0,0,0  }, },    // Ramp-Down  #0 profile
{ { 0,0,0,0,0,0,31,6,0,0,29,31,31,0,0,0  },        // Ramp-Up        #0 profile - Power Level 7
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down     # profile
{ { 0,0,0,10,0,25,0,0,0,11,31,31,20,0,0,0  },        // Ramp-Up       #0 profile - Power Level 8
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 0,0,0,0,0,0,0,0,0,22,0,0,13,31,31,31  },         // Ramp-Up       #0 profile - Power Level 9
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 0,31,0,0,0,0,0,0,25,0,23,25,24,0,0,0  },      // Ramp-Up       #0 profile - Power Level 10
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 0,0,30,0,0,0,0,0,0,31,0,16,31,20,0,0  },         // Ramp-Up       #0 profile - Power Level 11
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },       // Ramp-Down   #0 profile
{ { 0,0,0,31,0,0,30,0,0,0,15,31,10,11,0,0  },         // Ramp-Up      #0 profile - Power Level 12
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down    #0 profile
{ { 0,0,31,0,0,13,0,31,0,0,13,18,22,0,0,0  },        // Ramp-Up       #0 profile - Power Level 13
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down     #0 profile
{ { 0,0,0,31,0,0,0,0,11,31,31,0,24,0,0,0  },         // Ramp-Up        #0 profile - Power Level 14
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down     #0 profile
{ { 15,0,0,0,0,0,0,20,31,31,31,0,0,0,0,0  },        // Ramp-Up      #0 profile - Power Level 15
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down     #0 profile
{ { 0,0,0,0,0,4,31,31,31,31,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 16
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },    // Ramp-Down    #0 profile
{ { 0,0,0,0,4,31,31,31,31,0,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 17
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },    // Ramp-Down    #0 profile
{ { 0,0,0,0,17,31,31,31,18,0,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 18
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },    // Ramp-Down    #0 profile
{ { 0,0,0,0,31,31,31,31,4,0,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 19
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  } } },    // Ramp-Down    #0 profile
#endif
    { //TX temperature compensation
      #if (ORDER2_TX_TEMP_CAL==1)
      { -11,  0,  0,  0 },
      {  +9,  0,  0,  0 },
      { +39,  0,  0,  0 },
      { +59,  0,  0,  0 },
      { 127,  0,  0,  0 }
      #else
      { -11,  0 },
      {  +9,  0 },
      { +39,  0 },
      { +59,  0 },
      { 127,  0 }
      #endif
    },
  },
  //IQ swap
  SWAP_IQ_GSM850,
};

//copy from dcs1800
const T_RF_BAND rf_1900 =
{
  { //RX structure
    { //T_RX_CAL_PARAMS rx_cal_params
      188,      //g_magic
       40,      //lna gain * 2
       40,      //lna_th_high
       44       //lna_th_low
    },
    { //T_RF_AGC_BAND   agc_bands[RF_RX_CAL_CHAN_SIZE];
     /*--------------*/
     /*-- PCS band --*/
     /*--------------*/
      { 548,  0},     //
      { 622,  0},     //
      { 680,  0},     //
      { 745,  0},     //
      { 812,  0},     //
      { 860,  0},     //
      { 885,  0},     //
      { 991,  0},    //
      { 992,  0},    //
      {1023,  0},    //
    },
    { //RX temperature compensation
      { -15 ,  0  },
      {  -5 ,  0  },
      {   6 ,  0  },
      {  16 ,  0  },
      {  25 ,  0  },
      {  35 ,  0  },
      {  45 ,  0  },
      {  56 ,  0  },
      {  66 ,  0  },
      {  75 ,  0  },
      { 100 ,  0  }
    }
  },
  { //TX structure
    {// pcs1900 T_LEVEL_TX
#if (PA == 3)  // Hitachi
      {915, 0, 0}, // 0 Highest power
      {715, 1, 0}, // 1
      {570, 2, 0}, // 2
      {465, 3, 0}, // 3
      {390, 4, 0}, // 4
      {320, 5, 0}, // 5
      {265, 6, 0}, // 6
      {220, 7, 0}, // 7
      {183, 8, 0}, // 8
      {155, 9, 0}, // 9
      {129, 10, 0}, // 10
      {111, 11, 0}, // 11
      {93, 12, 0}, // 12
      {80, 13, 0}, // 13
      {72, 14, 0}, // 14
      {62, 15, 0}, // 15 Lowest power
      {62, 15, 0}, // 16
      {62, 15, 0}, // 17
      {62, 15, 0}, // 18
      {62, 15, 0}, // 19
      {62, 15, 0}, // 20
      {62, 15, 0}, // 21
      {62, 15, 0}, // 22
      {62, 15, 0}, // 23
      {62, 15, 0}, // 24
      {62, 15, 0}, // 25
      {62, 15, 0}, // 26
      {62, 15, 0}, // 27
      {62, 15, 0}, // 28
      {915, 0, 0}, // 29 Highest power
      {915, 0, 0}, // 30 Highest power
      {915, 0, 0}, // 31 Highest power
#else
      { 949, 0, 0  }, // 0 Highest power
      { 615, 1, 0  }, // 1
      { 499, 2, 0  }, // 2
      { 404, 3, 0  }, // 3
      { 328, 4, 0  }, // 4
      { 270, 5, 0  }, // 5
      { 224, 6, 0  }, // 6
      { 187, 7, 0  }, // 7
      { 158, 8, 0  }, // 8
      { 137, 9, 0  }, // 9
      { 119, 10, 0  }, // 10
      { 105, 11, 0  }, // 11
      { 92, 12, 0  }, // 12
      { 81, 13, 0  }, // 13
      { 75, 14, 0  }, // 14
      { 70, 15, 0  }, // 15 Lowest power
      { 70, 15, 0  }, // 16
      { 70, 15, 0  }, // 17
      { 70, 15, 0  }, // 18
      { 70, 15, 0  }, // 19
      { 70, 15, 0  }, // 20
      { 70, 15, 0  }, // 21
      { 70, 15, 0  }, // 22
      { 70, 15, 0  }, // 23
      { 70, 15, 0  }, // 24
      { 70, 15, 0  }, // 25
      { 70, 15, 0  }, // 26
      { 70, 15, 0  }, // 27
      { 70, 15, 0  }, // 28
      { 754, 0, 0  }, // 29 Highest power
      { 754, 0, 0  }, // 30 Highest power
      { 754, 0, 0  }, // 31 Highest power
#endif
    },
    {// Channel Calibration Tables
      {// arfcn, tx_chan_cal
        {  554, 128 }, // Calibration Table 0
        {  722, 128 },
        {  746, 128 },
        {  774, 128 },
        {  808, 128 },
        {  810, 128 },
        {  810, 128 },
        {  810, 128 }
      },
      {
        {  554, 128 }, // Calibration Table 1
        {  722, 128 },
        {  746, 128 },
        {  774, 128 },
        {  808, 128 },
        {  810, 128 },
        {  810, 128 },
        {  810, 128 }
      },
      {// arfcn, tx_chan_cal
        {  554, 128 }, // Calibration Table 2
        {  722, 128 },
        {  746, 128 },
        {  774, 128 },
        {  808, 128 },
        {  810, 128 },
        {  810, 128 },
        {  810, 128 }
      },
      {// arfcn, tx_chan_cal
        {  554, 128 }, // Calibration Table 3
        {  722, 128 },
        {  746, 128 },
        {  774, 128 },
        {  808, 128 },
        {  810, 128 },
        {  810, 128 },
        {  810, 128 }
      }
    },
    { // PCS Power Ramp Values
#if (PA == 3)  // Hitachi
     {
	    {// Ramp-Up      #0 profile - Power Level 0
          0,0,0,0,6,2,0,1,
          5,4,12,31,31,25,10,1
      },
      {// Ramp-Down    #0 profile
        8,11,18,31,31,17,12,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #1 profile - Power Level 1
        0,0,0,0,7,6,1,3,
        4,0,2,15,31,31,24,4
      },
      {// Ramp-Down    #1 profile
        8,25,31,19,19,20,6,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #2 profile - Power Level 2
        0,0,0,0,8,6,0,2,
        4,6,3,17,31,31,18,2
      },
      {// Ramp-Down    #2 profile
        4,10,31,29,31,23,0,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #3 profile - Power Level 3
        0,0,0,0,3,4,10,4,
        2,0,2,13,31,31,26,2
      },
      {// Ramp-Down    #3 profile
        6,24,22,20,27,20,9,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #4 profile - Power Level 4
        0,0,0,8,0,6,7,9,
        2,0,7,7,31,31,19,1
      },
      {// Ramp-Down    #4 profile
        3,14,28,31,31,12,9,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #5 profile - Power Level 5
        0,0,0,1,12,0,4,4,
        4,9,6,12,31,27,17,1
      },
      {// Ramp-Down    #5 profile
        3,18,31,31,11,26,4,4,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #6 profile - Power Level 6
        0,0,0,3,8,7,2,7,
        1,4,22,5,29,26,12,2
      },
      {// Ramp-Down    #6 profile
        4,20,21,31,31,21,0,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #7 profile - Power Level 7
        0,0,0,4,8,2,7,7,
        5,7,6,6,31,31,14,0
      },
      {// Ramp-Down    #7 profile
        3,13,31,31,31,19,0,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #8 profile - Power Level 8
        0,0,0,6,2,8,3,5,
        16,3,9,25,6,31,14,0
      },
      {// Ramp-Down    #8 profile
        5,13,29,31,31,19,0,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #9 profile - Power Level 9
        0,0,0,7,2,0,8,12,
        17,3,31,9,3,27,8,1
      },
      {// Ramp-Down    #9 profile
        1,17,22,31,31,26,0,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #10 profile - Power Level 10
        0,0,0,6,2,3,6,8,
        12,31,14,18,15,11,2,0
      },
      {// Ramp-Down    #10 profile
        3,8,27,21,31,31,7,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #11 profile - Power Level 11
        0,0,0,3,3,4,2,28,
        12,31,31,7,3,3,1,0
      },
      {// Ramp-Down    #11 profile
        3,12,26,20,31,31,5,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #12 profile - Power Level 12
        0,0,0,1,4,9,31,30,
        26,20,7,0,0,0,0,0
      },
      {// Ramp-Down    #12 profile
        2,4,29,31,31,31,0,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #13 profile - Power Level 13
        0,0,0,0,8,30,28,31,
        16,11,4,0,0,0,0,0
      },
      {// Ramp-Down    #13 profile
        2,4,31,31,31,29,0,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #14 profile - Power Level 14
        0,0,0,6,26,28,30,25,
        13,0,0,0,0,0,0,0
      },
      {// Ramp-Down    #14 profile
        0,6,18,31,31,31,11,0,
        0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #15 profile - Power Level 15
        0,0,24,22,21,20,21,14,
        6,0,0,0,0,0,0,0
      },
      {// Ramp-Down    #15 profile
        0,0,22,31,24,30,21,0,
        0,0,0,0,0,0,0,0
      },
     },
    },
#else
{ { 0,0,13,0,0,0,5,0,0,13,24,31,31,11,0,0  },           // Ramp-Up       #0 profile - Power Level 0
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,21,0,0,0,0,0,11,30,31,25,10,0,0  },           // Ramp-Up       #0 profile - Power Level 1
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 0,0,0,0,6,20,0,0,0,0,31,31,30,10,0,0  },        // Ramp-Up       #0 profile - Power Level 2
{   0,31,27,28,24,18,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 0,0,0,30,0,0,0,0,0,0,16,30,31,21,0,0  },         // Ramp-Up       #0 profile - Power Level 3
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },       // Ramp-Down   #0 profile
{ { 0,0,0,0,31,0,0,0,0,12,31,30,24,0,0,0  },        // Ramp-Up       #0 profile - Power Level 4
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,31,0,0,0,0,0,0,31,31,31,4,0,0  },         // Ramp-Up       #0 profile - Power Level 5
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },      // Ramp-Down   #0 profile
{ { 0,0,0,6,31,0,0,0,0,0,25,31,31,4,0,0  },       // Ramp-Up       #0 profile - Power Level 6
{   0,31,31,31,31,4,0,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,19,31,0,0,0,0,12,31,31,4,0,0  },        // Ramp-Up       #0 profile - Power Level 7
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,20,31,0,0,0,0,0,11,31,31,4,0,0  },        // Ramp-Up       #0 profile - Power Level 8
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,21,31,0,0,0,0,0,14,31,31,0,0,0  },        // Ramp-Up       #0 profile - Power Level 9
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,27,31,0,0,0,16,18,0,31,5,0,0  },        // Ramp-Up       #0 profile - Power Level 10
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,31,0,0,0,31,31,0,31,4,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 11
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,0,16,27,31,31,23,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 12
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,16,31,25,31,25,0,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 13
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,31,31,31,31,4,0,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 14
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, },     // Ramp-Down   #0 profile
{ { 0,0,0,0,31,31,31,31,4,0,0,0,0,0,0,0  },        // Ramp-Up       #0 profile - Power Level 15
{   0,0,31,31,31,31,4,0,0,0,0,0,0,0,0,0  }, }, },     // Ramp-Down   #0 profile
#endif
    { //TX temperature compensation
      #if (ORDER2_TX_TEMP_CAL==1)
      { -11,  0,  0,  0 },
      {  +9,  0,  0,  0 },
      { +39,  0,  0,  0 },
      { +59,  0,  0,  0 },
      { 127,  0,  0,  0 }
      #else
      { -11,  0 },
      {  +9,  0 },
      { +39,  0 },
      { +59,  0 },
      { 127,  0 }
    #endif
    },
  },
  //IQ swap
  SWAP_IQ_PCS
};

/*------------------------------------------*/
/* ABB Initialization words
/*------------------------------------------*/
#if (ANALOG == 1)
  UWORD16 abb[ABB_TABLE_SIZE] =
  {
    C_AFCCTLADD,  // Value at reset
    C_VBUR,       // Uplink gain amp 0dB, Sidetone gain to mute
    C_VBDR,       // Downlink gain amp 0dB, Volume control 0 dB
    C_BBCTL,      // value at reset
    C_APCOFF,     // value at reset
    C_BULIOFF,    // value at reset
    C_BULQOFF,    // value at reset
    C_DAI_ON_OFF, // value at reset
    C_AUXDAC,     // value at reset
    C_VBCR,       // VULSWITCH=0, VDLAUX=1, VDLEAR=1
    C_APCDEL      // value at reset
};
#elif (ANALOG == 2)
  UWORD16 abb[ABB_TABLE_SIZE] =
  {
    C_AFCCTLADD,
    C_VBUR,
    C_VBDR,
    C_BBCTL,
    C_BULGCAL,
    C_APCOFF,
    C_BULIOFF,
    C_BULQOFF,
    C_DAI_ON_OFF,
    C_AUXDAC,
    C_VBCR,
    C_VBCR2,
    C_APCDEL,
    C_APCDEL2
  };

#elif (ANALOG == 3)
  UWORD16 abb[ABB_TABLE_SIZE] =
  {
    C_AFCCTLADD,
    C_VBUR,
    C_VBDR,
    C_BBCTL,
    C_BULGCAL,
    C_APCOFF,
    C_BULIOFF,
    C_BULQOFF,
    C_DAI_ON_OFF,
    C_AUXDAC,
    C_VBCR,
    C_VBCR2,
    C_APCDEL,
    C_APCDEL2,
    C_VBPOP,
    C_VAUDINITD,
    C_VAUDCR,
    C_VAUOCR,
    C_VAUSCR,
    C_VAUDPLL
  };

#endif

/*------------------------------------------*/
/*             Gain table                   */
/*  specified in the TRF6053 spec           */
/*     2 dB steps - LNA always ON       */
/*------------------------------------------*/
UWORD16 AGC_TABLE[AGC_TABLE_SIZE] =
{
  0x00,  //reserved
  0x01,  //reserved
  0x02,  //reserved
  0x03,  //reserved
  0x04,  //reserved
  0x05,  //reserved
  0x06,  //14 dB
  0x07,  //16
  0x08,  //18
  0x09,  //20
  0x0a,  //22
  0x0b,  //24
  0x0c,  //26
  0x0d,  //28
  0x0e,  //30
  0x0f,  //32
  0x10,  //34
  0x11,  //36
  0x12,  //38
  0x13,  //40
  /*
  0x14,  //reserved
  0x15,  //reserved
  0x16,  //reserved
  0x17,  //reserved
  0x18,  //reserved
  0x19,  //reserved
  0x1a,  //reserved
  0x1b,  //reserved
  0x1c,  //reserved
  0x1d,  //reserved
  0x1e,  //reserved
  0x1f,  //reserved
  */
};

// structure for ADC conversion (4 Internal channel + 5 Ext channels max.)
T_ADC adc;

// MADC calibration structure
T_ADCCAL adc_cal=
{ // a: 0,..,8
  // b, 0,..,8
  // cal_a = 4*1750 is the Typical value 1.75 V ref voltage , divide by 4
  7000, 8750, 7000, 7000, 7000, 7000, 7000, 256, 7000,
     0,    0,    0,    0,    0,    0,   0,    0,    0
};

#if (BOARD == 41)
// table which converts ADC value into RF temperature
T_TEMP temperature[TEMP_TABLE_SIZE] =
{
// Temperature compensation for EVARITA - S.Glock, J.Demay 04/23/2003
  582, -40,
  640, -10,
  698, 25,
  756, 60,
  815, 90
};
#else
// table which converts ADC value into RF temperature
T_TEMP temperature[TEMP_TABLE_SIZE] =
{
  7, -35,
  7, -34,
  8, -33,
  8, -32,
  9, -31,
  9, -30,
  10, -29,
  11, -28,
  11, -27,
  12, -26,
  13, -25,
  14, -24,
  14, -23,
  15, -22,
  16, -21,
  17, -20,
  18, -19,
  19, -18,
  21, -17,
  22, -16,
  23, -15,
  24, -14,
  26, -13,
  27, -12,
  29, -11,
  30, -10,
  32, -9,
  34, -8,
  36, -7,
  37, -6,
  39, -5,
  41, -4,
  44, -3,
  46, -2,
  48, -1,
  51, 0,
  53, 1,
  56, 2,
  59, 3,
  61, 4,
  64, 5,
  68, 6,
  71, 7,
  74, 8,
  78, 9,
  81, 10,
  85, 11,
  89, 12,
  93, 13,
  97, 14,
  101, 15,
  105, 16,
  110, 17,
  115, 18,
  119, 19,
  124, 20,
  130, 21,
  135, 22,
  140, 23,
  146, 24,
  152, 25,
  158, 26,
  164, 27,
  170, 28,
  176, 29,
  183, 30,
  190, 31,
  197, 32,
  204, 33,
  211, 34,
  219, 35,
  226, 36,
  234, 37,
  242, 38,
  250, 39,
  259, 40,
  267, 41,
  276, 42,
  285, 43,
  294, 44,
  303, 45,
  312, 46,
  322, 47,
  331, 48,
  341, 49,
  351, 50,
  361, 51,
  371, 52,
  382, 53,
  392, 54,
  403, 55,
  413, 56,
  424, 57,
  435, 58,
  446, 59,
  458, 60,
  469, 61,
  480, 62,
  492, 63,
  503, 64,
  515, 65,
  527, 66,
  539, 67,
  550, 68,
  562, 69,
  574, 70,
  586, 71,
  598, 72,
  611, 73,
  623, 74,
  635, 75,
  647, 76,
  659, 77,
  671, 78,
  683, 79,
  696, 80,
  708, 81,
  720, 82,
  732, 83,
  744, 84,
  756, 85,
  768, 86,
  780, 87,
  792, 88,
  804, 89,
  816, 90,
  827, 91,
  839, 92,
  851, 93,
  862, 94,
  873, 95
};
#endif

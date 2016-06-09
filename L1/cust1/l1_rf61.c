/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 *
 *        Filename l1_rf61.c
 *        Version  1.0
 *        Date     June 12th, 2005
 *
 ************* Revision Controle System Header *************/

T_RF rf =
{
  RF_LOCOSTO,          //RF revision
  RF_HW_BAND_SUPPORT,   // radio_band_support E-GSM/DCS + PCS

  { //RX structure
    { //AGC structure
      140,  // low_agc_noise_thr;
      110,  // high_agc_sat_thr;
        0,  // low_agc;
       23,  // high_agc;
      //IL2AGC tables
      {  // below is: il2agc_pwr[121];
          //           il2agc_max[121];
          //           il2agc_av[121];
          // il2agc_pwr
          // Note this is shared between PCN and EGSM.
          0,          /*  EGSM_MAX  IL=0 */
          0,          /*  EGSM_MAX  IL=-1 */
          0,          /*  EGSM_MAX  IL=-2 */
          0,          /*  EGSM_MAX  IL=-3 */
          0,          /*  EGSM_MAX  IL=-4 */
          0,          /*  EGSM_MAX  IL=-5 */
          0,          /*  EGSM_MAX  IL=-6 */
          0,          /*  EGSM_MAX  IL=-7 */
          0,          /*  EGSM_MAX  IL=-8 */
          0,          /*  EGSM_MAX  IL=-9 */
          0,          /*  EGSM_MAX  IL=-10 */
          0,          /*  EGSM_MAX  IL=-11 */
          0,          /*  EGSM_MAX  IL=-12 */
          0,          /*  EGSM_MAX  IL=-13 */
          0,          /*  EGSM_MAX  IL=-14 */
          0,          /*  EGSM_MAX  IL=-15 */
          0,          /*  EGSM_MAX  IL=-16 */
          0,          /*  EGSM_MAX  IL=-17 */
          0,          /*  EGSM_MAX  IL=-18 */
          0,          /*  EGSM_MAX  IL=-19 */
          0,          /*  EGSM_MAX  IL=-20 */
          0,          /*  EGSM_MAX  IL=-21 */
          0,          /*  EGSM_MAX  IL=-22 */
          0,          /*  EGSM_MAX  IL=-23 */
          0,          /*  EGSM_MAX  IL=-24 */
          0,          /*  EGSM_MAX  IL=-25 */
          0,          /*  EGSM_MAX  IL=-26 */
          0,          /*  EGSM_MAX  IL=-27 */
          0,          /*  EGSM_MAX  IL=-28 */
          0,          /*  EGSM_MAX  IL=-29 */
          0,          /*  EGSM_MAX  IL=-30 */
          0,          /*  EGSM_MAX  IL=-31 */
          0,          /*  EGSM_MAX  IL=-32 */
          0,          /*  EGSM_MAX  IL=-33 */
          0,          /*  EGSM_MAX  IL=-34 */
          0,          /*  EGSM_MAX  IL=-35 */
          0,          /*  EGSM_MAX  IL=-36 */
          0,          /*  EGSM_MAX  IL=-37 */
          0,          /*  EGSM_MAX  IL=-38 */
          0,          /*  EGSM_MAX  IL=-39 */
          0,          /*  EGSM_MAX  IL=-40 */
          0,          /*  EGSM_MAX  IL=-41 */
          0,          /*  EGSM_MAX  IL=-42 */
          5,          /*  EGSM_MAX  IL=-43 */
          5,          /*  EGSM_MAX  IL=-44 */
          0,          /*  EGSM_MAX  IL=-45 */
          0,          /*  EGSM_MAX  IL=-46 */
          0,          /*  EGSM_MAX  IL=-47 */
          0,          /*  EGSM_MAX  IL=-48 */
          0,          /*  EGSM_MAX  IL=-49 */
          0,          /*  EGSM_MAX  IL=-50 */
          0,          /*  EGSM_MAX  IL=-51 */
          0,          /*  EGSM_MAX  IL=-52 */
          0,          /*  EGSM_MAX  IL=-53 */
          0,          /*  EGSM_MAX  IL=-54 */
          0,          /*  EGSM_MAX  IL=-55 */
          0,          /*  EGSM_MAX  IL=-56 */
          0,          /*  EGSM_MAX  IL=-57 */
          0,          /*  EGSM_MAX  IL=-58 */
          0,          /*  EGSM_MAX  IL=-59 */
          0,          /*  EGSM_MAX  IL=-60 */
          0,          /*  EGSM_MAX  IL=-61 */
          0,          /*  EGSM_MAX  IL=-62 */
          0,          /*  EGSM_MAX  IL=-63 */
          0,          /*  EGSM_MAX  IL=-64 */
          0,          /*  EGSM_MAX  IL=-65 */
          0,          /*  EGSM_MAX  IL=-66 */
          0,          /*  EGSM_MAX  IL=-67 */
          0,          /*  EGSM_MAX  IL=-68 */
          0,          /*  EGSM_MAX  IL=-69 */
          0,          /*  EGSM_MAX  IL=-70 */
          0,          /*  EGSM_MAX  IL=-71 */
          0,          /*  EGSM_MAX  IL=-72 */
          0,          /*  EGSM_MAX  IL=-73 */
          0,          /*  EGSM_MAX  IL=-74 */
          5,          /*  EGSM_MAX  IL=-75 */
          5,          /*  EGSM_MAX  IL=-76 */
          8,          /*  EGSM_MAX  IL=-77 */
          8,          /*  EGSM_MAX  IL=-78 */
          11,          /*  EGSM_MAX  IL=-79 */
          11,          /*  EGSM_MAX  IL=-80 */
          11,          /*  EGSM_MAX  IL=-81 */
          11,          /*  EGSM_MAX  IL=-82 */
          11,          /*  EGSM_MAX  IL=-83 */
          11,          /*  EGSM_MAX  IL=-84 */
          14,          /*  EGSM_MAX  IL=-85 */
          14,          /*  EGSM_MAX  IL=-86 */
          14,          /*  EGSM_MAX  IL=-87 */
          14,          /*  EGSM_MAX  IL=-88 */
          14,          /*  EGSM_MAX  IL=-89 */
          14,          /*  EGSM_MAX  IL=-90 */
          14,          /*  EGSM_MAX  IL=-91 */
          14,          /*  EGSM_MAX  IL=-92 */
          14,          /*  EGSM_MAX  IL=-93 */
          14,          /*  EGSM_MAX  IL=-94 */
          17,          /*  EGSM_MAX  IL=-95 */
          17,          /*  EGSM_MAX  IL=-96 */
          20,          /*  EGSM_MAX  IL=-97 */
          20,          /*  EGSM_MAX  IL=-98 */
          20,          /*  EGSM_MAX  IL=-99 */
          20,          /*  EGSM_MAX  IL=-100 */
          23,          /*  EGSM_MAX  IL=-101 */
          23,          /*  EGSM_MAX  IL=-102 */
          23,          /*  EGSM_MAX  IL=-103 */
          23,          /*  EGSM_MAX  IL=-104 */
          23,          /*  EGSM_MAX  IL=-105 */
          23,          /*  EGSM_MAX  IL=-106 */
          23,          /*  EGSM_MAX  IL=-107 */
          23,          /*  EGSM_MAX  IL=-108 */
          23,          /*  EGSM_MAX  IL=-109 */
          23,          /*  EGSM_MAX  IL=-110 */
          23,          /*  EGSM_MAX  IL=-111 */
          23,          /*  EGSM_MAX  IL=-112 */
          23,          /*  EGSM_MAX  IL=-113 */
          23,          /*  EGSM_MAX  IL=-114 */
          23,          /*  EGSM_MAX  IL=-115 */
          23,          /*  EGSM_MAX  IL=-116 */
          23,          /*  EGSM_MAX  IL=-117 */
          23,          /*  EGSM_MAX  IL=-118 */
          23,          /*  EGSM_MAX  IL=-119 */
          23           /*  EGSM_MAX  IL=-120 */
      },
      { // il2agc_max
          // Note this is shared between PCN and EGSM.
          0,          /*  EGSM_MAX  IL=0 */
          0,          /*  EGSM_MAX  IL=-1 */
          0,          /*  EGSM_MAX  IL=-2 */
          0,          /*  EGSM_MAX  IL=-3 */
          0,          /*  EGSM_MAX  IL=-4 */
          0,          /*  EGSM_MAX  IL=-5 */
          0,          /*  EGSM_MAX  IL=-6 */
          0,          /*  EGSM_MAX  IL=-7 */
          0,          /*  EGSM_MAX  IL=-8 */
          0,          /*  EGSM_MAX  IL=-9 */
          0,          /*  EGSM_MAX  IL=-10 */
          0,          /*  EGSM_MAX  IL=-11 */
          0,          /*  EGSM_MAX  IL=-12 */
          0,          /*  EGSM_MAX  IL=-13 */
          0,          /*  EGSM_MAX  IL=-14 */
          0,          /*  EGSM_MAX  IL=-15 */
          0,          /*  EGSM_MAX  IL=-16 */
          0,          /*  EGSM_MAX  IL=-17 */
          0,          /*  EGSM_MAX  IL=-18 */
          0,          /*  EGSM_MAX  IL=-19 */
          0,          /*  EGSM_MAX  IL=-20 */
          0,          /*  EGSM_MAX  IL=-21 */
          0,          /*  EGSM_MAX  IL=-22 */
          0,          /*  EGSM_MAX  IL=-23 */
          0,          /*  EGSM_MAX  IL=-24 */
          0,          /*  EGSM_MAX  IL=-25 */
          0,          /*  EGSM_MAX  IL=-26 */
          0,          /*  EGSM_MAX  IL=-27 */
          0,          /*  EGSM_MAX  IL=-28 */
          0,          /*  EGSM_MAX  IL=-29 */
          0,          /*  EGSM_MAX  IL=-30 */
          0,          /*  EGSM_MAX  IL=-31 */
          0,          /*  EGSM_MAX  IL=-32 */
          0,          /*  EGSM_MAX  IL=-33 */
          0,          /*  EGSM_MAX  IL=-34 */
          0,          /*  EGSM_MAX  IL=-35 */
          0,          /*  EGSM_MAX  IL=-36 */
          0,          /*  EGSM_MAX  IL=-37 */
          0,          /*  EGSM_MAX  IL=-38 */
          0,          /*  EGSM_MAX  IL=-39 */
          0,          /*  EGSM_MAX  IL=-40 */
          0,          /*  EGSM_MAX  IL=-41 */
          0,          /*  EGSM_MAX  IL=-42 */
          5,          /*  EGSM_MAX  IL=-43 */
          5,          /*  EGSM_MAX  IL=-44 */
          0,          /*  EGSM_MAX  IL=-45 */
          0,          /*  EGSM_MAX  IL=-46 */
          0,          /*  EGSM_MAX  IL=-47 */
          0,          /*  EGSM_MAX  IL=-48 */
          0,          /*  EGSM_MAX  IL=-49 */
          0,          /*  EGSM_MAX  IL=-50 */
          0,          /*  EGSM_MAX  IL=-51 */
          0,          /*  EGSM_MAX  IL=-52 */
          0,          /*  EGSM_MAX  IL=-53 */
          0,          /*  EGSM_MAX  IL=-54 */
          0,          /*  EGSM_MAX  IL=-55 */
          0,          /*  EGSM_MAX  IL=-56 */
          0,          /*  EGSM_MAX  IL=-57 */
          0,          /*  EGSM_MAX  IL=-58 */
          0,          /*  EGSM_MAX  IL=-59 */
          0,          /*  EGSM_MAX  IL=-60 */
          0,          /*  EGSM_MAX  IL=-61 */
          0,          /*  EGSM_MAX  IL=-62 */
          0,          /*  EGSM_MAX  IL=-63 */
          0,          /*  EGSM_MAX  IL=-64 */
          0,          /*  EGSM_MAX  IL=-65 */
          0,          /*  EGSM_MAX  IL=-66 */
          0,          /*  EGSM_MAX  IL=-67 */
          0,          /*  EGSM_MAX  IL=-68 */
          0,          /*  EGSM_MAX  IL=-69 */
          0,          /*  EGSM_MAX  IL=-70 */
          0,          /*  EGSM_MAX  IL=-71 */
          0,          /*  EGSM_MAX  IL=-72 */
          0,          /*  EGSM_MAX  IL=-73 */
          0,          /*  EGSM_MAX  IL=-74 */
          5,          /*  EGSM_MAX  IL=-75 */
          5,          /*  EGSM_MAX  IL=-76 */
          8,          /*  EGSM_MAX  IL=-77 */
          8,          /*  EGSM_MAX  IL=-78 */
          11,          /*  EGSM_MAX  IL=-79 */
          11,          /*  EGSM_MAX  IL=-80 */
          11,          /*  EGSM_MAX  IL=-81 */
          11,          /*  EGSM_MAX  IL=-82 */
          11,          /*  EGSM_MAX  IL=-83 */
          11,          /*  EGSM_MAX  IL=-84 */
          14,          /*  EGSM_MAX  IL=-85 */
          14,          /*  EGSM_MAX  IL=-86 */
          14,          /*  EGSM_MAX  IL=-87 */
          14,          /*  EGSM_MAX  IL=-88 */
          14,          /*  EGSM_MAX  IL=-89 */
          14,          /*  EGSM_MAX  IL=-90 */
          14,          /*  EGSM_MAX  IL=-91 */
          14,          /*  EGSM_MAX  IL=-92 */
          14,          /*  EGSM_MAX  IL=-93 */
          14,          /*  EGSM_MAX  IL=-94 */
          17,          /*  EGSM_MAX  IL=-95 */
          17,          /*  EGSM_MAX  IL=-96 */
          20,          /*  EGSM_MAX  IL=-97 */
          20,          /*  EGSM_MAX  IL=-98 */
          20,          /*  EGSM_MAX  IL=-99 */
          20,          /*  EGSM_MAX  IL=-100 */
          23,          /*  EGSM_MAX  IL=-101 */
          23,          /*  EGSM_MAX  IL=-102 */
          23,          /*  EGSM_MAX  IL=-103 */
          23,          /*  EGSM_MAX  IL=-104 */
          23,          /*  EGSM_MAX  IL=-105 */
          23,          /*  EGSM_MAX  IL=-106 */
          23,          /*  EGSM_MAX  IL=-107 */
          23,          /*  EGSM_MAX  IL=-108 */
          23,          /*  EGSM_MAX  IL=-109 */
          23,          /*  EGSM_MAX  IL=-110 */
          23,          /*  EGSM_MAX  IL=-111 */
          23,          /*  EGSM_MAX  IL=-112 */
          23,          /*  EGSM_MAX  IL=-113 */
          23,          /*  EGSM_MAX  IL=-114 */
          23,          /*  EGSM_MAX  IL=-115 */
          23,          /*  EGSM_MAX  IL=-116 */
          23,          /*  EGSM_MAX  IL=-117 */
          23,          /*  EGSM_MAX  IL=-118 */
          23,          /*  EGSM_MAX  IL=-119 */
          23           /*  EGSM_MAX  IL=-120 */
        },
        { // il2agc_av
          // Note this is shared between PCN and EGSM.
          0,          /*  EGSM_MAX  IL=0 */
          0,          /*  EGSM_MAX  IL=-1 */
          0,          /*  EGSM_MAX  IL=-2 */
          0,          /*  EGSM_MAX  IL=-3 */
          0,          /*  EGSM_MAX  IL=-4 */
          0,          /*  EGSM_MAX  IL=-5 */
          0,          /*  EGSM_MAX  IL=-6 */
          0,          /*  EGSM_MAX  IL=-7 */
          0,          /*  EGSM_MAX  IL=-8 */
          0,          /*  EGSM_MAX  IL=-9 */
          0,          /*  EGSM_MAX  IL=-10 */
          0,          /*  EGSM_MAX  IL=-11 */
          0,          /*  EGSM_MAX  IL=-12 */
          0,          /*  EGSM_MAX  IL=-13 */
          0,          /*  EGSM_MAX  IL=-14 */
          0,          /*  EGSM_MAX  IL=-15 */
          0,          /*  EGSM_MAX  IL=-16 */
          0,          /*  EGSM_MAX  IL=-17 */
          0,          /*  EGSM_MAX  IL=-18 */
          0,          /*  EGSM_MAX  IL=-19 */
          0,          /*  EGSM_MAX  IL=-20 */
          0,          /*  EGSM_MAX  IL=-21 */
          0,          /*  EGSM_MAX  IL=-22 */
          0,          /*  EGSM_MAX  IL=-23 */
          0,          /*  EGSM_MAX  IL=-24 */
          0,          /*  EGSM_MAX  IL=-25 */
          0,          /*  EGSM_MAX  IL=-26 */
          0,          /*  EGSM_MAX  IL=-27 */
          0,          /*  EGSM_MAX  IL=-28 */
          0,          /*  EGSM_MAX  IL=-29 */
          0,          /*  EGSM_MAX  IL=-30 */
          0,          /*  EGSM_MAX  IL=-31 */
          0,          /*  EGSM_MAX  IL=-32 */
          0,          /*  EGSM_MAX  IL=-33 */
          0,          /*  EGSM_MAX  IL=-34 */
          0,          /*  EGSM_MAX  IL=-35 */
          0,          /*  EGSM_MAX  IL=-36 */
          0,          /*  EGSM_MAX  IL=-37 */
          0,          /*  EGSM_MAX  IL=-38 */
          0,          /*  EGSM_MAX  IL=-39 */
          0,          /*  EGSM_MAX  IL=-40 */
          0,          /*  EGSM_MAX  IL=-41 */
          0,          /*  EGSM_MAX  IL=-42 */
          5,          /*  EGSM_MAX  IL=-43 */
          5,          /*  EGSM_MAX  IL=-44 */
          0,          /*  EGSM_MAX  IL=-45 */
          0,          /*  EGSM_MAX  IL=-46 */
          0,          /*  EGSM_MAX  IL=-47 */
          0,          /*  EGSM_MAX  IL=-48 */
          0,          /*  EGSM_MAX  IL=-49 */
          0,          /*  EGSM_MAX  IL=-50 */
          0,          /*  EGSM_MAX  IL=-51 */
          0,          /*  EGSM_MAX  IL=-52 */
          0,          /*  EGSM_MAX  IL=-53 */
          0,          /*  EGSM_MAX  IL=-54 */
          0,          /*  EGSM_MAX  IL=-55 */
          0,          /*  EGSM_MAX  IL=-56 */
          0,          /*  EGSM_MAX  IL=-57 */
          0,          /*  EGSM_MAX  IL=-58 */
          0,          /*  EGSM_MAX  IL=-59 */
          0,          /*  EGSM_MAX  IL=-60 */
          0,          /*  EGSM_MAX  IL=-61 */
          0,          /*  EGSM_MAX  IL=-62 */
          0,          /*  EGSM_MAX  IL=-63 */
          0,          /*  EGSM_MAX  IL=-64 */
          0,          /*  EGSM_MAX  IL=-65 */
          0,          /*  EGSM_MAX  IL=-66 */
          0,          /*  EGSM_MAX  IL=-67 */
          0,          /*  EGSM_MAX  IL=-68 */
          0,          /*  EGSM_MAX  IL=-69 */
          0,          /*  EGSM_MAX  IL=-70 */
          0,          /*  EGSM_MAX  IL=-71 */
          0,          /*  EGSM_MAX  IL=-72 */
          0,          /*  EGSM_MAX  IL=-73 */
          0,          /*  EGSM_MAX  IL=-74 */
          5,          /*  EGSM_MAX  IL=-75 */
          5,          /*  EGSM_MAX  IL=-76 */
          8,          /*  EGSM_MAX  IL=-77 */
          8,          /*  EGSM_MAX  IL=-78 */
          11,          /*  EGSM_MAX  IL=-79 */
          11,          /*  EGSM_MAX  IL=-80 */
          11,          /*  EGSM_MAX  IL=-81 */
          11,          /*  EGSM_MAX  IL=-82 */
          11,          /*  EGSM_MAX  IL=-83 */
          11,          /*  EGSM_MAX  IL=-84 */
          14,          /*  EGSM_MAX  IL=-85 */
          14,          /*  EGSM_MAX  IL=-86 */
          14,          /*  EGSM_MAX  IL=-87 */
          14,          /*  EGSM_MAX  IL=-88 */
          14,          /*  EGSM_MAX  IL=-89 */
          14,          /*  EGSM_MAX  IL=-90 */
          14,          /*  EGSM_MAX  IL=-91 */
          14,          /*  EGSM_MAX  IL=-92 */
          14,          /*  EGSM_MAX  IL=-93 */
          14,          /*  EGSM_MAX  IL=-94 */
          17,          /*  EGSM_MAX  IL=-95 */
          17,          /*  EGSM_MAX  IL=-96 */
          20,          /*  EGSM_MAX  IL=-97 */
          20,          /*  EGSM_MAX  IL=-98 */
          20,          /*  EGSM_MAX  IL=-99 */
          20,          /*  EGSM_MAX  IL=-100 */
          23,          /*  EGSM_MAX  IL=-101 */
          23,          /*  EGSM_MAX  IL=-102 */
          23,          /*  EGSM_MAX  IL=-103 */
          23,          /*  EGSM_MAX  IL=-104 */
          23,          /*  EGSM_MAX  IL=-105 */
          23,          /*  EGSM_MAX  IL=-106 */
          23,          /*  EGSM_MAX  IL=-107 */
          23,          /*  EGSM_MAX  IL=-108 */
          23,          /*  EGSM_MAX  IL=-109 */
          23,          /*  EGSM_MAX  IL=-110 */
          23,          /*  EGSM_MAX  IL=-111 */
          23,          /*  EGSM_MAX  IL=-112 */
          23,          /*  EGSM_MAX  IL=-113 */
          23,          /*  EGSM_MAX  IL=-114 */
          23,          /*  EGSM_MAX  IL=-115 */
          23,          /*  EGSM_MAX  IL=-116 */
          23,          /*  EGSM_MAX  IL=-117 */
          23,          /*  EGSM_MAX  IL=-118 */
          23,          /*  EGSM_MAX  IL=-119 */
          23           /*  EGSM_MAX  IL=-120 */
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

#if (L1_FF_MULTIBAND == 0)
    T_RF_BAND rf_band[GSM_BANDS]; //uninitialised rf struct for bands
#else
    T_RF_BAND rf_band[RF_NB_SUPPORTED_BANDS];
#endif


const T_RF_BAND rf_900 =
{
  { //RX structure
     //T_RX_CAL_PARAMS rx_cal_params
    {
        229,      //g_magic
         54,      //lna_gain_max * 2
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
      {457,  0, 0}, // 0
      {457,  0, 0}, // 1
      {457,  0, 0}, // 2
      {457,  0, 0}, // 3
      {457,  0, 0}, // 4
      {457,  0, 0}, // 5 Highest power
      {400,  1, 0}, // 6
      {325,  2, 0}, // 7
      {270,  3, 0}, // 8
      {220,  4, 0}, // 9
      {180,  5, 0}, // 10
      {155,  6, 0}, // 11
      {130,  7, 0}, // 12
      {110,  8, 0}, // 13
      { 95,  9, 0},  // 14
      { 80, 10, 0}, // 15
      { 70, 11, 0}, // 16
      { 62, 12, 0}, // 17
      { 55, 13, 0}, // 18
      { 50, 14, 0}, // 19 Lowest power
      { 50, 14, 0}, // 20
      { 50, 14, 0}, // 21
      { 50, 14, 0}, // 22
      { 50, 14, 0}, // 23
      { 50, 14, 0}, // 24
      { 50, 14, 0}, // 25
      { 50, 14, 0}, // 26
      { 50, 14, 0}, // 27
      { 50, 14, 0}, // 28
      { 50, 14, 0}, // 29
      { 50, 14, 0}, // 30
      { 50, 14, 0}, // 31
    },
    #if(REL99 && FF_PRF)// needs proper values from RF
    {// gsm900 levels_power_reduction
    	{457,  0,  0}, // for uplink 1 timeslot = no power reduction ie 0,0 dB
    	{457,  0,  0}, // for 2 uplink timeslot = reduction between 0,0 dB to 3,0 dB
    	{457,  0,  0}, // for 3 uplink timeslot = reduction between 1,8 dB to 4,8 dB
    	{457,  0,  0}, // for 4 uplink timselot = reduction between 3,0 dB to 6,0 dB
	},
    #endif
    {// Channel Calibration Tables
      {// arfcn, tx_chan_cal
        {   20, 128 }, // Calibration Table 0
        {  100, 128 },
        {  124, 128 },
        {  586, 128 },
        {  661, 128 },
        {  736, 128 },
        {  885, 128 },
        { 1023, 128 }
        },
      {// arfcn, tx_chan_cal
        {   40, 128 }, // Calibration Table 1
        {   80, 128 },
        {  124, 128 },
        {  586, 128 },
        {  661, 128 },
        {  736, 128 },
        {  885, 128 },
        { 1023, 128 }
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
    { // GSM Power Ramp Values
     {
      {// Ramp-Up      #0 profile - Power Level 5
        0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #0 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #1 profile - Power Level 6
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #1 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #2 profile - Power Level 7
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #2 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #3 profile - Power Level 8
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #3 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #4 profile - Power Level 9
        0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #4 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #5 profile - Power Level 10
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #5 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #6 profile - Power Level 11
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #6 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #7 profile - Power Level 12
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #7 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #8 profile - Power Level 13
        0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #8 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #9 profile - Power Level 14
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #9 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #10 profile - Power Level 15
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #10 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #11 profile - Power Level 16
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #11 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #12 profile - Power Level 17
        0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #12 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #13 profile - Power Level 18
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #13 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #14 profile - Power Level 19
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #14 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #15 profile - Power Level 19
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #15 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
    },
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
      229,      //g_magic
       54,      //lna gain * 2
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
      {424, 0, 0}, // 0 Highest power
      {375, 1, 0}, // 1
      {310, 2, 0}, // 2
      {255, 3, 0}, // 3
      {210, 4, 0}, // 4
      {180, 5, 0}, // 5
      {152, 6, 0}, // 6
      {130, 7, 0}, // 7
      {112, 8, 0}, // 8
      {98, 9, 0}, // 9
      {85, 10, 0}, // 10
      {75, 11, 0}, // 11
      {66, 12, 0}, // 12
      {58, 13, 0}, // 13
      {52, 14, 0}, // 14
      {47, 15, 0}, // 15 Lowest power
      {47, 15, 0}, // 16
      {47, 15, 0}, // 17
      {47, 15, 0}, // 18
      {47, 15, 0}, // 19
      {47, 15, 0}, // 20
      {47, 15, 0}, // 21
      {47, 15, 0}, // 22
      {47, 15, 0}, // 23
      {47, 15, 0}, // 24
      {47, 15, 0}, // 25
      {47, 15, 0}, // 26
      {47, 15, 0}, // 27
      {47, 15, 0}, // 28
      {424, 0, 0}, // 29 Highest power
      {424, 0, 0}, // 30 Highest power
      {424, 0, 0}, // 31 Highest power
    },
    #if(REL99 && FF_PRF)// needs proper values from RF
    {// 1800 levels_power_reduction
    	{424,  0,  0}, // for uplink 1 timeslot = no power reduction ie 0,0 dB
    	{424,  0,  0}, // for 2 uplink timeslot = reduction between 0,0 dB to 3,0 dB
    	{424,  0,  0}, // for 3 uplink timeslot = reduction between 1,8 dB to 4,8 dB
    	{424,  0,  0}, // for 4 uplink timselot = reduction between 3,0 dB to 6,0 dB
	},
    #endif
    {// Channel Calibration Tables
      {// arfcn, tx_chan_cal
        {  554, 128 }, // Calibration Table 0
        {  722, 128 },
        {  746, 128 },
        {  774, 128 },
        {  808, 128 },
        {  851, 128 },
        {  870, 128 },
        {  885, 128 }
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
      {
      {// Ramp-Up      #0 profile - Power Level 0
        0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #0 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #1 profile - Power Level 1
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #1 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #2 profile - Power Level 2
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #2 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #3 profile - Power Level 3
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #3 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #4 profile - Power Level 4
        0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #4 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #5 profile - Power Level 5
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #5 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #6 profile - Power Level 6
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #6 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #7 profile - Power Level 7
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #7 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #8 profile - Power Level 8
        0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #8 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #9 profile - Power Level 9
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #9 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #10 profile - Power Level 10
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #10 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #11 profile - Power Level 11
       0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #11 profile
       255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #12 profile - Power Level 12
        0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #12 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #13 profile - Power Level 13
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #13 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #14 profile - Power Level 14
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255

      },
      {// Ramp-Down    #14 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #15 profile - Power Level 15
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #15 profile
        255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
      },
     },
    },
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

        229,      //g_magic
         54,      //lna_gain_max * 2
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
      {486, 0, 0}, // 0
      {486, 0, 0}, // 1
      {486, 0, 0}, // 2
      {486, 0, 0}, // 3
      {486, 0, 0}, // 4
      {486, 0, 0}, // 5 Highest power
      {400, 1, 0}, // 6
      {330, 2, 0}, // 7
      {270, 3, 0}, // 8
      {223, 4, 0}, // 9
      {186, 5, 0}, // 10
      {158, 6, 0}, // 11
      {133, 7, 0}, // 12
      {114, 8, 0}, // 13
      {97, 9, 0}, // 14
      {84, 10, 0}, // 15
      {73, 11, 0}, // 16
      {64, 12, 0}, // 17
      {57, 13, 0}, // 18
      {52, 14, 0}, // 19 Lowest power
      {52, 14, 0}, // 20
      {52, 14, 0}, // 21
      {52, 14, 0}, // 22
      {52, 14, 0}, // 23
      {52, 14, 0}, // 24
      {52, 14, 0}, // 25
      {52, 14, 0}, // 26
      {52, 14, 0}, // 27
      {52, 14, 0}, // 28
      {52, 14, 0}, // 29
      {52, 14, 0}, // 30
      {52, 14, 0}, // 31
    },
    #if(REL99 && FF_PRF)// needs proper values from RF
    {// gsm850 levels_power_reduction
    	{486,  0,  0}, // for uplink 1 timeslot = no power reduction ie 0,0 dB
    	{486,  0,  0}, // for 2 uplink timeslot = reduction between 0,0 dB to 3,0 dB
    	{486,  0,  0}, // for 3 uplink timeslot = reduction between 1,8 dB to 4,8 dB
    	{486,  0,  0}, // for 4 uplink timselot = reduction between 3,0 dB to 6,0 dB
	},
    #endif
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
      {
        {// Ramp-Up      #0 profile - Power Level 5
          0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #0 profile
          255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #1 profile - Power Level 6
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #1 profile
          255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #2 profile - Power Level 7
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #2 profile
          255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #3 profile - Power Level 8
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #3 profile
          255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #4 profile - Power Level 9
          0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #4 profile
          255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #5 profile - Power Level 10
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #5 profile
          255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #6 profile - Power Level 11
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #6 profile
          255,251,238,218,191,160,127,95,64,37,
        17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #7 profile - Power Level 12
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #7 profile
          255,251,238,218,191,160,127,95,64,37,
          17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #8 profile - Power Level 13
          0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #8 profile
          255,251,238,218,191,160,127,95,64,37,
          17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #9 profile - Power Level 14
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #9 profile
          255,251,238,218,191,160,127,95,64,37,
          17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #10 profile - Power Level 15
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #10 profile
          255,251,238,218,191,160,127,95,64,37,
          17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #11 profile - Power Level 16
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #11 profile
          255,251,238,218,191,160,127,95,64,37,
          17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #12 profile - Power Level 17
          0,0,0,0,0,0,10,14,17,25,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #12 profile
          255,251,238,218,191,160,127,95,64,37,
          17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #13 profile - Power Level 18
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #13 profile
          255,251,238,218,191,160,127,95,64,37,
          17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #14 profile - Power Level 19
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #14 profile
          255,251,238,218,191,160,127,95,64,37,
          17,4,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #15 profile - Power Level 19
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
        },
        {// Ramp-Down    #15 profile
          255,251,238,218,191,160,127,95,64,37,
          17,4,0,0,0,0,0,0,0,0
        },
      },
    },
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
      229,      //g_magic
       54,      //lna gain * 2
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
      {438, 0, 0}, // 0 Highest power
      {380, 1, 0}, // 1
      {310, 2, 0}, // 2
      {255, 3, 0}, // 3
      {213, 4, 0}, // 4
      {180, 5, 0}, // 5
      {152, 6, 0}, // 6
      {130, 7, 0}, // 7
      {112, 8, 0}, // 8
      {98, 9, 0}, // 9
      {87, 10, 0}, // 10
      {77, 11, 0}, // 11
      {68, 12, 0}, // 12
      {60, 13, 0}, // 13
      {53, 14, 0}, // 14
      {48, 15, 0}, // 15 Lowest power
      {48, 15, 0}, // 16
      {48, 15, 0}, // 17
      {48, 15, 0}, // 18
      {48, 15, 0}, // 19
      {48, 15, 0}, // 20
      {48, 15, 0}, // 21
      {48, 15, 0}, // 22
      {48, 15, 0}, // 23
      {48, 15, 0}, // 24
      {48, 15, 0}, // 25
      {48, 15, 0}, // 26
      {48, 15, 0}, // 27
      {48, 15, 0}, // 28
      {438, 0, 0}, // 29 Highest power
      {438, 0, 0}, // 30 Highest power
      {438, 0, 0}, // 31 Highest power
    },
    #if(REL99 && FF_PRF)// needs proper values from RF
    {// PCS1900 band levels_power_reduction
    	{438,  0,  0}, // for uplink 1 timeslot = no power reduction ie 0,0 dB
    	{438,  0,  0}, // for 2 uplink timeslot = reduction between 0,0 dB to 3,0 dB
    	{438,  0,  0}, // for 3 uplink timeslot = reduction between 1,8 dB to 4,8 dB
    	{438,  0,  0}, // for 4 uplink timselot = reduction between 3,0 dB to 6,0 dB
	},
    #endif
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
     {
      {// Ramp-Up      #0 profile - Power Level 0
          0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #0 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #1 profile - Power Level 1
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #1 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #2 profile - Power Level 2
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #2 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #3 profile - Power Level 3
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #3 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #4 profile - Power Level 4
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #4 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #5 profile - Power Level 5
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #5 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #6 profile - Power Level 6
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #6 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #7 profile - Power Level 7
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #7 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #8 profile - Power Level 8
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #8 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #9 profile - Power Level 9
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #9 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #10 profile - Power Level 10
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #10 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #11 profile - Power Level 11
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #11 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #12 profile - Power Level 12
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #12 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #13 profile - Power Level 13
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #13 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #14 profile - Power Level 14
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #14 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
     {
      {// Ramp-Up      #15 profile - Power Level 15
        0,0,0,0,0,0,10,14,21,29,
		37,64,95,127,160,191,218,238,251,255
      },
      {// Ramp-Down    #15 profile
        255,240,228,220,185,160,127,95,64,37,
        17,14,0,0,0,0,0,0,0,0
      },
     },
    },
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





UWORD16 drp_wrapper[DRP_WRAPPER_TABLE_SIZE]=
{
    C_APCCTRL2,
    C_APCDEL1,
    C_APCDEL2
    };



/*------------------------------------------*/
/*             Gain table                   */
/*  specified in the TRF6053 spec           */
/*     2 dB steps - LNA always ON           */
/*------------------------------------------*/
UWORD16 AGC_TABLE[AGC_TABLE_SIZE] =
{
   ABE_0_DB,
   ABE_0_DB,
   ABE_2_DB,
   ABE_2_DB,
   ABE_2_DB,
   ABE_5_DB,
   ABE_5_DB,
   ABE_5_DB,
   ABE_8_DB,
   ABE_8_DB,
   ABE_8_DB,
   ABE_11_DB,
   ABE_11_DB,
   ABE_11_DB,
   ABE_14_DB,
   ABE_14_DB,
   ABE_14_DB,
   ABE_17_DB,
   ABE_17_DB,
   ABE_17_DB,
   ABE_20_DB,
   ABE_20_DB,
   ABE_20_DB,
   ABE_23_DB
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

// table which converts ADC value into RF temperature
const T_TEMP temperature[TEMP_TABLE_SIZE] =
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


/*------------------------------------------*/
/* ABB Initialization words                 */
/*------------------------------------------*/
#if (ANLG_FAM == 11)
 UWORD8 abb[ABB_TABLE_SIZE] =
{
    C_VULGAIN,
    C_VDLGAIN,
    C_SIDETONE,
    C_CTRL1,
    C_CTRL2,
    C_CTRL3,
    C_CTRL4,
    C_CTRL5,
    C_CTRL6,
    C_POPAUTO,
    C_OUTEN1,
    C_OUTEN2,
    C_OUTEN3,
    C_AULGA,
    C_AURGA
};
#endif

#if (L1_FF_MULTIBAND == 0)
#else
/* The physical RF bands are enumerated in order of increasing frequencies */
/* The same order must be used in l1_rf61, l1_cust, and l1_const */
const T_MULTIBAND_RF multiband_rf[RF_NB_SUPPORTED_BANDS + 1] =
{
 /*power_class, tx_turning_point, max_txpwr, gsm_band_identifier, name*/
#if (GSM900_SUPPORTED == 1)
  { 4,           0,                19,        RF_GSM900,              "900"},
#endif /* if(GSM900_SUPPORTED == 1)*/
#if (GSM850_SUPPORTED == 1)
  { 4,          0,                19,        RF_GSM850,              "850"},
#endif /*if (GSM850_SUPPORTED == 1)*/
#if (DCS1800_SUPPORTED == 1)
  { 1,          28,               15,        RF_DCS1800,             "1800"},
#endif /*if (DCS1800_SUPPORTED == 1)*/
#if (PCS1900_SUPPORTED == 1)
  {1,           21,               15,        RF_PCS1900,             "1900"},
#endif /*if (PCS1900_SUPPORTED == 1)*/
} ;

/* Array for mapping from subband index into physical band index */
const WORD8 rf_subband2band[RF_NB_SUBBANDS] =
{
#if (GSM900_SUPPORTED == 1)
  RF_GSM900,
  RF_GSM900,
#endif
#if (GSM850_SUPPORTED == 1)
  RF_GSM850,
#endif
#if (DCS1800_SUPPORTED == 1)
  RF_DCS1800,
#endif
#if (PCS1900_SUPPORTED == 1)
  RF_PCS1900,
#endif
};

const T_MULTIBAND_CONVERT multiband_convert[RF_NB_SUBBANDS] =
{ /*first_rf_freq,   last_rf_freq,                  first_l1_freq,    l1freq2rffreq */
#if (GSM900_SUPPORTED == 1)
 { RF_FREQ_1ST_900L, RF_FREQ_1ST_900L+NB_CHAN_900L, L1_FREQ_1ST_900L, RF_FREQ_1ST_900L-L1_FREQ_1ST_900L},
 { RF_FREQ_1ST_900H, RF_FREQ_1ST_900H+NB_CHAN_900H, L1_FREQ_1ST_900H, RF_FREQ_1ST_900H-L1_FREQ_1ST_900H},
#endif
#if (GSM850_SUPPORTED == 1)
 { RF_FREQ_1ST_850, RF_FREQ_1ST_850+NB_CHAN_850, L1_FREQ_1ST_850,  RF_FREQ_1ST_850-L1_FREQ_1ST_850},
#endif
#if (DCS1800_SUPPORTED == 1)
 { RF_FREQ_1ST_1800, RF_FREQ_1ST_1800+NB_CHAN_1800, L1_FREQ_1ST_1800, RF_FREQ_1ST_1800-L1_FREQ_1ST_1800},
#endif
#if (PCS1900_SUPPORTED == 1)
 { RF_FREQ_1ST_1900, RF_FREQ_1ST_1900+NB_CHAN_1900, L1_FREQ_1ST_1900, RF_FREQ_1ST_1900-L1_FREQ_1ST_1900},
#endif
};

/*-------------------------------------------------------*/
/* rf_convert_l1freq_to_l1subband                        */
/*-------------------------------------------------------*/
/* Parameters : l1freq, the l1 radio_freq to convert     */
/*                                                       */
/* Return     : The l1 subband index l1freq belongs to   */
/*                                                       */
/* Functionality : compare l1freq with the band ranges,  */
/*      return the l1 band index where match is found     */
 /*-------------------------------------------------------*/
UWORD8 rf_convert_l1freq_to_l1subband(UWORD16 l1_freq)
{
  WORD32 subband_idx;
  for(subband_idx = RF_NB_SUBBANDS -1; subband_idx>=0; subband_idx--)
  {
    if(l1_freq >= multiband_convert[subband_idx].first_l1_freq)
      return ((UWORD8)subband_idx);
  }
  l1_multiband_error_handler(l1_freq);
  return 0;
}

/*-------------------------------------------------------*/
/* rf_convert_rffreq_to_l1subband                        */
/*-------------------------------------------------------*/
/* Parameters : rffreq, the physical rf freq channel     */
/*              number to convert                        */
/*                                                       */
/* Return     : The l1 subband index rf_freq belongs to  */
/*                                                       */
/* Functionality : compare rf_freq with the band ranges,  */
/*      return the l1 band index where match is found     */
 /*-------------------------------------------------------*/
UWORD8 rf_convert_rffreq_to_l1subband(UWORD16 rf_freq)
{
  WORD32 subband_idx;
  for(subband_idx = RF_NB_SUBBANDS -1; subband_idx>=0; subband_idx--)
  {
    if((rf_freq >= multiband_convert[subband_idx].first_rf_freq)&&
      (rf_freq < multiband_convert[subband_idx].last_rf_freq))
      return ((UWORD8)subband_idx);
  }
  l1_multiband_error_handler(rf_freq);
  return 0;
}

/*-------------------------------------------------------*/
/* rf_convert_l1freq_to_band_idx                         */
/*-------------------------------------------------------*/
/* Parameters : l1freq, the l1 radio_freq to convert     */
/*                                                       */
/* Return     : The physical index of the band of l1freq */
/*                                                       */
/* Functionality : compare l1freq with the band ranges,  */
/*      return the rf_band_index of the l1_band_index    */
/*      where match is found                             */
 /*-------------------------------------------------------*/
WORD8 rf_convert_l1freq_to_rf_band_idx(UWORD16 l1_freq)
{
  return rf_subband2band[rf_convert_l1freq_to_l1subband(l1_freq)];
}

/*-------------------------------------------------------*/
/* rf_convert_l1freq_to_rffreq                            */
/*-------------------------------------------------------*/
/* Parameters : l1freq, the l1 radio_freq to convert     */
/*                                                       */
 /* Return    : ARFCN, absolute radio frequency channel  */
/*             number and in *bandidx the band index     */
/* Functionality : identify band index and look up ARFCN */
/*-------------------------------------------------------*/
UWORD16 rf_convert_l1freq_to_rffreq(UWORD16 l1_freq )
{
  WORD32 subband_idx = rf_convert_l1freq_to_l1subband(l1_freq);
  return(l1_freq+multiband_convert[subband_idx].l1freq2rffreq);
}

/*-------------------------------------------------------*/
/* rf_convert_l1freq_to_arfcn_and_bandidx                */
/*-------------------------------------------------------*/
/* Parameters : l1freq, the l1 radio_freq to convert     */
/*                                                       */
 /* Return    : ARFCN, absolute radio frequency channel  */
/*             number and in *bandidx the RF band index  */
/* Functionality : identify band index and look up ARFCN */
/*-------------------------------------------------------*/

/*-------------------------------------------------------*/
/* rf_convert_l1freq_to_rffreq_and_bandidx                */
/*-------------------------------------------------------*/
/* Parameters : l1freq, the l1 radio_freq to convert     */
/*                                                       */
 /* Return    : ARFCN, absolute radio frequency channel  */
/*             number and in *bandidx the band index     */
/* Functionality : identify band index and look up ARFCN */
/*-------------------------------------------------------*/
UWORD16 rf_convert_l1freq_to_rffreq_rfband(UWORD16 l1_freq, WORD8 *rf_band_index)
{
  WORD32 subband_idx = rf_convert_l1freq_to_l1subband(l1_freq);
  *rf_band_index = rf_subband2band[subband_idx];
  return(l1_freq+multiband_convert[subband_idx].l1freq2rffreq);
}

/*-------------------------------------------------------*/
/* rf_convert_l1freq_to_arfcn_and_bandidx                */
/*-------------------------------------------------------*/
/* Parameters : l1freq, the l1 radio_freq to convert     */
/*                                                       */
 /* Return    : ARFCN, absolute radio frequency channel  */
/*             number and in *bandidx the RF band index  */
/* Functionality : identify band index and look up ARFCN */
/*-------------------------------------------------------*/
UWORD16 rf_convert_l1freq_to_arfcn_rfband(UWORD16 l1_freq, WORD8 *rf_band_index)
{
  WORD32 subband_idx = rf_convert_l1freq_to_l1subband(l1_freq);
  *rf_band_index = rf_subband2band[subband_idx];
#if (PCS1900_SUPPORTED)
  if(RF_PCS1900 == rf_subband2band[subband_idx])
    return(l1_freq+multiband_convert[subband_idx].l1freq2rffreq-512);
  else
#endif
    return(l1_freq+multiband_convert[subband_idx].l1freq2rffreq);
}

UWORD16 rf_convert_rffreq_to_l1freq(UWORD16 rf_freq)
{
  WORD32 subband_idx = rf_convert_rffreq_to_l1subband(rf_freq);
  return(rf_freq - multiband_convert[subband_idx].l1freq2rffreq);
}

/*-------------------------------------------------------*/
/* rf_convert_rffreq_to_l1freq_rfband                    */
/*-------------------------------------------------------*/
/* Parameters : rffreq, the physical rf channel number   */
/*              to convert                               */
/*                                                       */
 /* Return    : l1freq, l1 internal frequency number     */
/*             and in *bandidx the RF band index         */
/* Functionality : identify band index and look up ARFCN */
/*-------------------------------------------------------*/
UWORD16 rf_convert_rffreq_to_l1freq_rfband(UWORD16 rf_freq, WORD8 *rf_band_index)
{
  WORD32 subband_idx = rf_convert_rffreq_to_l1subband(rf_freq);
  *rf_band_index = (WORD8)subband_idx;
  return(rf_freq - multiband_convert[subband_idx].l1freq2rffreq);
}

#if(PCS1900_SUPPORTED)
extern UWORD16 tm_std_band;
#endif
UWORD16 rf_convert_tmarfcn_to_l1freq(UWORD16 tm_arfcn, WORD8 *error_flag)
{
  *error_flag = 0;
#if(GSM900_SUPPORTED)
  if(NB_CHAN_900L> tm_arfcn)
    return (tm_arfcn + L1_FREQ_1ST_900L- RF_FREQ_1ST_900L);
  if((ARFCN_1ST_900H <= tm_arfcn)&&(ARFCN_1ST_900H + NB_CHAN_900H > tm_arfcn))
    return (tm_arfcn + L1_FREQ_1ST_900H- RF_FREQ_1ST_900H);
#endif
#if(GSM850_SUPPORTED)
  if((ARFCN_1ST_850 <= tm_arfcn)||(ARFCN_1ST_850+NB_CHAN_850 > tm_arfcn))
    return(tm_arfcn + L1_FREQ_1ST_850- RF_FREQ_1ST_850);
#endif
#if(PCS1900_SUPPORTED)
  if((RF_FREQ_1ST_1900<=tm_arfcn)&&((RF_FREQ_1ST_1900+NB_CHAN_1900)>tm_arfcn))
    return (tm_arfcn + L1_FREQ_1ST_1900 - RF_FREQ_1ST_1900); /* PCS RF_FREQ format */
  if(((3 == (tm_std_band & 0xff))||(8==(tm_std_band & 0xff))) &&
    (ARFCN_1ST_1900 <= tm_arfcn) && (ARFCN_1ST_1900+NB_CHAN_1900 > tm_arfcn))
    return(tm_arfcn+L1_FREQ_1ST_1900-ARFCN_1ST_1900); /* PCS Legacy compatible*/
  if((1000+ARFCN_1ST_1900 <= tm_arfcn)&&
    (1000+ARFCN_1ST_1900+NB_CHAN_1900 >= tm_arfcn))
    return(tm_arfcn+L1_FREQ_1ST_1900-ARFCN_1ST_1900-1000); /*ARFCN+1000 format*/
#endif
#if(DCS1800_SUPPORTED)
    if((ARFCN_1ST_1800 <= tm_arfcn)||(ARFCN_1ST_1800+NB_CHAN_1800 > tm_arfcn))
      return(tm_arfcn + L1_FREQ_1ST_1800- RF_FREQ_1ST_1800);
#endif
  *error_flag = 1;
  return (tm_arfcn);
}
#endif// if(L1_FF_MULTIBAND == 1)



T_RF rf =
{
  RF_PASCAL_20, //RF revision
  RF_HW_BAND_SUPPORT, // radio_band_support E-GSM/DCS

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

  6,          /*  EGSM_MAX  IL=0 */  
	6,          /*  EGSM_MAX  IL=-1 */
	6,          /*  EGSM_MAX  IL=-2 */
	6,          /*  EGSM_MAX  IL=-3 */
	6,          /*  EGSM_MAX  IL=-4 */
	6,          /*  EGSM_MAX  IL=-5 */
	6,          /*  EGSM_MAX  IL=-6 */
	6,          /*  EGSM_MAX  IL=-7 */
	6,          /*  EGSM_MAX  IL=-8 */
	6,          /*  EGSM_MAX  IL=-9 */
	6,          /*  EGSM_MAX  IL=-10 */  
	6,          /*  EGSM_MAX  IL=-11 */
	6,          /*  EGSM_MAX  IL=-12 */
	6,          /*  EGSM_MAX  IL=-13 */
	6,          /*  EGSM_MAX  IL=-14 */
	6,          /*  EGSM_MAX  IL=-15 */
	6,          /*  EGSM_MAX  IL=-16 */
	6,          /*  EGSM_MAX  IL=-17 */
	6,          /*  EGSM_MAX  IL=-18 */
	6,          /*  EGSM_MAX  IL=-19 */
	6,          /*  EGSM_MAX  IL=-20 */  
	6,          /*  EGSM_MAX  IL=-21 */
	6,          /*  EGSM_MAX  IL=-22 */
	6,          /*  EGSM_MAX  IL=-23 */
	6,          /*  EGSM_MAX  IL=-24 */
	6,          /*  EGSM_MAX  IL=-25 */
	6,          /*  EGSM_MAX  IL=-26 */
	6,          /*  EGSM_MAX  IL=-27 */
	6,          /*  EGSM_MAX  IL=-28 */
	6,          /*  EGSM_MAX  IL=-29 */
	6,          /*  EGSM_MAX  IL=-30 */  
	6,          /*  EGSM_MAX  IL=-31 */
	6,          /*  EGSM_MAX  IL=-32 */
	6,          /*  EGSM_MAX  IL=-33 */
	6,          /*  EGSM_MAX  IL=-34 */
	6,          /*  EGSM_MAX  IL=-35 */
	6,          /*  EGSM_MAX  IL=-36 */
	6,          /*  EGSM_MAX  IL=-37 */
	6,          /*  EGSM_MAX  IL=-38 */
	6,          /*  EGSM_MAX  IL=-39 */
	6,          /*  EGSM_MAX  IL=-40 */
	6,          /*  EGSM_MAX  IL=-41 */
	6,          /*  EGSM_MAX  IL=-42 */
	6,          /*  EGSM_MAX  IL=-43 */
	6,          /*  EGSM_MAX  IL=-44 */
	6,          /*  EGSM_MAX  IL=-45 */
	6,          /*  EGSM_MAX  IL=-46 */
	6,          /*  EGSM_MAX  IL=-47 */
	8,          /*  EGSM_MAX  IL=-48 */
	8,          /*  EGSM_MAX  IL=-49 */
	10,          /*  EGSM_MAX  IL=-50 */
	10,          /*  EGSM_MAX  IL=-51 */
	12,          /*  EGSM_MAX  IL=-52 */
	12,          /*  EGSM_MAX  IL=-53 */
	14,          /*  EGSM_MAX  IL=-54 */
	14,          /*  EGSM_MAX  IL=-55 */
	16,          /*  EGSM_MAX  IL=-56 */
	16,          /*  EGSM_MAX  IL=-57 */
	18,          /*  EGSM_MAX  IL=-58 */
	18,          /*  EGSM_MAX  IL=-59 */
	20,          /*  EGSM_MAX  IL=-60 */
	20,          /*  EGSM_MAX  IL=-61 */
	22,          /*  EGSM_MAX  IL=-62 */
	22,          /*  EGSM_MAX  IL=-63 */
	24,          /*  EGSM_MAX  IL=-64 */
	24,          /*  EGSM_MAX  IL=-65 */
	26,          /*  EGSM_MAX  IL=-66 */
	26,          /*  EGSM_MAX  IL=-67 */
	28,          /*  EGSM_MAX  IL=-68 */
	28,          /*  EGSM_MAX  IL=-69 */
	30,          /*  EGSM_MAX  IL=-70 */
	30,          /*  EGSM_MAX  IL=-71 */
	32,          /*  EGSM_MAX  IL=-72 */
	32,          /*  EGSM_MAX  IL=-73 */
	34,          /*  EGSM_MAX  IL=-74 */
	34,          /*  EGSM_MAX  IL=-75 */
	36,          /*  EGSM_MAX  IL=-76 */
	36,          /*  EGSM_MAX  IL=-77 */
	38,          /*  EGSM_MAX  IL=-78 */
	38,          /*  EGSM_MAX  IL=-79 */
	40,          /*  EGSM_MAX  IL=-80 */
	40,          /*  EGSM_MAX  IL=-81 */
	42,          /*  EGSM_MAX  IL=-82 */
	42,          /*  EGSM_MAX  IL=-83 */
	44,          /*  EGSM_MAX  IL=-84 */
	44,          /*  EGSM_MAX  IL=-85 */
	46,          /*  EGSM_MAX  IL=-86 */
	46,          /*  EGSM_MAX  IL=-87 */
	48,          /*  EGSM_MAX  IL=-88 */
	48,          /*  EGSM_MAX  IL=-89 */
	50,          /*  EGSM_MAX  IL=-90 */
	50,          /*  EGSM_MAX  IL=-91 */
	50,          /*  EGSM_MAX  IL=-92 */
	50,          /*  EGSM_MAX  IL=-93 */
	50,          /*  EGSM_MAX  IL=-94 */
	50,          /*  EGSM_MAX  IL=-95 */
	50,          /*  EGSM_MAX  IL=-96 */
	50,          /*  EGSM_MAX  IL=-97 */
	50,          /*  EGSM_MAX  IL=-98 */
	50,          /*  EGSM_MAX  IL=-99 */
	50,          /*  EGSM_MAX  IL=-100 */
	50,          /*  EGSM_MAX  IL=-101 */
	50,          /*  EGSM_MAX  IL=-102 */
	50,          /*  EGSM_MAX  IL=-103 */
	50,          /*  EGSM_MAX  IL=-104 */
	50,          /*  EGSM_MAX  IL=-105 */
	50,          /*  EGSM_MAX  IL=-106 */
	50,          /*  EGSM_MAX  IL=-107 */
	50,          /*  EGSM_MAX  IL=-108 */
	50,          /*  EGSM_MAX  IL=-109 */
	50,          /*  EGSM_MAX  IL=-110 */
	50,          /*  EGSM_MAX  IL=-111 */
	50,          /*  EGSM_MAX  IL=-112 */
	50,          /*  EGSM_MAX  IL=-113 */
	50,          /*  EGSM_MAX  IL=-114 */
	50,          /*  EGSM_MAX  IL=-115 */
	50,          /*  EGSM_MAX  IL=-116 */
	50,          /*  EGSM_MAX  IL=-117 */
	50,          /*  EGSM_MAX  IL=-118 */
	50,          /*  EGSM_MAX  IL=-119 */
	50           /*  EGSM_MAX  IL=-120 */
      },
  { // il2agc_max
        // Note this is shared between PCN and EGSM.
	6,          /*  EGSM_MAX  IL=0 */  
	6,          /*  EGSM_MAX  IL=-1 */
	6,          /*  EGSM_MAX  IL=-2 */
	6,          /*  EGSM_MAX  IL=-3 */
	6,          /*  EGSM_MAX  IL=-4 */
	6,          /*  EGSM_MAX  IL=-5 */
	6,          /*  EGSM_MAX  IL=-6 */
	6,          /*  EGSM_MAX  IL=-7 */
	6,          /*  EGSM_MAX  IL=-8 */
	6,          /*  EGSM_MAX  IL=-9 */
	6,          /*  EGSM_MAX  IL=-10 */  
	6,          /*  EGSM_MAX  IL=-11 */
	6,          /*  EGSM_MAX  IL=-12 */
	6,          /*  EGSM_MAX  IL=-13 */
	6,          /*  EGSM_MAX  IL=-14 */
	6,          /*  EGSM_MAX  IL=-15 */
	6,          /*  EGSM_MAX  IL=-16 */
	6,          /*  EGSM_MAX  IL=-17 */
	6,          /*  EGSM_MAX  IL=-18 */
	6,          /*  EGSM_MAX  IL=-19 */
	6,          /*  EGSM_MAX  IL=-20 */  
	6,          /*  EGSM_MAX  IL=-21 */
	6,          /*  EGSM_MAX  IL=-22 */
	6,          /*  EGSM_MAX  IL=-23 */
	6,          /*  EGSM_MAX  IL=-24 */
	6,          /*  EGSM_MAX  IL=-25 */
	6,          /*  EGSM_MAX  IL=-26 */
	6,          /*  EGSM_MAX  IL=-27 */
	6,          /*  EGSM_MAX  IL=-28 */
	6,          /*  EGSM_MAX  IL=-29 */
	6,          /*  EGSM_MAX  IL=-30 */  
	6,          /*  EGSM_MAX  IL=-31 */
	6,          /*  EGSM_MAX  IL=-32 */
	6,          /*  EGSM_MAX  IL=-33 */
	6,          /*  EGSM_MAX  IL=-34 */
	6,          /*  EGSM_MAX  IL=-35 */
	6,          /*  EGSM_MAX  IL=-36 */
	6,          /*  EGSM_MAX  IL=-37 */
	6,          /*  EGSM_MAX  IL=-38 */
	6,          /*  EGSM_MAX  IL=-39 */
	6,          /*  EGSM_MAX  IL=-40 */
	6,          /*  EGSM_MAX  IL=-41 */
	6,          /*  EGSM_MAX  IL=-42 */
	6,          /*  EGSM_MAX  IL=-43 */
	6,          /*  EGSM_MAX  IL=-44 */
	6,          /*  EGSM_MAX  IL=-45 */
	6,          /*  EGSM_MAX  IL=-46 */
	6,          /*  EGSM_MAX  IL=-47 */
	8,          /*  EGSM_MAX  IL=-48 */
	8,          /*  EGSM_MAX  IL=-49 */
	10,          /*  EGSM_MAX  IL=-50 */
	10,          /*  EGSM_MAX  IL=-51 */
	12,          /*  EGSM_MAX  IL=-52 */
	12,          /*  EGSM_MAX  IL=-53 */
	14,          /*  EGSM_MAX  IL=-54 */
	14,          /*  EGSM_MAX  IL=-55 */
	16,          /*  EGSM_MAX  IL=-56 */
	16,          /*  EGSM_MAX  IL=-57 */
	18,          /*  EGSM_MAX  IL=-58 */
	18,          /*  EGSM_MAX  IL=-59 */
	20,          /*  EGSM_MAX  IL=-60 */
	20,          /*  EGSM_MAX  IL=-61 */
	22,          /*  EGSM_MAX  IL=-62 */
	22,          /*  EGSM_MAX  IL=-63 */
	24,          /*  EGSM_MAX  IL=-64 */
	24,          /*  EGSM_MAX  IL=-65 */
	26,          /*  EGSM_MAX  IL=-66 */
	26,          /*  EGSM_MAX  IL=-67 */
	28,          /*  EGSM_MAX  IL=-68 */
	28,          /*  EGSM_MAX  IL=-69 */
	30,          /*  EGSM_MAX  IL=-70 */
	30,          /*  EGSM_MAX  IL=-71 */
	32,          /*  EGSM_MAX  IL=-72 */
	32,          /*  EGSM_MAX  IL=-73 */
	34,          /*  EGSM_MAX  IL=-74 */
	34,          /*  EGSM_MAX  IL=-75 */
	36,          /*  EGSM_MAX  IL=-76 */
	36,          /*  EGSM_MAX  IL=-77 */
	38,          /*  EGSM_MAX  IL=-78 */
	38,          /*  EGSM_MAX  IL=-79 */
	40,          /*  EGSM_MAX  IL=-80 */
	40,          /*  EGSM_MAX  IL=-81 */
	42,          /*  EGSM_MAX  IL=-82 */
	42,          /*  EGSM_MAX  IL=-83 */
	44,          /*  EGSM_MAX  IL=-84 */
	44,          /*  EGSM_MAX  IL=-85 */
	46,          /*  EGSM_MAX  IL=-86 */
	46,          /*  EGSM_MAX  IL=-87 */
	48,          /*  EGSM_MAX  IL=-88 */
	48,          /*  EGSM_MAX  IL=-89 */
	50,          /*  EGSM_MAX  IL=-90 */
	50,          /*  EGSM_MAX  IL=-91 */
	50,          /*  EGSM_MAX  IL=-92 */
	50,          /*  EGSM_MAX  IL=-93 */
	50,          /*  EGSM_MAX  IL=-94 */
	50,          /*  EGSM_MAX  IL=-95 */
	50,          /*  EGSM_MAX  IL=-96 */
	50,          /*  EGSM_MAX  IL=-97 */
	50,          /*  EGSM_MAX  IL=-98 */
	50,          /*  EGSM_MAX  IL=-99 */
	50,          /*  EGSM_MAX  IL=-100 */
	50,          /*  EGSM_MAX  IL=-101 */
	50,          /*  EGSM_MAX  IL=-102 */
	50,          /*  EGSM_MAX  IL=-103 */
	50,          /*  EGSM_MAX  IL=-104 */
	50,          /*  EGSM_MAX  IL=-105 */
	50,          /*  EGSM_MAX  IL=-106 */
	50,          /*  EGSM_MAX  IL=-107 */
	50,          /*  EGSM_MAX  IL=-108 */
	50,          /*  EGSM_MAX  IL=-109 */
	50,          /*  EGSM_MAX  IL=-110 */
	50,          /*  EGSM_MAX  IL=-111 */
	50,          /*  EGSM_MAX  IL=-112 */
	50,          /*  EGSM_MAX  IL=-113 */
	50,          /*  EGSM_MAX  IL=-114 */
	50,          /*  EGSM_MAX  IL=-115 */
	50,          /*  EGSM_MAX  IL=-116 */
	50,          /*  EGSM_MAX  IL=-117 */
	50,          /*  EGSM_MAX  IL=-118 */
	50,          /*  EGSM_MAX  IL=-119 */
	50           /*  EGSM_MAX  IL=-120 */
      },
       { // il2agc_av
        // Note this is shared between PCN and EGSM.
	6,          /*  EGSM_AV  IL=0 */  
	6,          /*  EGSM_AV  IL=-1 */
	6,          /*  EGSM_AV  IL=-2 */
	6,          /*  EGSM_AV  IL=-3 */
	6,          /*  EGSM_AV  IL=-4 */
	6,          /*  EGSM_AV  IL=-5 */
	6,          /*  EGSM_AV  IL=-6 */
	6,          /*  EGSM_AV  IL=-7 */
	6,          /*  EGSM_AV  IL=-8 */
	6,          /*  EGSM_AV  IL=-9 */
	6,          /*  EGSM_AV  IL=-10 */  
	6,          /*  EGSM_AV  IL=-11 */
	6,          /*  EGSM_AV  IL=-12 */
	6,          /*  EGSM_AV  IL=-13 */
	6,          /*  EGSM_AV  IL=-14 */
	6,          /*  EGSM_AV  IL=-15 */
	6,          /*  EGSM_AV  IL=-16 */
	6,          /*  EGSM_AV  IL=-17 */
	6,          /*  EGSM_AV  IL=-18 */
	6,          /*  EGSM_AV  IL=-19 */
	6,          /*  EGSM_AV  IL=-20 */  
	6,          /*  EGSM_AV  IL=-21 */
	6,          /*  EGSM_AV  IL=-22 */
	6,          /*  EGSM_AV  IL=-23 */
	6,          /*  EGSM_AV  IL=-24 */
	6,          /*  EGSM_AV  IL=-25 */
	6,          /*  EGSM_AV  IL=-26 */
	6,          /*  EGSM_AV  IL=-27 */
	6,          /*  EGSM_AV  IL=-28 */
	6,          /*  EGSM_AV  IL=-29 */
	6,          /*  EGSM_AV  IL=-30 */  
	6,          /*  EGSM_AV  IL=-31 */
	6,          /*  EGSM_AV  IL=-32 */
	6,          /*  EGSM_AV  IL=-33 */
	6,          /*  EGSM_AV  IL=-34 */
	6,          /*  EGSM_AV  IL=-35 */
	6,          /*  EGSM_AV  IL=-36 */
	6,          /*  EGSM_AV  IL=-37 */
	6,          /*  EGSM_AV  IL=-38 */
	6,          /*  EGSM_AV  IL=-39 */
	6,          /*  EGSM_AV  IL=-40 */
	6,          /*  EGSM_AV  IL=-41 */
	6,          /*  EGSM_AV  IL=-42 */
	6,          /*  EGSM_AV  IL=-43 */
	6,          /*  EGSM_AV  IL=-44 */
	6,          /*  EGSM_AV  IL=-45 */
	6,          /*  EGSM_AV  IL=-46 */
	6,          /*  EGSM_AV  IL=-47 */
	8,          /*  EGSM_AV  IL=-48 */
	8,          /*  EGSM_AV  IL=-49 */
	10,          /*  EGSM_AV  IL=-50 */
	10,          /*  EGSM_AV  IL=-51 */
	12,          /*  EGSM_AV  IL=-52 */
	12,          /*  EGSM_AV  IL=-53 */
	14,          /*  EGSM_AV  IL=-54 */
	14,          /*  EGSM_AV  IL=-55 */
	16,          /*  EGSM_AV  IL=-56 */
	16,          /*  EGSM_AV  IL=-57 */
	18,          /*  EGSM_AV  IL=-58 */
	18,          /*  EGSM_AV  IL=-59 */
	20,          /*  EGSM_AV  IL=-60 */
	20,          /*  EGSM_AV  IL=-61 */
	22,          /*  EGSM_AV  IL=-62 */
	22,          /*  EGSM_AV  IL=-63 */
	24,          /*  EGSM_AV  IL=-64 */
	24,          /*  EGSM_AV  IL=-65 */
	26,          /*  EGSM_AV  IL=-66 */
	26,          /*  EGSM_AV  IL=-67 */
	28,          /*  EGSM_AV  IL=-68 */
	28,          /*  EGSM_AV  IL=-69 */
	30,          /*  EGSM_AV  IL=-70 */
	30,          /*  EGSM_AV  IL=-71 */
	32,          /*  EGSM_AV  IL=-72 */
	32,          /*  EGSM_AV  IL=-73 */
	34,          /*  EGSM_AV  IL=-74 */
	34,          /*  EGSM_AV  IL=-75 */
	36,          /*  EGSM_AV  IL=-76 */
	36,          /*  EGSM_AV  IL=-77 */
	38,          /*  EGSM_AV  IL=-78 */
	38,          /*  EGSM_AV  IL=-79 */
	40,          /*  EGSM_AV  IL=-80 */
	40,          /*  EGSM_AV  IL=-81 */
	42,          /*  EGSM_AV  IL=-82 */
	42,          /*  EGSM_AV  IL=-83 */
	44,          /*  EGSM_AV  IL=-84 */
	44,          /*  EGSM_AV  IL=-85 */
	46,          /*  EGSM_AV  IL=-86 */
	46,          /*  EGSM_AV  IL=-87 */
	48,          /*  EGSM_AV  IL=-88 */
	48,          /*  EGSM_AV  IL=-89 */
	50,          /*  EGSM_AV  IL=-90 */
	50,          /*  EGSM_AV  IL=-91 */
	50,          /*  EGSM_AV  IL=-92 */
	50,          /*  EGSM_AV  IL=-93 */
	50,          /*  EGSM_AV  IL=-94 */
	50,          /*  EGSM_AV  IL=-95 */
	50,          /*  EGSM_AV  IL=-96 */
	50,          /*  EGSM_AV  IL=-97 */
	50,          /*  EGSM_AV  IL=-98 */
	50,          /*  EGSM_AV  IL=-99 */
	50,          /*  EGSM_AV  IL=-100 */
	50,          /*  EGSM_AV  IL=-101 */
	50,          /*  EGSM_AV  IL=-102 */
	50,          /*  EGSM_AV  IL=-103 */
	50,          /*  EGSM_AV  IL=-104 */
	50,          /*  EGSM_AV  IL=-105 */
	50,          /*  EGSM_AV  IL=-106 */
	50,          /*  EGSM_AV  IL=-107 */
	50,          /*  EGSM_AV  IL=-108 */
	50,          /*  EGSM_AV  IL=-109 */
	50,          /*  EGSM_AV  IL=-110 */
	50,          /*  EGSM_AV  IL=-111 */
	50,          /*  EGSM_AV  IL=-112 */
	50,          /*  EGSM_AV  IL=-113 */
	50,          /*  EGSM_AV  IL=-114 */
	50,          /*  EGSM_AV  IL=-115 */
	50,          /*  EGSM_AV  IL=-116 */
	50,          /*  EGSM_AV  IL=-117 */
	50,          /*  EGSM_AV  IL=-118 */
	50,          /*  EGSM_AV  IL=-119 */
	50           /*  EGSM_AV  IL=-120 */    
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

    #if (VCXO_ALGO == 1)
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
      {  10,  1},     // sub-band1 up to arfcn =  10, Agc calibration = 0db
      {  30,  1},     // sub-band2 up to arfcn =  30, Agc calibration = 0db
      {  51,  0},     // sub-band3 up to arfcn =  51, Agc calibration = 0db
      {  71,  0},     // etc.
      {  90,  1},     // 
      { 112,  1},     // 
      { 124,  2},     //
      { 991,  3},    // 
      {1009,  2},    // 
      {1023,  1},    // 
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
      { 510, 0, 0  }, // 0 
      { 510, 0, 0  }, // 1 
      { 510, 0, 0  }, // 2 
      { 510, 0, 0  }, // 3 
      { 510, 0, 0  }, // 4 
      { 510, 0, 0  }, // 5 
      { 449, 1, 0  }, // 6 
      { 361, 2, 0  }, // 7 
      { 291, 3, 0  }, // 8 
      { 236, 4, 0  }, // 9 
      { 192, 5, 0  }, // 10 
      { 157, 6, 0  }, // 11 
      { 130, 7, 0  }, // 12 
      { 107, 8, 0  }, // 13 
      { 86, 9, 0  }, // 14 
      { 71, 10, 0  }, // 15 
      { 61, 11, 0  }, // 16 
      { 52, 12, 0  }, // 17 
      { 47, 13, 0  }, // 18 
      { 43, 14, 0  }, // 19 
      { 43, 14, 0  }, // 20 
      { 43, 14, 0  }, // 21 
      { 43, 14, 0  }, // 22 
      { 43, 14, 0  }, // 23 
      { 43, 14, 0  }, // 24 
      { 43, 14, 0  }, // 25 
      { 43, 14, 0  }, // 26 
      { 43, 14, 0  }, // 27 
      { 43, 14, 0  }, // 28 
      { 43, 14, 0  }, // 29 
      { 43, 14, 0  }, // 30 
      { 43, 14, 0  }, // 31 
    },
    {// Channel Calibration Talbles
      {// arfcn, tx_chan_cal
        {   27, 126 }, // Calibration Table 0
        {   47, 128 },
        {   66, 129 },
        {   85, 129 },
        {  104, 133 },
        {  124, 133 },
        {  994, 125 },
        { 1023, 125 }
        },
      {// arfcn, tx_chan_cal
        {   27, 128 }, // Calibration Table 1
        {   47, 128 },
        {   66, 128 },
        {   85, 128 },
        {  104, 128 },
        {  124, 128 },
        {  994, 128 },
        { 1023, 128 }
      },
      {// arfcn, tx_chan_cal
        {   27, 128 }, // Calibration Table 2
        {   47, 128 },
        {   66, 128 },
        {   85, 128 },
        {  104, 128 },
        {  124, 128 },  
        {  994, 128 },
        { 1023, 128 }
        },
      {// arfcn, tx_chan_cal
        {   27, 128 }, // Calibration Table 3
        {   47, 128 },
        {   66, 128 },
        {   85, 128 },
        {  104, 128 },
        {  124, 128 },
        {  994, 128 },
        { 1023, 128 }
      } 
    }, 
    { // GSM Power Ramp Values
      {  
        {// Ramp-Up      #0 profile - Power Level 5
         0,0,0,0,0,0,0,0,0,10,31,31,31,15,10,0
        },
        {// Ramp-Down    #0 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #1 profile - Power Level 6
         0,0,0,0,0,0,0,0,0,10,31,31,31,15,10,0
        },
        {// Ramp-Down    #1 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #2 profile - Power Level 7
          0,0,0,0,0,0,0,0,0,6,19,31,31,31,10,0
        },
        {// Ramp-Down    #2 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #3 profile - Power Level 8
            0,0,0,0,0,0,0,0,0,6,19,31,31,31,10,0
        },
        {// Ramp-Down    #3 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #4 profile - Power Level 9
          0,0,0,0,0,0,0,0,0,9,16,31,31,31,10,0
        },
        {// Ramp-Down    #4 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #5 profile - Power Level 10
           0,0,0,0,0,0,0,0,0,9,16,31,31,31,10,0
        },
        {// Ramp-Down    #5 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #6 profile - Power Level 11
          0,0,0,0,0,0,0,0,0,9,16,31,31,31,10,0
        },
        {// Ramp-Down    #6 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #7 profile - Power Level 12
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #7 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #8 profile - Power Level 13
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #8 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #9 profile - Power Level 14
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #9 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #10 profile - Power Level 15
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #10 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #11 profile - Power Level 16
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #11 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #12 profile - Power Level 17
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #12 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #13 profile - Power Level 18
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #13 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #14 profile - Power Level 19
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #14 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #15 profile - Power Level 19
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #15 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        }
      }   
    },
    { //TX temperature compensation
#if (ORDER2_TX_TEMP_CAL==1)
      { -11,  0, 0, 0 },
      {  +9,  0, 0, 0 },
      { +39,  0, 0, 0 },
      { +59,  0, 0, 0 },
      { 127,  0, 0, 0 }
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
      {548,  0},     // 
      {622,  1},     // 
      {680,  0},     // 
      {745,  0},     // 
      {812,  0},     // 
      {860,  0},     // 
      {885,  2},     // 
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
      { 463, 0, 0  }, // 0 
      { 376, 1, 0  }, // 1 
      { 309, 2, 0  }, // 2 
      { 255, 3, 0  }, // 3 
      { 211, 4, 0  }, // 4 
      { 176, 5, 0  }, // 5 
      { 146, 6, 0  }, // 6 
      { 122, 7, 0  }, // 7 
      { 99, 8, 0  }, // 8 
      { 83, 9, 0  }, // 9 
      { 70, 10, 0  }, // 10 
      { 58, 11, 0  }, // 11 
      { 47, 12, 0  }, // 12 
      { 38, 13, 0  }, // 13 
      { 32, 14, 0  }, // 14 
      { 26, 15, 0  }, // 15 
      { 26, 15, 0  }, // 16 
      { 26, 15, 0  }, // 17 
      { 26, 15, 0  }, // 18 
      { 26, 15, 0  }, // 19 
      { 26, 15, 0  }, // 20 
      { 26, 15, 0  }, // 21 
      { 26, 15, 0  }, // 22 
      { 26, 15, 0  }, // 23 
      { 26, 15, 0  }, // 24 
      { 26, 15, 0  }, // 25 
      { 26, 15, 0  }, // 26 
      { 26, 15, 0  }, // 27 
      { 26, 15, 0  }, // 28 
      { 463, 0, 0  }, // 29 
      { 463, 0, 0  }, // 30 
      { 463, 0, 0  }, // 31 
    },
    {// Channel Calibration Talbles
      {// arfcn, tx_chan_cal
        {  553, 128 }, // Calibration Table 0
        {  594, 128 },
        {  636, 128 },
        {  677, 128 },
        {  720, 128 },
        {  760, 128 },
        {  802, 127 },
        {  885, 127 }
      },
      {
        {  553, 128 }, // Calibration Table 1
        {  594, 128 },
        {  636, 128 },
        {  677, 128 },
        {  720, 128 },
        {  760, 128 },
        {  802, 128 },
        {  885, 128 }
      },  
      {// arfcn, tx_chan_cal
        {  553, 128 }, // Calibration Table 2
        {  594, 128 },
        {  636, 128 },
        {  677, 128 },
        {  720, 128 },
        {  760, 128 },
        {  802, 128 },
        {  885, 128 }
      },
      {// arfcn, tx_chan_cal
        {  553, 128 }, // Calibration Table 3
        {  594, 128 },
        {  636, 128 },
        {  677, 128 },
        {  720, 128 },
        {  760, 128 },
        {  802, 128 },
        {  885, 128 }
      } 
    }, 
    { // DCS Power Ramp Values
      {  
        {// Ramp-Up      #0 profile - Power Level 0
          0,0,0,0,0,0,0,0,0,5,20,31,31,31,10,0
        },
        {// Ramp-Down    #0 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #1 profile - Power Level 1
          0,0,0,0,0,0,0,0,0,5,20,31,31,31,10,0
        },
        {// Ramp-Down    #1 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #2 profile - Power Level 2
          0,0,0,0,0,0,0,0,0,5,20,31,31,31,10,0
        },
        {// Ramp-Down    #2 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #3 profile - Power Level 3
          0,0,0,0,0,0,0,0,0,5,20,31,31,31,10,0
        },
        {// Ramp-Down    #3 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #4 profile - Power Level 4
          0,0,0,0,0,0,0,0,0,5,20,31,31,31,10,0
        },
        {// Ramp-Down    #4 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #5 profile - Power Level 5
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #5 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #6 profile - Power Level 6
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #6 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #7 profile - Power Level 7
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #7 profile
         0 ,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #8 profile - Power Level 8
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #8 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #9 profile - Power Level 9
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #9 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #10 profile - Power Level 10
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #10 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #11 profile - Power Level 11
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #11 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #12 profile - Power Level 12
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #12 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #13 profile - Power Level 13
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #13 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #14 profile - Power Level 14
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #14 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #15 profile - Power Level 15
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
        },
        {// Ramp-Down    #15 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
    },
    { //TX temperature compensation
#if (ORDER2_TX_TEMP_CAL==1)
      { -11,  0, 0, 0 },
      {  +9,  0, 0, 0 },
      { +39,  0, 0, 0 },
      { +59,  0, 0, 0 },
      { 127,  0, 0, 0 }
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
        181,      //g_magic
         40,      //lna_gain_max * 2   
         40,      //lna_th_high
         44       //lna_th_low
    },
    { //T_RF_AGC_BAND   agc_bands[RF_RX_CAL_CHAN_SIZE];
     // Remark: ARFCN=0 (GSM-E) is maintained by 1st GSM subbband.
                     // upper_bound, agc_calib 
      {  10,  0},     // sub-band1 up to arfcn =  10, Agc calibration = 0db
      {  30,  0},     // sub-band2 up to arfcn =  30, Agc calibration = 0db
      {  51,  0},     // sub-band3 up to arfcn =  51, Agc calibration = 0db
      {  71,  0},     // etc.
      {  90,  0},     // 
      { 112,  0},     // 
      { 124,  0},     //
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
      { 510, 0, 0  }, // 0 
      { 510, 0, 0  }, // 1 
      { 510, 0, 0  }, // 2 
      { 510, 0, 0  }, // 3 
      { 510, 0, 0  }, // 4 
      { 510, 0, 0  }, // 5 
      { 449, 1, 0  }, // 6 
      { 361, 2, 0  }, // 7 
      { 291, 3, 0  }, // 8 
      { 236, 4, 0  }, // 9 
      { 192, 5, 0  }, // 10 
      { 157, 6, 0  }, // 11 
      { 130, 7, 0  }, // 12 
      { 107, 8, 0  }, // 13 
      { 86, 9, 0  }, // 14 
      { 71, 10, 0  }, // 15 
      { 61, 11, 0  }, // 16 
      { 52, 12, 0  }, // 17 
      { 47, 13, 0  }, // 18 
      { 43, 14, 0  }, // 19 
      { 43, 14, 0  }, // 20 
      { 43, 14, 0  }, // 21 
      { 43, 14, 0  }, // 22 
      { 43, 14, 0  }, // 23 
      { 43, 14, 0  }, // 24 
      { 43, 14, 0  }, // 25 
      { 43, 14, 0  }, // 26 
      { 43, 14, 0  }, // 27 
      { 43, 14, 0  }, // 28 
      { 43, 14, 0  }, // 29 
      { 43, 14, 0  }, // 30 
      { 43, 14, 0  }, // 31 
    },
    {// Channel Calibration Talbles
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
    { //Ramp profiles    850
      {
        {// Ramp-Up      #0 profile - Power Level 5
          0,0,0,0,0,0,0,0,0,10,31,31,31,15,10,0
	},
	{ //Ramp-Down #0 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #1 profile - Power Level 6
         0,0,0,0,0,0,0,0,0,10,31,31,31,15,10,0
	},
	{ //Ramp-Down #1 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #2 profile - Power Level 7
          0,0,0,0,0,0,0,0,0,6,19,31,31,31,10,0
	},
	{ //Ramp-Down #2 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #3 profile - Power Level 8
            0,0,0,0,0,0,0,0,0,6,19,31,31,31,10,0
	},
	{ //Ramp-Down #3 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #4 profile - Power Level 9
          0,0,0,0,0,0,0,0,0,9,16,31,31,31,10,0
	},
	{ //Ramp-Down #4 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #5 profile - Power Level 10
          0,0,0,0,0,0,0,0,0,9,16,31,31,31,10,0
	},
	{ //Ramp-Down #5 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #6 profile - Power Level 11
          0,0,0,0,0,0,0,0,0,9,16,31,31,31,10,0
	},
	{ //Ramp-Down #6 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #7 profile - Power Level 12
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
	},
 	{ //Ramp-Down #7 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #8 profile - Power Level 13
           0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
	},
	{ //Ramp-Down #8 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #9 profile - Power Level 14
           0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
	},
	{ //Ramp-Down #9 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #10 profile - Power Level 15
           0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
	},
	{ //Ramp-Down #10 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #11 profile - Power Level 16
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
	},
	{ //Ramp-Down #11 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #12 profile - Power Level 17
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
	},
	{ //Ramp-Down #12 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #13 profile - Power Level 18
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
	},
	{ //Ramp-Down #13 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #14 profile - Power Level 19
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
	},
	{ //Ramp-Down #14 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {
        {// Ramp-Up      #15 profile - Power Level 19
          0,0,0,0,0,0,0,0,0,0,25,31,31,31,10,0
	      },
        {// Ramp-Down    #15 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        }
      }   
      },
      { //TX temperature compensation
#if (ORDER2_TX_TEMP_CAL==1)
      { -11,  0, 0, 0 },
      {  +9,  0, 0, 0 },
      { +39,  0, 0, 0 },
      { +59,  0, 0, 0 },
      { 127,  0, 0, 0 }
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
      186,      //g_magic
       40,      //lna gain * 2
       40,      //lna_th_high
       44       //lna_th_low
    },
    { //T_RF_AGC_BAND   agc_bands[RF_RX_CAL_CHAN_SIZE];
     /*--------------*/
     /*-- PCS band --*/
     /*--------------*/
      { 548,  1},     // 
      { 622,  1},     // 
      { 680,  0},     // 
      { 745,  0},     // 
      { 812,  3},     // 
      { 860,  0},     // 
      { 885,  2},     // 
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
    {// pcs1900 T_LEVEL_TX
      { 460, 0, 0  }, // 0 
      { 373, 1, 0  }, // 1 
      { 307, 2, 0  }, // 2 
      { 253, 3, 0  }, // 3 
      { 209, 4, 0  }, // 4 
      { 175, 5, 0  }, // 5 
      { 145, 6, 0  }, // 6 
      { 122, 7, 0  }, // 7 
      { 99, 8, 0  }, // 8 
      { 83, 9, 0  }, // 9 
      { 70, 10, 0  }, // 10 
      { 58, 11, 0  }, // 11 
      { 47, 12, 0  }, // 12 
      { 38, 13, 0  }, // 13 
      { 32, 14, 0  }, // 14 
      { 25, 15, 0  }, // 15 
      { 25, 15, 0  }, // 16 
      { 25, 15, 0  }, // 17 
      { 25, 15, 0  }, // 18 
      { 25, 15, 0  }, // 19 
      { 25, 15, 0  }, // 20 
      { 25, 15, 0  }, // 21 
      { 25, 15, 0  }, // 22 
      { 25, 15, 0  }, // 23 
      { 25, 15, 0  }, // 24 
      { 25, 15, 0  }, // 25 
      { 25, 15, 0  }, // 26 
      { 25, 15, 0  }, // 27 
      { 25, 15, 0  }, // 28 
      { 460, 0, 0  }, // 29 
      { 460, 0, 0  }, // 30 
      { 460, 0, 0  }, // 31 
    },
    {// Channel Calibration Tables
      {// arfcn, tx_chan_cal
        {  549, 127 }, // Calibration Table 0
        {  586, 127 },
        {  623, 127 },
        {  697, 128 },
        {  726, 129 },
        {  754, 128 },
        {  782, 129 },
        {  810, 131 }
      },
      {
        {  549, 128 }, // Calibration Table 1
        {  586, 128 },
        {  623, 128 },
        {  697, 128 },
        {  726, 128 },
        {  754, 128 },
        {  782, 128 },
        {  810, 128 }
      },  
      {// arfcn, tx_chan_cal
        {  549, 128 }, // Calibration Table 2
        {  586, 128 },
        {  623, 128 },
        {  697, 128 },
        {  726, 128 },
        {  754, 128 },
        {  782, 128 },
        {  810, 128 }
      },
      {// arfcn, tx_chan_cal
        {  549, 128 }, // Calibration Table 3
        {  586, 128 },
        {  623, 128 },
        {  697, 128 },
        {  726, 128 },
        {  754, 128 },
        {  782, 128 },
        {  810, 128 }
      } 
    }, 
    { // PCS1900 Power Ramp Values 
      {  
        {// Ramp-Up      #0 profile - Power Level 0
          0,0,0,0,0,0,0,0,0,9,16,31,31,31,10,0
        },
        {// Ramp-Down    #0 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #1 profile - Power Level 1
          0,0,0,0,0,0,0,0,0,10,15,31,31,31,10,0
        },
        {// Ramp-Down    #1 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #2 profile - Power Level 2
          0,0,0,0,0,0,0,0,0,10,15,31,31,31,10,0
        },
        {// Ramp-Down    #2 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #3 profile - Power Level 3
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #3 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #4 profile - Power Level 4
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #4 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #5 profile - Power Level 5
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #5 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #6 profile - Power Level 6
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #6 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #7 profile - Power Level 7
          0 ,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #7 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #8 profile - Power Level 8
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #8 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #9 profile - Power Level 9
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #9 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #10 profile - Power Level 10
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #10 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #11 profile - Power Level 11
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #11 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #12 profile - Power Level 12
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #12 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #13 profile - Power Level 13
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #13 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #14 profile - Power Level 14
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #14 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
      {  
        {// Ramp-Up      #15 profile - Power Level 15
          0,0,0,0,0,0,0,0,0,17,18,29,24,30,10,0
        },
        {// Ramp-Down    #15 profile
          0,10,25,31,31,22,9,0,0,0,0,0,0,0,0,0
        },
      },
    },
    { //TX temperature compensation
#if (ORDER2_TX_TEMP_CAL==1)
      { -11,  0, 0, 0 },
      {  +9,  0, 0, 0 },
      { +39,  0, 0, 0 },
      { +59,  0, 0, 0 },
      { 127,  0, 0, 0 }
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
/*     2 dB steps - LNA always ON 			*/		  
/*------------------------------------------*/
UWORD16 AGC_TABLE[AGC_TABLE_SIZE] =
{
  0x00,  //6dB
  0x01,  //8dB
  0x02,  //10dB
  0x03,  //12dB
  0x04,  //14
  0x05,  //16
  0x06,  //18
  0x07,  //20
  0x08,  //22
  0x09,  //24
  0x0a,  //26
  0x0b,  //28
  0x0c,  //30
  0x0d,  //32
  0x0e,  //34
  0x0f,  //36
  0x10,  //38
  0x11,  //40
  0x12,  //42
  0x13,  //44
  0x14,  //46
  0x15,  //48
  0x16,  //50
  0x17,  //52
  0x18,  //54
  0x19,  //56
  0x1a   //58
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

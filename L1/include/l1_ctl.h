/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_CTL.H
 *
 *        Filename l1_ctl.h
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

/************************************/
/* Automatic timing control (TOA)   */
/************************************/
#define  C_RED              1   // Factor used to reduce the maximum accumulated values.
                                // Default : 1/2
#define  C_GEW              1   // Weighting factor. Default : 1/2

#define  C_SNRGR         2560   // 2.5 F6.10
#define  C_SNR_THR       8192   // 8   F6.10
#define  TOA_HISTO_LEN   11     // Histogram length

#define   IL_FOR_RXLEV_SNR      220 // RX POWER LEVEL  //increased from 210 to 220

#if (TOA_ALGO == 2)
  #define   L1_TOA_SCALING_CONSTANT    (32768)    // L1_TOA_SCALING_CONSTANT
  #define   L1_TOA_LAMBDA              (0x7800)   // 0.9375 represented in Q15 i.e. 0.9375*32768
  #define   L1_TOA_ONE_MINUS_LAMBDA    (0x0800)   // 0.0625 represented in Q15 i.e. 0.0625*32768
  #define   L1_TOA_THRESHOLD_15        (0x1333)   // 0.15 in Q15
  #define   L1_TOA_THRESHOLD_20        (0x1999)   // 0.20 in Q15
  #define   L1_TOA_THRESHOLD_25        (0x2000)   // 0.25 in Q15
  #define   L1_TOA_THRESHOLD_30        (0x2666)   // 0.30 in Q15
  #if (CODE_VERSION == SIMULATION)
    #define   L1_TOA_SNR_THRESHOLD       (0)      // For simulator the threshold is made zero to facilitate
                                                  // TOA testing in simulator
  #else
    #if (NEW_SNR_THRESHOLD == 0)
    #define   L1_TOA_SNR_THRESHOLD       (480)    // 0x1E0 in F6.10 is equal to 0.46875
    #else
    #define   L1_TOA_SNR_THRESHOLD       (2048)    // 0x1E0 in F6.10 is equal to 0.46875
	#endif /* NEW_SNR_THRESHOLD*/

  #endif
  #if (CODE_VERSION == SIMULATION)
    #define   L1_TOA_EXPECTED_TOA        (14)       // Expected TOA on the MCU side
  #else
    #define   L1_TOA_EXPECTED_TOA        (14)       // Expected TOA on the MCU side
  #endif
#endif

#if (NEW_SNR_THRESHOLD == 1)
  #define SAIC_OFF (0)
  #define SAIC_ON  (1)
#endif /* NEW_SNR_THRESHOLD */
/************************************/
/* Automatic Gain Control (AGC)     */
/************************************/

#define INDEX_MIN           0
#define INDEX_MAX         240   // 120

/************************************/
/* Automatic frequency compensation */
/************************************/
#if (L1_SAIC == 1)
#define  C_thr_snr        2048//  1/0.4    * 2**10   - CQ no- 76320
#else
#define  C_thr_snr        2560     //  1/0.4    * 2**10
#endif
#define  C_thr_P          524288L  //  0.5      * 2**20
#define  C_cov_start      838861L  //  0.8      * 2**20
#define  C_a0_kalman      10486L   //  0.01     * 2**20
#define  C_g_kalman       53687091L//  0.05     * 2**30
#define  C_N_del          2        //  delay of frequency control loop
                                   //  due to C W R pipeline
#define  C_Q              3L       //  0.000003 * 2**20
#define  C_thr_K          209715L  //  0.2      * 2**20
#define  C_thr_phi        328      //  0.01     * 2**15

#if (VCXO_ALGO == 1)
  #define  C_WIN_AVG_SIZE_M       64  // average size M
  #define  C_PSI_AVG_SIZE_D       32  // distance size D
  #define  C_MSIZE                (C_WIN_AVG_SIZE_M * C_PSI_AVG_SIZE_D) // Data history for predictor
  #define  C_RGAP_BAD_SNR_COUNT_B 32  // bad SNR count B
  #define  ALGO_AFC_RXGAP            1  // reception gap algo
  #define  ALGO_AFC_KALMAN           1  // Kalman filter
  #define  ALGO_AFC_LQG_PREDICTOR    2  // LQG filter + rgap predictor
  #define  ALGO_AFC_KALMAN_PREDICTOR 3 // Kalman filter + rgap predictor
#endif

#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3) || (ANALOG == 11))
   // clipping related to AFC DAC linearity range
  #define  C_max_step        32000   //   4000 * 2**3
  #define  C_min_step       -32000   //  -4000 * 2**3
#endif

/***************************************************/
/* SAIC (Single Antenna Interference Cancellation) */
/***************************************************/

#if (L1_SAIC != 0)
#define   L1_SAIC_GENIE_GSM_GPRS_IDLE_THRESHOLD        192 // Input Level Threshold for GSM/GPRS Idle.
  #define   L1_SAIC_GENIE_GSM_DEDIC_THRESHOLD          192 // Input Level Threshold for GSM Dedicated. 
  #define   L1_SAIC_GENIE_GPRS_PCKT_TRAN_THRESHOLD     192 // Input Level Threshold for GPRS Packet Transfer  
#endif


#define  L1_SAIC_HARDWARE_FILTER     (1)
#define  L1_SAIC_PROGRAMMABLE_FILTER (0)

//Locosto. For Locosto psi_quant = F14.2  else F13.3
#if(RF_FAM == 61)
#define CONVERT_PSI_QUANT(value) (value >> 2)
#else
#define CONVERT_PSI_QUANT(value) (value >> 3)
#endif


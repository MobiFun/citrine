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

/*
 * FreeCalypso Frankenstein: the following definition has been added
 * from LoCosto version of this file, as it is used by l1_cmplx.c
 * fairly extensively.
 *
 * Disassembly-matching reconstruction has revealed that the constant
 * in question was originally 210 in the TCS211 version, and then
 * increased to 220 in the LoCosto source.  We currently seek to match
 * TCS211 without any changes, hence we are setting it back to 210.
 */
#define   IL_FOR_RXLEV_SNR      210 // RX POWER LEVEL

/************************************/
/* Automatic Gain Control (AGC)     */
/************************************/

#define INDEX_MIN           0
#define INDEX_MAX         240   // 120

/************************************/
/* Automatic frequency compensation */
/************************************/
#define  C_thr_snr        2560     //  1/0.4    * 2**10               
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

#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
   // clipping related to AFC DAC linearity range
  #define  C_max_step        32000   //   4000 * 2**3                    
  #define  C_min_step       -32000   //  -4000 * 2**3                   
#endif

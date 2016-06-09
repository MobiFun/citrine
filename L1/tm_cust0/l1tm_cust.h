/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1TM_CUST.H
 *
 *        Filename l1tm_cust.h
 *  Copyright 2003 (C) Texas Instruments  
 * 
 ************* Revision Controle System Header *************/


/*---------------------------------------------------------*/
/* Initial settings for test mode config => Cust_tm_init() */
/*---------------------------------------------------------*/

// Control algorithm settings: 0=>OFF, 1=>ON
#define AGC_ENABLE   1
#define AFC_ENABLE   1

// ADC conversion setting: 0=>OFF, 1=>ON
#define ADC_ENABLE   1

// AGC settings
#define TM_AGC_VALUE   50   // AGC gain
#define TM_LNA_OFF      0   // 0=>LNA ON, 1=>LNA OFF

// Power measurement settings
#define TM_NUM_MEAS     1  // number of measurements per TDMA
#define TM_WIN_MEAS     1  // position of measurement within TDMA

// BEACON and TCH settings
#define TM_BCCH_ARFCN   80     // beacon 
#define TM_TCH_ARFCN    62     // TCH arfcn
#define TM_MON_ARFCN    33     // monitor arfcn
#define TM_CHAN_TYPE    TCH_F  // channel type
#define TM_SUB_CHAN      0     // subchannel number
#define TM_SLOT_NUM      4     // TS number
#define TM_TSC           5     // Training Sequence
#define TM_TXPWR        15     // TXPWR setting
#define TM_TXPWR_SKIP    4
#define TM_TA            0     // timing advance setting
#define TM_BURST_TYPE    0     // 0=>normal burst, 1=>RACH burst
#define TM_BURST_DATA    0     // as defined in TM100.doc: tx_param_write
#define TM_PM_ENABLE     1     // Enable power measurements in packet transfer mode

// Statistics settings
#define TM_NUM_LOOPS              0  // number of times a task is executed, 0 means infinite loop
#define TM_AUTO_RESULT_LOOPS      0  // number of loops before stats result is returned, 0 means infinite
#define TM_AUTO_RESET_LOOPS       0  // number of loops before stats I/F is reset, 0 means infinite
#define TM_STAT_TYPE              1  // type of stats as defined in TM100.doc: stats_read
#define TM_STAT_BITMASK      0x6057  // stats bitmaks as defined in TM100.doc: stats_read

#if L1_GPRS
  // Settings for GPRS test mode:
  #define TM_PDTCH_ARFCN        62  // PDTCH arfcn
  #define TM_MULTISLOT_CLASS     1  // GPRS multi slot class
  #define TM_STAT_GPRS_SLOTS  0x80  // Bit mask for RX stats from PDTCH
  #define TM_RX_ALLOCATION    0x80  // RX slot allocation (bit7->TS0...bit0->TS7)
  #define TM_RX_CODING_SCHEME    1  // RX coding scheme
  #define TM_TX_ALLOCATION    0x80  // TX slot allocation (bit7->TS0...bit0->TS7)
  #define TM_TX_CODING_SCHEME    2  // TX coding scheme
  #define TM_TXPWR_GPRS         15  // GPRS txpwr level
#endif





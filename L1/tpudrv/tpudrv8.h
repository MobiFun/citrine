/****************** Revision Controle System Header ***********************
 *                      GSM Layer 1 software                                
 *              Copyright (c) Texas Instruments 1998                      
 *                                                                        
 *        Filename tpudrv8.h
 *  Copyright 2003 (C) Texas Instruments  
 *                                                                        
 ****************** Revision Controle System Header ***********************/

//SI4133 definitions
#define  WordAdd0000     0x000000 //Main Configuration
#define  AutoPDB         0x000080 //Auto Power Down - Uses PWDNB pin
#define  AuxSel          0x030000 //Auxiliary output pin use = LOCK Detect
#define  WordAdd0011     0x000003 //RF1 N Divider
#define  WordAdd0100     0x000004 //RF2 N Divider
#define  WordAdd0101     0x000005 //IF N Divider
#define  RFPWR           0x000020 //RF LO high power
#define  XPDM            0x000100 //Reference amplifier ON when PWDNB pin = 0

//TRF6053 definitions
#define  Mode0           0x000000
#define  Mode1           0x000001
#define  Mode2           0x000003
#define  Mode3           0x000005
#define  Mode4           0x000007
#define  LNAMixPwrOn     0x000080    //Mode0
#define  VCODiv2PwrOn    0x000040    //Mode0
#define  RXBBIFStgPwrOn  0x000020    //Mode0
#define  OFFStrCalOn     0x000010    //Mode0
#define  VCORDivPwrOn    0x000008    //Mode0
#define  MixLOBuffPwrOn  0x000004    //Mode0
#define  TXStagesPwrOn   0x000002    //Mode0
#define  FreqDetDis      0x000400    //Mode4
#define  IFVCOExternal   0x000200    //Mode4
#define  IFPLLBuffDis    0x000100    //Mode4
#define  LBandLNAExt     0x000080    //Mode4
#define  HBandLNAExt     0x000040    //Mode4
#define  Div2ToRXStgs    0x000020    //Mode4
#define  DivRToTXStgs    0x000010    //Mode4
#define  ChgPPLBNeg      0x000010    //Mode2
#define  ChgPPHBNeg      0x000010    //Mode3
#define  PreCCLBDis      0x000008    //Mode2
#define  PreCCHBDis      0x000008    //Mode3
#define  LNAGainLow      0x000010    //Mode2
#define  BandHigh        0x000008    //Mode1
#define  LowBIF610       0x000020    //Mode2
#define  HighBIF412      0x000020    //Mode3
#define  HighBIF25       0x000040    //Mode3
#define  HighBIF410      0x000060    //Mode3

/*------------------------------------------*/
/*        Download delay values             */
/*------------------------------------------*/
#define TRF6053_DOWNLOAD_TIME   15
#define SYNTH_DOWNLOAD_TIME     20

//--------------------------------------------
// internal tpu timing
//--------------------------------------------

#define DLT_1     1   // 1 tpu instruction = 1 qbit
#define DLT_2     2 
#define DLT_3     3 
#define DLT_4     4
 
#define DLT_1B    4   // 3*move + 1*byte (download)
#define DLT_2B    6   // 4*move + 2*byte
#define DLT_3B    8   // 5*move + 3*byte

#define SL_SU_DELAY1 4  // No. bits to send + load data to shift + send write cmd + 1
#define SL_SU_DELAY2 3  // load data to shift + send write cmd + 1
#define SL_SU_DELAY3 5  // SL_SU_DELAY1 + serialization

/*------------------------------------------*/
/*        Download delay values             */
/*------------------------------------------*/
// 0.9230769 usec ~ 1 qbit i.e. 200 usec is ~ 217 qbit

#define T TPU_CLOCK_RANGE  // TODO: should be a define from L1. 
 
// time below are offset to when BDLENA goes low
#define TRF_R11 (    0 - DLT_1B)           // disable BDLON & BDLENA
#define TRF_R10 (  - 5 - DLT_1B)           // disable TRF6053

// burst data comes here
// time below are offset to when BDLENA goes high
#define TRF_R9  (PROVISION_TIME -   0 - DLT_1B) // enable BDLENA, disable BDLCAL
#define TRF_R8  (PROVISION_TIME -  11 - DLT_1B) // power on RX front end, DC cal. off
#define TRF_R7  (PROVISION_TIME -  65 - DLT_1B) // enable BDLCAL
#define TRF_R6  (PROVISION_TIME -  72 - DLT_1B) // enable BDLON
#define TRF_R5  (PROVISION_TIME -  76 - DLT_1B) // power on receiver, start DC cal.
#define TRF_R4  (PROVISION_TIME -  80 - DLT_2B) // set RX gain & band.
                                                // ADC read, uses min 11 qbit due to 5 wait 
#define TRF_R3  (PROVISION_TIME - 196 - DLT_1B) // power up TRF2253
#define TRF_R1  (PROVISION_TIME - 205 - DLT_3B) // set RF PLL N counter = r1 and IF PLL N counter in TRF2253 = r2

// time below are offset to when BULENA goes low
#define TRF_T13   (  32 - DLT_1B)    // disable PA_ON, BULON, TRF6053
#define TRF_T12   (  18 - DLT_1 )    // disable TSPACT01
#define TRF_T11   (   0 - DLT_1B)    // disable BULENA
#define TRF_T10_1 (- 40 - DLT_1B)    // ADC read
// burst data comes here
// time below are offset to when BULENA goes high
#define TRF_T10   (+  15 - DLT_3)    // enable PA_ON + 2*rfswitch
#define TRF_T9    (-   0 - DLT_1B)   // enable BULENA
#define TRF_T8    (- 109 - DLT_2B)   // power on transceiver 
#define TRF_T7    (- 115 - DLT_1B)   // disable BULCAL
#define TRF_T6    (- 230 - DLT_1B)   // power up TRF2253
#define TRF_T5    (- 233 - DLT_2B)   // set TX band in TRF6053
#define TRF_T3    (- 249 - DLT_3B)   // set RF PLL N counter = t3 and IF PLL N counter in TRF2253 = t4
#define TRF_T2    (- 260 - DLT_1B)   // enable BULCAL
#define TRF_T1    (- 278 - DLT_1B)   // enable BULON

#if (BOARD == 7) || (BOARD == 8) || (BOARD == 9) || (BOARD == 40) || (BOARD == 41) || (BOARD == 42) || (BOARD == 43) || (BOARD == 45)
  #define PA_ON     0x20     // act5
  #define TSPACT01  0x02     // act1

  // RF signals connected to TSPACTX
  #define RX900     0x04     // act10
  #define RX1800    0x08     // act11

  #define TC1_DEVICE_ABB TC1_DEVICE0 
  #define TC1_DEVICE_RF  TC1_DEVICE1
  #define TC1_DEVICE_PLL TC1_DEVICE2
#endif

#if (BOARD == 6)
  #define PA_ON     0x10     // act4

  // RF signals connected to TSPACTX
  #define RX900     0x08     // act11 => needs to be connected to act12
  #define RX1800    0x04     // act10

  #define TC1_DEVICE_ABB TC1_DEVICE0
  #define TC1_DEVICE_PLL TC1_DEVICE1   
  #define TC1_DEVICE_RF  TC1_DEVICE2
#endif

#ifdef TPUDRV8_C
// Function prototypes
SYS_UWORD16 Convert_l1_radio_freq(SYS_UWORD16 radio_freq);

#endif   

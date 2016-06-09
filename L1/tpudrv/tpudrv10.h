/****************** Revision Controle System Header ***********************
 *                      GSM Layer 1 software                                
 *              Copyright (c) Texas Instruments 1998                      
 *                                                                        
 *        Filename tpudrv10.h
 *  Copyright 2003 (C) Texas Instruments  
 *                                                                        
 ****************** Revision Controle System Header ***********************/

#define BIT_0       0x000001 
#define BIT_1       0x000002
#define BIT_2       0x000004
#define BIT_3       0x000008
#define BIT_4       0x000010
#define BIT_5       0x000020
#define BIT_6       0x000040
#define BIT_7       0x000080
#define BIT_8       0x000100
#define BIT_9       0x000200
#define BIT_10      0x000400
#define BIT_11      0x000800
#define BIT_12      0x001000
#define BIT_13      0x002000
#define BIT_14      0x004000
#define BIT_15      0x008000
#define BIT_16      0x010000
#define BIT_17      0x020000
#define BIT_18      0x040000
#define BIT_19      0x080000
#define BIT_20      0x100000
#define BIT_21      0x200000
#define BIT_22      0x400000
#define BIT_23      0x800000


//TRF6150 definitions
#define  MODE0           0x000000
#define  MODE1           0x000001
#define  MODE2           0x000002
#define  MODE3           0x000003
#define  MODE4           0x000004
#define  MODE5           0x000005
#define  MODE6           0x000006
#define  MODE7           0x000007

#define  REGUL_ON             BIT_3    //MODE0
#define  BG_SPEEDUP           BIT_4    //MODE0
#define  RX_ON_CLARA          BIT_5    //MODE0
#define  TX_ON_CLARA          BIT_6    //MODE0
#define  PA_CTRLR_ON          BIT_7    //MODE0
#define  AUX_SYNTH_ON         BIT_8    //MODE0
#define  MAIN_SYNTH_OFF       0x000000 //MODE0
#define  MAIN_SYNTH_ON_RX     BIT_9    //MODE0
#define  MAIN_SYNTH_ON_TX     BIT_10   //MODE0
#define  DCO_COMP_ON          BIT_11   //MODE0
#define  DCO_COMP_RUN         BIT_12   //MODE0
#define  BAND_SELECT_GSM      BIT_13   //MODE0
#define  BAND_SELECT_850      BIT_13   //MODE0
#define  BAND_SELECT_PCS      BIT_14   //MODE0
#define  BAND_SELECT_DCS     (BIT_14 | BIT_13)

#define  RX_RF_GAIN           BIT_15   //MODE0

// MODE1 is only for Receiver gain programming (AGC)

#define  AUX_SHDW_ADD(arfcn) ((arfcn >= 822) && (arfcn <= 885)) ? BIT_3 : 0 //MODE2
#define  AUX_SHDW_RCL         BIT_4    //MODE2
#define  MAIN_FCU_REG_100     BIT_7    //MODE2
#define  PA_CTRL_I_DIOD       BIT_23   //MODE2

//MODE3 
#define  TEST_MODE            BIT_3    //MODE3
#define  HB_OPLL_PRECHARGE    BIT_4    //MODE3

#define  HB_OPLL_CP_CUR_0_125MA   0x000000         //0.125 mA
#define  HB_OPLL_CP_CUR_0_25MA    BIT_5            //0.25 mA
#define  HB_OPLL_CP_CUR_0_5MA     BIT_6            //0.5 mA
#define  HB_OPLL_CP_CUR_1MA       (BIT_6 | BIT_5)  //1 mA
#define  HB_OPLL_CP_CUR_2MA       BIT_7            //2 mA  

#define  LB_OPLL_PRECHARGE    BIT_8    //MODE3

#define  LB_OPLL_CP_CUR_0_125MA   0x000000          //0.125 mA
#define  LB_OPLL_CP_CUR_0_25MA    BIT_9             //0.25 mA
#define  LB_OPLL_CP_CUR_0_5MA     BIT_10            //0.5 mA
#define  LB_OPLL_CP_CUR_1MA       (BIT_10 | BIT_9)  //1 mA
#define  LB_OPLL_CP_CUR_2MA       BIT_11            //2 mA 

#define  CLK_REF              BIT_17   //MODE3
#define  MAIN_VCO_EN          BIT_18   //MODE3
#define  AUX_VCO_EN           BIT_19   //MODE3
#define  EXT_VCO_CONTROL      BIT_20   //MODE3
#define  TEMP_SENSOR_EN       BIT_21   //MODE3

//MODE4
#define  MAIN_TIMER_RX_49_2US BIT_6     //MODE4
#define  MAIN_TIMER_RX_55_35US ( 8 << 3)  //added 30.01.02
#define  MAIN_TIMER_RX_61_5US (10 << 3)
#define  MAIN_TIMER_RX_78_9US (13 << 3)
#define  MAIN_TIMER_RX_91_9US (15 << 3)   
#define  MAIN_TIMER_RX_98_4US (16 << 3)   
#define  MAIN_TIMER_RX_159_9US (26 << 3)   //added 21.08 CR

 
#define  MAIN_TIMER_TX_49_2US BIT_11    //MODE4
#define  MAIN_TIMER_TX_61_5US (10 << 8)  //added 30.01.02
#define  MAIN_TIMER_TX_104US  (17 << 8)   //added for RS 
#define  MAIN_TIMER_TX_98_4US (16 << 8)
#define  MAIN_TIMER_TX_123US  (20 << 8)   //added 21.08 CR

#define  MAIN_CP_CUR_0        0x000000  //MODE4   400uA, 1.6mA
#define  MAIN_CP_CUR_1        BIT_21    //MODE4   400uA, 3.2mA
#define  MAIN_CP_CUR_2        BIT_22    //MODE4   800uA, 3.2mA
#define  MAIN_CP_CUR_3        (BIT_22 | BIT_21)//MODE4  same as 2

#define  FC_60                 (60 << 13)
#define  FC_63                 (63 << 13)
#define  FC_70                 (70 << 13)
#define  FC_100               (100 << 13)
#define  FC_109               (109 << 13)
#define  FC_110               (110 << 13)

//MODE5
#define  SHDW_LOAD            BIT_3     //MODE5 
#define  AUX_PRG_MOD          BIT_4     //MODE5
#define  AUX_PFD              BIT_14    //MODE5

//MODE6
#define  FREQ_CAL_ON          BIT_4     //MODE6
#define  FREQ_CAL_MODE        BIT_5     //MODE6

//MODE7
#define FREQ_CAL_DATA         (0xd << 19)     // 6.15  (00000)-8.88 (01101)-12.66 pF (11111)- modified CR 11.09.01, was   (0xb << 19) 


// RF signals connected to TSPACT  [0..7]
//#define RESET_RF      BIT_0  // act0
#define CLA_SER_ON    BIT_0  // act0 
#define CLA_SER_OFF   0      
#define TXVCO_ON      0      // act3 inverted
#define TXVCO_OFF     BIT_3
#define TX_ON         BIT_5  // act5
#define TX_OFF        0    

// RF signals connected to TSPACT for Titanium v2.2
#if 0
//B-Sample
#define PA900_ON      BIT_2  // signals are inverted therefore PA900_ON act1
#define PA1800_ON     BIT_1  // and PA1800_ON act2
#define PA900_OFF     BIT_1  // 
#define PA1800_OFF    BIT_2  // 
#endif

#if 0
//C-Sample
#define PA900_ON      BIT_1  // signals are inverted therefore PA900_ON act1
#define PA1800_ON     BIT_2  // and PA1800_ON act2
#define PA900_OFF     BIT_2  // 
#define PA1800_OFF    BIT_1  // 
#endif

#if 1
//D-Sample
#define  PA900_ON     BIT_1  // signals are inverted therefore PA900_ON act1
#define PA1800_ON     BIT_2  // and PA1800_ON act2
#define RX1900_ON     0  
#define  PA900_OFF    BIT_2  // 
#define PA1800_OFF    BIT_1  // 
#define RX1900_OFF    BIT_4

//RX_UP/DOWN and TX_UP/DOWN 
#define RU_900     (PA900_OFF | PA1800_OFF | RX1900_OFF)
#define RD_900     (PA900_OFF | PA1800_OFF | RX1900_OFF)
#define TU_900     (PA900_ON  | PA1800_OFF | RX1900_OFF)
#define TD_900     (PA900_OFF | PA1800_OFF | RX1900_OFF)
#define TU_REV_900  (PA900_OFF | PA1800_ON  | RX1900_OFF)

#define RU_850     (PA900_OFF | PA1800_OFF | RX1900_OFF)
#define RD_850     (PA900_OFF | PA1800_OFF | RX1900_OFF)
#define TU_850     (PA900_ON  | PA1800_OFF | RX1900_OFF)
#define TD_850     (PA900_OFF | PA1800_OFF | RX1900_OFF)
#define TU_REV_850  (PA900_OFF | PA1800_ON  | RX1900_OFF)

#define RU_1800    (PA900_OFF | PA1800_OFF | RX1900_OFF)
#define RD_1800    (PA900_OFF | PA1800_OFF | RX1900_OFF)
#define TU_1800    (PA900_OFF | PA1800_ON  | RX1900_OFF)
#define TD_1800    (PA900_OFF | PA1800_OFF | RX1900_OFF)
#define TU_REV_1800 (PA900_ON  | PA1800_OFF | RX1900_OFF)

#define RU_1900    (PA900_OFF | PA1800_OFF | RX1900_ON)
#define RD_1900    (PA900_OFF | PA1800_OFF | RX1900_OFF)
#define TU_1900    (PA900_OFF | PA1800_ON  | RX1900_OFF)
#define TD_1900    (PA900_OFF | PA1800_OFF | RX1900_OFF)
#define TU_REV_1900 (PA900_ON  | PA1800_OFF | RX1900_OFF)


#endif

#define TC1_DEVICE_ABB     TC1_DEVICE0
#define TC1_DEVICE_RF      TC1_DEVICE2 


  #define SL_SU_DELAY1 4  // No. bits to send + load data to shift + send write cmd + 1
  #define SL_SU_DELAY2 3  // load data to shift + send write cmd + 1
  #define SL_SU_DELAY3 5  // SL_SU_DELAY1 + serialization

#define DLT  20  // (TRF6150) DownLoadTime

#define DLT_1     1   // 1 tpu instruction = 1 qbit
#define DLT_2     2 
#define DLT_3     3 
 
#define DLT_1B    4   // 3*move + 1*byte (download)
#define DLT_2B    6   // 4*move + 2*byte
#define DLT_3B    8   // 5*move + 3*byte

//#define crch_timing 420//250//420//0 // CR d.07.08.01 - Temperary movement of Rx and Tx timing for Titanium. Will be set to 0 when new LF is ready.
#define rdt       0//359  // rx delta timing
#define tdt       0//293  // tx delta timing

/*------------------------------------------*/
/*        Download delay values             */
/*------------------------------------------*/
// 0.9230769 usec ~ 1 qbit i.e. 200 usec is ~ 217 qbit
 
#define T TPU_CLOCK_RANGE  
 
#define TRF_I7   334  //qbit 
#define TRF_I8   378  //qbit

// time below are offset to when BDLENA goes low 
#define TRF_R15  (   0 - DLT_1B)   // 0, BDLENA low, needs DLT_1B to execute
#define TRF_R13  ( - 32 - DLT_1B)   // 8 right after, power off transceiver

//burst data comes here
// time below are offset to when BDLENA goes high
#define TRF_R12  (PROVISION_TIME -   0 - DLT_1B)        // BDLENA i/q comes 32qbit later
#define TRF_R10  (PROVISION_TIME -   8 - DLT_1B)        // Set RX/TX switch (not really necessary as the default setting is RX mode)
#define TRF_R9   (PROVISION_TIME -  16 - DLT_2B)  // RX_ON_CLARA
#define TRF_R7   (PROVISION_TIME -  66 - DLT_1B)        // 67qbit duration BDLON + BDLCAL
#define TRF_R6   (PROVISION_TIME -  83 - DLT_1B)        // BDLON, RX_ON_CLARA
#define TRF_R5   (PROVISION_TIME - 172 - DLT_2B - rdt)  // DC offset comp. start LNA ON
//#define TRF_R4   (PROVISION_TIME - 172 - DLT_2B - rdt)  // DC offset comp. LNA
#define TRF_R3   (PROVISION_TIME - 177 - DLT_2B - rdt)  // DC offset comp. GAIN
//l1dmacro_adc_read_rx() called here requires ~ 16 tpuinst
//#define TRF_R2_1 (PROVISION_TIME - 199 - DLT_2B - rdt)  // fc
//#define TRF_R2   (PROVISION_TIME - 199 - DLT_2B - rdt)  // select band
#define TRF_R1   (PROVISION_TIME - 209 - DLT_3B - rdt)  // Main PLL + set of Main PLL FC & CP current


// time below are offset to when BULENA goes low 
#define TRF_T17   (   32 - SL_SU_DELAY2)    // right after, BULON low
//#define TRF_T17   (   32 )    // right after, BULON low
#define TRF_T16   (   26 - DLT_1B)    // Power down Clara
#define TRF_T15   (   14 - DLT_1)     // disable TX_ON
#define TRF_T14   (    0 - DLT_1B)    // BULENA off
#define TRF_T13_3 (-  40 - DLT_1B)    // ADC read
//burst data comes here
// time below are offset to when BULENA goes high
#define TRF_T13_2 (   25 - DLT_1)     //  TX_ON
#define TRF_T13_1 (   17 - DLT_1)     //  set rf switch 
#define TRF_T12   (-   0 - DLT_1B)    //  BULENA Start of TX burst
#define TRF_T10   (-  70 - DLT_3B - tdt)    //  normal speed
#define TRF_T9    (- 121 - DLT_2B - tdt)    //  Power up TXVCO
#define TRF_T8    (- 127 - DLT_1B - tdt)    //  BULON, disable BULCAL
#define TRF_T7    (- 127 - DLT_1B - tdt)    //  131 BULON, disable BULCAL
#define TRF_T6    (- 137 - DLT_3B - tdt)    //  Speed up
#define TRF_T4    (- 249 - DLT_1B - tdt)    //  prog AUX PLL & detector polarity
#define TRF_T3_1  (- 258 - DLT_2B - tdt)    //  fc
#define TRF_T3    (- 258 - DLT_2B - tdt)    //  20 BULON + BULCAL + select band				
#define TRF_T2    (- 267 - DLT_3B - tdt)    //  set of Main PLL FC & CP current		 
#define TRF_T1    (- 277 - DLT_3B - tdt)    //  BULON + Main PLL


/*------------------------------------------*/
/*   Is arfcn in the DCS band (512-885) ?   */
/*------------------------------------------*/
// is working only for GSM and DCS (not PCN)
#define IS_DCS_HIGH(arfcn) (((arfcn >= 576) && (arfcn <= 885))? 1 : 0)    //Changed by CR 30.08.01, was (((arfcn >= 822) && (arfcn <= 885))? 1 : 0)

#ifdef TPUDRV10_C

#endif    



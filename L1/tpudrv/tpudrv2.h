/****************** Revision Controle System Header ***********************
 *                      GSM Layer 1 software                                
 *              Copyright (c) Texas Instruments 1998                      
 *                                                                        
 *        Filename tpudrv2.h
 *  Copyright 2003 (C) Texas Instruments  
 *                                                                        
 ****************** Revision Controle System Header ***********************/

/***********************************************************/
/*                                                         */
/* Used Timing definitions given in "L1_TIME.H"            */
/* --------------------------------------------            */
/*                                                         */
/*  START_RX_FB            STOP_RX_FB                      */
/*  START_RX_SB            STOP_RX_SB                      */
/*  START_RX_SNB           STOP_RX_SNB                     */
/*  START_RX_NNB           STOP_RX_NNB                     */
/*  START_RX_PW_1          STOP_RX_PW_1                    */
/*  START_RX_FB26          STOP_RX_FB26                    */
/*  START_TX_NB            STOP_TX_NB                      */
/*  START_RX_RA            STOP_RX_RA                      */
/*                                                         */
/***********************************************************/

// BB Timings
#define VG_CAL_RX_DELAY 65
#define VG_CAL_TX_DELAY 143
#define VG_BDLON_DELAY  70
#define VG_BULOFF_DELAY 35
#define VG_BULON_DELAY  159
    
#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
  #define OM_CAL_RX_DELAY 65
  #define OM_CAL_TX_DELAY 230
  #define OM_BDLON_DELAY  166
  #define OM_BULOFF_DELAY 35
  #define OM_BULON_DELAY  250

  #define SL_SU_DELAY1 4    
  #define SL_SU_DELAY2 3
#endif

#define RA_TRANSMIS_DURATION ( RA_BURST_DURATION + 46L )   
#define NB_TRANSMIS_DURATION ( NB_BURST_DURATION_UL +  29L )  
#define START_TX_NB  ( 4984L ) // Calibration time is reduced of 4 GSM bit due to a slow APC ramp
#define STOP_TX_NB           ( START_TX_NB + NB_TRANSMIS_DURATION ) 
#define STOP_TX_RA           ( START_TX_RA + RA_TRANSMIS_DURATION ) 


#ifdef TPUDRV2_C

#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
  const unsigned short RF_Sleep[] ={
   /*** Immediate ***/
   TPU_MOVE(TSP_SPI_SET1, TSP_ENA_POS_MSB),
   TPU_MOVE(TSP_SPI_SET2, TSP_ENA_POS_MSB | TSP_ENA_POS),
   0
  };

  const unsigned short RF_Wakeup[] ={
   /*** Immediate ***/
   TPU_MOVE(TSP_SPI_SET1, 0x00),
   TPU_MOVE(TSP_SPI_SET2, TSP_ENA_POS),
   0
  };


/*--------------------------------------------------------------------------------------------------------------*/
/*     Serial link delay for OMEGA. this delay includes                                                         */
/*     TSP register programming and serialization of data to OMEGA                                              */
/*                                                                                                              */
/*                                                                                                              */
/*           4991          4992          4993         4994           4995        4996         4997              */
/*       ----------------------------------------------------------------------------------------------         */
/*      |            |             |             |             |            |            |                      */
/*OMEGA |  AT(4991)  |  Clock conf |   Nb of bit |   Load data | Send write |  Serialization                    */
/*      |            |             |   to shift  |   to shift  |  command   |            |                      */
/*      ----------------------------------------------------------------------------------------------          */
/*      |            |             |             |             |            |            |                      */
/* VEGA |            |             |             |             |            |   AT(4996) |   TSPACT             */
/*      |            |             |             |             |            |            |                      */
/*      ------------------------------------------------------------------------------------------|---          */
/*      <------------------------------------------------------------------>                      |             */
/*                       SL_SU_DELAY1                                                             |             */
/*                                                                                                V             */ 
/*                                                                                         ACTION ON WINDOW     */
/*                                                                                                              */
/*  When the TSP port is already configured is not necessary to configure the clock and the number of bits      */
/*                                                                                                              */
/*                                                                                                              */
/*           4998          4999            0            1           2                                           */
/*       -------------------------------------------------------------------                                    */
/*      |            |             |             |            |            |                                    */
/*OMEGA |  AT(4998)  |  Load data  | Send write  |  Serialization          |                                    */
/*      |            |   to shift  |   command   |            |            |                                    */
/*      ----------------------------------------------------------------------                                  */
/*      |            |             |             |            |            |                                    */
/* VEGA |            |             |             |  AT(4996)  |   TSPACT                                        */
/*      |            |             |             |            |            |                                    */
/*      ------------------------------------------------------------|-------                                    */
/*      <--------------------------------------->                   |                                           */
/*                       SL_SU_DELAY2                               |                                           */
/*                                                                  V                                           */ 
/*                                                            ACTION ON WINDOW                                  */
/*                                                                                                              */
/*                                                                                                              */
/* NOTE : WITH THIS IMPLEMENTATION THE OMEGA SCENARIO ANTICIPATES THE ACTION ON WINDOW SIGNAL OF 347 ns.        */   
/*        ANYWAY ACTION IS TAKEN IN THE SAME QB INTERVAL                                                        */ 
/*                                                                                                              */
/*                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------*/

                                                                                                             

/***********************************************************/
/*    BASEBAND TPU SCENARIOS    FOR OMEGA                  */
/***********************************************************/

  #if ((CHIPSET == 4) || (CHIPSET == 7)  || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET  ==  11))
    // Samson TPU scenario: add 1 bit to reception window for DMA thres = 2
    const SYS_UWORD16 VG_DlNormalBurst [] = {
                                            
    TPU_AT    (START_RX_SNB -VG_BDLON_DELAY - SL_SU_DELAY1 ),            // AT(4991) 
//  TPU_MOVE  (TSP_SPI_SET1, TSP_CLK_RISE), 
    TPU_MOVE  (TSP_CTRL1,6),
    TPU_MOVE  (TSP_TX_REG_1,BDLON),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),                     


    TPU_AT    (START_RX_SNB - VG_CAL_RX_DELAY - SL_SU_DELAY2),           // AT(4998)
    TPU_MOVE  (TSP_TX_REG_1,BDLON | BDLCAL),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),

    TPU_AT    (START_RX_SNB - SL_SU_DELAY2),                             // AT(63)
    TPU_MOVE  (TSP_TX_REG_1,BDLON | BDLENA),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),

    TPU_AT    (STOP_RX_SNB - SL_SU_DELAY2),                              // AT(699)
    TPU_MOVE  (TSP_TX_REG_1,0x00),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),

    0
    };


    // HERCULES TPU scenario  
    const SYS_UWORD16 VG_DlFrequencyBurstIdle [] = {
                                              
    TPU_AT    (START_RX_FB - VG_BDLON_DELAY -SL_SU_DELAY1 ),             // AT(4991)
//  TPU_MOVE  (TSP_SPI_SET1, TSP_CLK_RISE),
    TPU_MOVE  (TSP_CTRL1,6),
    TPU_MOVE  (TSP_TX_REG_1,BDLON),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),


    TPU_AT    (START_RX_FB - VG_CAL_RX_DELAY -SL_SU_DELAY2),            // AT(4998)
    TPU_MOVE  (TSP_TX_REG_1,BDLON | BDLCAL),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),


    TPU_AT    (START_RX_FB - SL_SU_DELAY2),                              // AT(63)
    TPU_MOVE  (TSP_TX_REG_1,BDLON | BDLENA),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),

    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),

    TPU_AT    (STOP_RX_FB - SL_SU_DELAY2),                               // AT(2119)
    TPU_MOVE  (TSP_TX_REG_1,0X00),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),

    0
    };


  #else
  /* HERCULES TPU scenario */ 
   
    const SYS_UWORD16 VG_DlNormalBurst [] = {
                                            
    TPU_AT    (START_RX_SNB -VG_BDLON_DELAY - SL_SU_DELAY1 ),            // AT(4991) 
//  TPU_MOVE  (TSP_SPI_SET1, TSP_CLK_RISE), 
    TPU_MOVE  (TSP_CTRL1,6),
    TPU_MOVE  (TSP_TX_REG_1,BDLON),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),                     


    TPU_AT    (START_RX_SNB - VG_CAL_RX_DELAY - SL_SU_DELAY2),           // AT(4998)
    TPU_MOVE  (TSP_TX_REG_1,BDLON | BDLCAL),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),

    TPU_AT    (START_RX_SNB - SL_SU_DELAY2),                             // AT(63)
    TPU_MOVE  (TSP_TX_REG_1,BDLON | BDLENA),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),

    TPU_AT    (STOP_RX_SNB - SL_SU_DELAY2),                              // AT(699)
    TPU_MOVE  (TSP_TX_REG_1,0x00),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),

    0
    };


    // HERCULES TPU scenario  
    const SYS_UWORD16 VG_DlFrequencyBurstIdle [] = {
                                              
    TPU_AT    (START_RX_FB - VG_BDLON_DELAY -SL_SU_DELAY1 ),             // AT(4991)
//  TPU_MOVE  (TSP_SPI_SET1, TSP_CLK_RISE),
    TPU_MOVE  (TSP_CTRL1,6),
    TPU_MOVE  (TSP_TX_REG_1,BDLON),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),


    TPU_AT    (START_RX_FB - VG_CAL_RX_DELAY -SL_SU_DELAY2),            // AT(4998)
    TPU_MOVE  (TSP_TX_REG_1,BDLON | BDLCAL),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),


    TPU_AT    (START_RX_FB - SL_SU_DELAY2),                              // AT(63)
    TPU_MOVE  (TSP_TX_REG_1,BDLON | BDLENA),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),

    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),
    TPU_AT    (0),

    TPU_AT    (STOP_RX_FB - SL_SU_DELAY2),                               // AT(2119)
    TPU_MOVE  (TSP_TX_REG_1,0X00),                     
    TPU_MOVE  (TSP_CTRL2, TC2_WR),

    0
    };



  #endif



  // HERCULES TPU scenario for Omega windows reset  
  const SYS_UWORD16 VG_Omega_win_reset[] = {
                       
  TPU_MOVE  (TSP_SPI_SET1, TSP_CLK_RISE),                     
  TPU_MOVE  (TSP_CTRL1,6),
  TPU_MOVE  (TSP_TX_REG_1,0x00),                     
  TPU_MOVE  (TSP_CTRL2, TC2_WR),
  0
  };

  #endif

#else
  extern const SYS_UWORD16 VG_DlNormalBurst[];
  extern const SYS_UWORD16 VG_DlFrequencyBurstIdle[];
#endif


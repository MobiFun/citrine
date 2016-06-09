/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_MFTAB.H
 *
 *        Filename l1_mftab.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

/***********************************************************
 * Content:
 *   This file contains the MultiFrame tables for all L1S
 *   basic tasks.
 ***********************************************************/

/*******************************************************************************************/
/* Multiframe Blocks for Dynamic MFTAB Building purpose.                                   */
/*******************************************************************************************/
// Multiframe table size....

#ifndef L1_MFTAB_H
#define L1_MFTAB_H

  #define BLOC_FBNEW_SIZE    14 + 2   // FB.

  #define BLOC_SB2_SIZE       5 + 2   // SB2.
#if ((REL99 == 1) && (FF_BHO == 1))
#define BLOC_FBSB_SIZE     12 + 2 + 2 + 2    // FB + SB + AGC
#endif
  #define BLOC_SBCONF_SIZE    4 + 2   // SBCONF.
  #define BLOC_BCCHN_SIZE     7 + 2   // BCCHN.
  #define BLOC_BCCHN_TOP_SIZE 7 + 2   // BCCHN_TOP (BCCHN top priority)

  #define BLOC_SYNCHRO_SIZE         1       // SYNC.
  #define BLOC_ADC_SIZE             1       // ADC in CS_MODE0
  #define BLOC_ABORT_SIZE           3       // ABORT.
  #define BLOC_RAACC_SIZE           3       // RAACC.
  #define S_RECT4_SIZE              6       // All "rectangular 4" serving tasks: NP/EP/BCCHS/ALLC.
  #define BLOC_TCHT_SIZE            3       // TCHTF / TCHTH / TCHD.
  #define BLOC_TCHA_SIZE            3       // TCHA.
  #define BLOC_SMSCB_SIZE           6       // SMSCB.
  #define BLOC_FB51_SIZE           14       // FB51.
  #define BLOC_SB51_SIZE            4       // SB51.
  #define BLOC_SBCNF51_SIZE         4       // SBCNF51.
  #define BLOC_FB26_SIZE            4       // FB26.
  #define BLOC_SB26_SIZE            5       // SB26.
  #define BLOC_SBCNF26_SIZE         5       // SBCNF26.
  #define BLOC_HWTEST_SIZE          4       // HWTEST.
  #define BLOC_DUL_ADL_MIXED_SIZED  7
  #if (L1_GPRS)
    #define BLOC_BCCHN_TRAN_SIZE     7  // BCCHN_TRAN.
  #endif


          
  #ifdef L1_ASYNC_C
    /*----------------------------------------------------*/
    /* TASK: Frequency Burst search...                    */
    /*----------------------------------------------------*/
    const T_FCT  BLOC_FBNEW[] =      
    { 
      {l1s_ctrl_msagc,FBNEW,NO_PAR},                               {NULL,NO_PAR,NO_PAR}, // frame 1
                                                                   {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_msagc,FBNEW,NO_PAR},{l1s_ctrl_fb,FBNEW,NO_PAR},    {NULL,NO_PAR,NO_PAR}, // frame 3
                                                                   {NULL,NO_PAR,NO_PAR}, // frame 4
                                    {l1s_read_mon_result,FBNEW, 1},{NULL,NO_PAR,NO_PAR}, // frame 5
                                    {l1s_read_mon_result,FBNEW, 2},{NULL,NO_PAR,NO_PAR}, // frame 6
                                    {l1s_read_mon_result,FBNEW, 3},{NULL,NO_PAR,NO_PAR}, // frame 7
                                    {l1s_read_mon_result,FBNEW, 4},{NULL,NO_PAR,NO_PAR}, // frame 8
                                    {l1s_read_mon_result,FBNEW, 5},{NULL,NO_PAR,NO_PAR}, // frame 9
                                    {l1s_read_mon_result,FBNEW, 6},{NULL,NO_PAR,NO_PAR}, // frame 10
                                    {l1s_read_mon_result,FBNEW, 7},{NULL,NO_PAR,NO_PAR}, // frame 11
                                    {l1s_read_mon_result,FBNEW, 8},{NULL,NO_PAR,NO_PAR}, // frame 12
                                    {l1s_read_mon_result,FBNEW, 9},{NULL,NO_PAR,NO_PAR}, // frame 13
                                    {l1s_read_mon_result,FBNEW,10},{NULL,NO_PAR,NO_PAR}, // frame 14
                                    {l1s_read_mon_result,FBNEW,11},{NULL,NO_PAR,NO_PAR}, // frame 15
                                    {l1s_read_mon_result,FBNEW,12},{NULL,NO_PAR,NO_PAR}  // frame 16
    };
                                             
    /*----------------------------------------------------*/
    /* TASK: SB2, New Synchro Burst search...             */
    /*----------------------------------------------------*/
    /*       C W R           -> AGC                       */  
    /*           C W W R     -> 1st SB                    */  
    /*             C W W R   -> 2nd SB                    */  
    /*----------------------------------------------------*/
    const T_FCT BLOC_SB2[] = 
    {
      {l1s_ctrl_msagc,SB2,NO_PAR},                              {NULL,NO_PAR,NO_PAR}, // frame 1
                                                                {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_msagc,SB2,NO_PAR}, {l1s_ctrl_sbgen,SB2,1},      {NULL,NO_PAR,NO_PAR}, // frame 3
                                   {l1s_ctrl_sbgen,SB2,2},      {NULL,NO_PAR,NO_PAR}, // frame 4
                                                                {NULL,NO_PAR,NO_PAR}, // frame 5
                                   {l1s_read_mon_result,SB2,1}, {NULL,NO_PAR,NO_PAR}, // frame 6
                                   {l1s_read_mon_result,SB2,2}, {NULL,NO_PAR,NO_PAR}  // frame 7
    };

    /*----------------------------------------------------*/
    /* TASK: SBCONF, Synchro confirmation.                */
    /*----------------------------------------------------*/
    /*       C W R           -> AGC                       */  
    /*           C W W R     -> SBCONF                    */  
    /*----------------------------------------------------*/
    const T_FCT BLOC_SBCONF[] = 
    {
      {l1s_ctrl_msagc,SBCONF,1},                               {NULL,NO_PAR,NO_PAR}, // frame 1
                                                               {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_msagc,SBCONF,1},{l1s_ctrl_sbgen,SBCONF,1},     {NULL,NO_PAR,NO_PAR}, // frame 3
                                                               {NULL,NO_PAR,NO_PAR}, // frame 4
                                                               {NULL,NO_PAR,NO_PAR}, // frame 5
                                {l1s_read_mon_result,SBCONF,1},{NULL,NO_PAR,NO_PAR}  // frame 6
    };


#if ((REL99 == 1) && (FF_BHO == 1))
  /*----------------------------------------------------*/
  /* TASK: FBSB Frequency + Synchro Bursts Search...    */
  /*----------------------------------------------------*/
  const T_FCT  BLOC_FBSB[] =      
  { 
    {l1s_ctrl_msagc,FBSB,NO_PAR},                               {NULL,NO_PAR,NO_PAR}, // frame 1
                                  {NULL,NO_PAR,NO_PAR}, // frame 2
    {l1s_read_msagc,FBSB,NO_PAR},{l1s_ctrl_fbsb,FBSB,NO_PAR},    {NULL,NO_PAR,NO_PAR}, // frame 3
                                  {NULL,NO_PAR,NO_PAR}, // frame 4
    {l1s_read_mon_result,FBSB, 1},{NULL,NO_PAR,NO_PAR}, // frame 5
    {l1s_read_mon_result,FBSB, 2},{NULL,NO_PAR,NO_PAR}, // frame 6
    {l1s_read_mon_result,FBSB, 3},{NULL,NO_PAR,NO_PAR}, // frame 7
    {l1s_read_mon_result,FBSB, 4},{NULL,NO_PAR,NO_PAR}, // frame 8
    {l1s_read_mon_result,FBSB, 5},{NULL,NO_PAR,NO_PAR}, // frame 9
    {l1s_read_mon_result,FBSB, 6},{NULL,NO_PAR,NO_PAR}, // frame 10
    {l1s_read_mon_result,FBSB, 7},{NULL,NO_PAR,NO_PAR}, // frame 11
    {l1s_read_mon_result,FBSB, 8},{NULL,NO_PAR,NO_PAR}, // frame 12
    {l1s_read_mon_result,FBSB, 9},{NULL,NO_PAR,NO_PAR}, // frame 13
    {l1s_read_mon_result,FBSB,10},{NULL,NO_PAR,NO_PAR}, // frame 14
    {l1s_read_mon_result,FBSB,11},{NULL,NO_PAR,NO_PAR}, // frame 15
    {l1s_read_mon_result,FBSB,12},{NULL,NO_PAR,NO_PAR}, // frame 16
    {l1s_read_mon_result,FBSB,13},{NULL,NO_PAR,NO_PAR}, // frame 17
    {l1s_read_mon_result,FBSB,14},{NULL,NO_PAR,NO_PAR}  // frame 18
  };
#endif

    /*----------------------------------------------------*/
    /* TASK: Serving cell Normal BCCH reading.            */
    /*----------------------------------------------------*/
    /* frame 1 2 3 4 5 6                                  */  
    /*       | | | | | |                                  */  
    /*       C W R | | |     -> burst 1                   */  
    /*         C W R | |     -> burst 2                   */  
    /*           C W R |     -> burst 3                   */  
    /*             C W R     -> burst 4                   */  
    /*----------------------------------------------------*/
    const T_FCT BLOC_NBCCHS[] =    
    { 
      {l1s_ctrl_snb_dl,NBCCHS,BURST_1},                                 {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_ctrl_snb_dl,NBCCHS,BURST_2},                                 {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_snb_dl,NBCCHS,BURST_1},{l1s_ctrl_snb_dl,NBCCHS,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_read_snb_dl,NBCCHS,BURST_2},{l1s_ctrl_snb_dl,NBCCHS,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 4
                                       {l1s_read_snb_dl,NBCCHS,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 5
                                       {l1s_read_snb_dl,NBCCHS,BURST_4},{NULL,NO_PAR,NO_PAR}  // frame 6
    };

    /*----------------------------------------------------*/
    /* TASK: Serving cell Extended BCCH reading.          */
    /*----------------------------------------------------*/
    /* frame 1 2 3 4 5 6                                  */  
    /*       | | | | | |                                  */  
    /*       C W R | | |     -> burst 1                   */  
    /*         C W R | |     -> burst 2                   */  
    /*           C W R |     -> burst 3                   */  
    /*             C W R     -> burst 4                   */  
    /*----------------------------------------------------*/
    const T_FCT BLOC_EBCCHS[] =    
    { 
      {l1s_ctrl_snb_dl,EBCCHS,BURST_1},                                 {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_ctrl_snb_dl,EBCCHS,BURST_2},                                 {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_snb_dl,EBCCHS,BURST_1},{l1s_ctrl_snb_dl,EBCCHS,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_read_snb_dl,EBCCHS,BURST_2},{l1s_ctrl_snb_dl,EBCCHS,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 4
                                       {l1s_read_snb_dl,EBCCHS,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 5
                                       {l1s_read_snb_dl,EBCCHS,BURST_4},{NULL,NO_PAR,NO_PAR}  // frame 6
    };

    /*----------------------------------------------------*/
    /* TASK: Neighbour Cell SYStem info reading.          */
    /*----------------------------------------------------*/
    /*   C W R                 -> AGC                     */  
    /*       C W W W W W R     -> all bursts              */  
    /*----------------------------------------------------*/
    const T_FCT BLOC_BCCHN[] =     
    { 
      {l1s_ctrl_msagc,BCCHN,NO_PAR},                            {NULL,NO_PAR,NO_PAR}, // frame 1
                                                                {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_msagc,BCCHN,NO_PAR},{l1s_ctrl_nnb,BCCHN,NO_PAR},{NULL,NO_PAR,NO_PAR}, // frame 3
                                                                {NULL,NO_PAR,NO_PAR}, // frame 4 
                                                                {NULL,NO_PAR,NO_PAR}, // frame 5 
                                                                {NULL,NO_PAR,NO_PAR}, // frame 6 
                                                                {NULL,NO_PAR,NO_PAR}, // frame 7 
                                                                {NULL,NO_PAR,NO_PAR}, // frame 8  
                                    {l1s_read_nnb,BCCHN,NO_PAR},{NULL,NO_PAR,NO_PAR}  // frame 9              
    }; 

    /*----------------------------------------------------*/
    /* TASK: Neighbour Cell SYStem info reading.          */
    /*----------------------------------------------------*/
    /*   C W R                 -> AGC                     */  
    /*       C W W W W W R     -> all bursts              */  
    /*----------------------------------------------------*/
    const T_FCT BLOC_BCCHN_TOP[] =     
    { 
      {l1s_ctrl_msagc,BCCHN_TOP,NO_PAR},                                {NULL,NO_PAR,NO_PAR}, // frame 1
                                                                        {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_msagc,BCCHN_TOP,NO_PAR},{l1s_ctrl_nnb,BCCHN_TOP,NO_PAR},{NULL,NO_PAR,NO_PAR}, // frame 3
                                                                        {NULL,NO_PAR,NO_PAR}, // frame 4 
                                                                        {NULL,NO_PAR,NO_PAR}, // frame 5 
                                                                        {NULL,NO_PAR,NO_PAR}, // frame 6 
                                                                        {NULL,NO_PAR,NO_PAR}, // frame 7 
                                                                        {NULL,NO_PAR,NO_PAR}, // frame 8  
                                        {l1s_read_nnb,BCCHN_TOP,NO_PAR},{NULL,NO_PAR,NO_PAR}  // frame 9              
    }; 

    /*----------------------------------------------------*/
    /* TASK: Neighbour Cell SYStem info reading.          */
    /*       for packet transfer mode                     */
    /*----------------------------------------------------*/
    /*       C W W W W W R     -> all bursts              */  
    /*----------------------------------------------------*/
  #if (L1_GPRS)
    const T_FCT BLOC_BCCHN_TRAN[] =     
    { 
                                    {l1s_ctrl_nnb,BCCHN_TRAN,NO_PAR},{NULL,NO_PAR,NO_PAR}, // frame 1
                                                                     {NULL,NO_PAR,NO_PAR}, // frame 2 
                                                                     {NULL,NO_PAR,NO_PAR}, // frame 3 
                                                                     {NULL,NO_PAR,NO_PAR}, // frame 4 
                                                                     {NULL,NO_PAR,NO_PAR}, // frame 5 
                                                                     {NULL,NO_PAR,NO_PAR}, // frame 6  
                                    {l1s_read_nnb,BCCHN_TRAN,NO_PAR},{NULL,NO_PAR,NO_PAR}  // frame 7              
    }; 
  #endif

    /*----------------------------------------------------*/
    /* TASK: Synchronization (camp on a new serving cell) */
    /*----------------------------------------------------*/
    const T_FCT  BLOC_SYNCHRO[] =      
    { 
      {l1s_new_synchro,NO_PAR,NO_PAR},{NULL,NO_PAR,NO_PAR} // frame 1
    }; 


    /*----------------------------------------------------*/
    /* TASK: ADC measurement in CS_MODE0                  */
    /*       C                                            */
    /* the ADC is performed inside the frame and the      */
    /* result is red in the same frame due to an          */
    /* Interrupt (handle by Riviera)                      */
    /*----------------------------------------------------*/
    const T_FCT  BLOC_ADC[] =      
    { 
      {l1s_ctrl_ADC,NO_PAR,NO_PAR},{NULL,NO_PAR,NO_PAR}  // frame 1
    };

             
    /*----------------------------------------------------*/
    /* TASK: Short Message Service Cell Broadcast         */
    /*----------------------------------------------------*/
    /* frame   1 2 3 4 5 6                                */  
    /*         | | | | | |                                */  
    /*         C W R | | | -> hopp. + burst 1             */  
    /*           C W R | | -> hopp. + burst 2             */  
    /*             C W R | -> hopp. + burst 3             */  
    /*               C W R -> hopp. + burst 4 + Synch back*/  
    /*----------------------------------------------------*/
    const T_FCT  BLOC_SMSCB[] =      
    { 
      {l1s_hopping_algo,SMSCB,NO_PAR},{l1s_ctrl_smscb, SMSCB,BURST_1},                                {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_hopping_algo,SMSCB,NO_PAR},{l1s_ctrl_smscb, SMSCB,BURST_2},                                {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_hopping_algo,SMSCB,NO_PAR},{l1s_read_snb_dl,SMSCB,BURST_1},{l1s_ctrl_smscb, SMSCB,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_hopping_algo,SMSCB,NO_PAR},{l1s_read_snb_dl,SMSCB,BURST_2},{l1s_ctrl_smscb, SMSCB,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 4
                                                                      {l1s_read_snb_dl,SMSCB,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 5
                                                                      {l1s_read_snb_dl,SMSCB,BURST_4},{NULL,NO_PAR,NO_PAR}  // frame 6
    }; 
    
    /*----------------------------------------------------*/
    /* TASK: Normal Paging...                             */
    /*----------------------------------------------------*/
    /* frame 1 2 3 4 5 6                                  */  
    /*       | | | | | |                                  */  
    /*       C W R | | |     -> burst 1                   */  
    /*         C W R | |     -> burst 2                   */  
    /*           C W R |     -> burst 3                   */  
    /*             C W R     -> burst 4                   */  
    /*----------------------------------------------------*/
    const T_FCT  BLOC_NP[] =      
    { 
      {l1s_ctrl_snb_dl,NP,BURST_1},                             {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_ctrl_snb_dl,NP,BURST_2},                             {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_snb_dl,NP,BURST_1},{l1s_ctrl_snb_dl,NP,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_read_snb_dl,NP,BURST_2},{l1s_ctrl_snb_dl,NP,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 4
                                   {l1s_read_snb_dl,NP,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 5
                                   {l1s_read_snb_dl,NP,BURST_4},{NULL,NO_PAR,NO_PAR}  // frame 6
    }; 
             
    /*----------------------------------------------------*/
    /* TASK: Extended Paging task...                      */
    /*----------------------------------------------------*/
    /* frame 1 2 3 4 5 6                                  */  
    /*       | | | | | |                                  */  
    /*       C W R | | |     -> burst 1                   */  
    /*         C W R | |     -> burst 2                   */  
    /*           C W R |     -> burst 3                   */  
    /*             C W R     -> burst 4                   */  
    /*----------------------------------------------------*/
    const T_FCT  BLOC_EP[] =      
    { 
      {l1s_ctrl_snb_dl,EP,BURST_1},                             {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_ctrl_snb_dl,EP,BURST_2},                             {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_snb_dl,EP,BURST_1},{l1s_ctrl_snb_dl,EP,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_read_snb_dl,EP,BURST_2},{l1s_ctrl_snb_dl,EP,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 4
                                   {l1s_read_snb_dl,EP,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 5
                                   {l1s_read_snb_dl,EP,BURST_4},{NULL,NO_PAR,NO_PAR}  // frame 6
    }; 

    /*----------------------------------------------------*/
    /* TASK: All CCCH reading task...                     */
    /*----------------------------------------------------*/
    /* frame 1 2 3 4 5 6                                  */  
    /*       | | | | | |                                  */  
    /*       C W R | | |     -> burst 1                   */  
    /*         C W R | |     -> burst 2                   */  
    /*           C W R |     -> burst 3                   */  
    /*             C W R     -> burst 4                   */  
    /*----------------------------------------------------*/
    const T_FCT  BLOC_ALLC[] =      
    { 
      {l1s_ctrl_snb_dl,ALLC,BURST_1},                               {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_ctrl_snb_dl,ALLC,BURST_2},                               {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_snb_dl,ALLC,BURST_1},{l1s_ctrl_snb_dl,ALLC,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_read_snb_dl,ALLC,BURST_2},{l1s_ctrl_snb_dl,ALLC,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 4
                                     {l1s_read_snb_dl,ALLC,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 5
                                     {l1s_read_snb_dl,ALLC,BURST_4},{NULL,NO_PAR,NO_PAR}  // frame 6
    }; 

    /*----------------------------------------------------*/
    /* TASK: SDCCH                                        */
    /*----------------------------------------------------*/
    /* frame 1 2 3 4 5 6                                  */  
    /*       | | | | | |                                  */  
    /*       C W R | | |     -> burst 1                   */  
    /*         C W R | |     -> burst 2                   */  
    /*           C W R |     -> burst 3                   */  
    /*             C W R     -> burst 4                   */  
    /*----------------------------------------------------*/
    const T_FCT  BLOC_DDL[] = 
    { 
      {l1s_hopping_algo,DDL,NO_PAR},{l1s_ctrl_snb_dl,  DDL,BURST_1},                                {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_hopping_algo,DDL,NO_PAR},{l1s_ctrl_snb_dl,  DDL,BURST_2},                                {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_hopping_algo,DDL,NO_PAR},{l1s_read_dedic_dl,DDL,BURST_1},{l1s_ctrl_snb_dl,  DDL,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_hopping_algo,DDL,NO_PAR},{l1s_read_dedic_dl,DDL,BURST_2},{l1s_ctrl_snb_dl,  DDL,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 4
                                                                    {l1s_read_dedic_dl,DDL,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 5
                                                                    {l1s_read_dedic_dl,DDL,BURST_4},{NULL,NO_PAR,NO_PAR}  // frame 6
    }; 

    const T_FCT  BLOC_DUL[] = 
    { 
      {l1s_hopping_algo,DUL,NO_PAR},{l1s_ctrl_snb_ul,   DUL,BURST_1},                                 {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_hopping_algo,DUL,NO_PAR},{l1s_ctrl_snb_ul,   DUL,BURST_2},                                 {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_hopping_algo,DUL,NO_PAR},{l1s_read_tx_result,DUL,BURST_1},{l1s_ctrl_snb_ul,   DUL,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_hopping_algo,DUL,NO_PAR},{l1s_read_tx_result,DUL,BURST_2},{l1s_ctrl_snb_ul,   DUL,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 4
                                                                     {l1s_read_tx_result,DUL,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 5
                                                                     {l1s_read_tx_result,DUL,BURST_4},{NULL,NO_PAR,NO_PAR}  // frame 6
    }; 

    const T_FCT  BLOC_ADL[] = 
    { 
      {l1s_hopping_algo,ADL,NO_PAR},{l1s_ctrl_snb_dl,  ADL,BURST_1},                                {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_hopping_algo,ADL,NO_PAR},{l1s_ctrl_snb_dl,  ADL,BURST_2},                                {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_hopping_algo,ADL,NO_PAR},{l1s_read_dedic_dl,ADL,BURST_1},{l1s_ctrl_snb_dl,  ADL,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_hopping_algo,ADL,NO_PAR},{l1s_read_dedic_dl,ADL,BURST_2},{l1s_ctrl_snb_dl,  ADL,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 4
                                                                    {l1s_read_dedic_dl,ADL,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 5
                                                                    {l1s_read_dedic_dl,ADL,BURST_4},{NULL,NO_PAR,NO_PAR}  // frame 6
    }; 

    const T_FCT  BLOC_AUL[] = 
    { 
      {l1s_hopping_algo,AUL,NO_PAR},{l1s_ctrl_snb_ul,   AUL,BURST_1},                                 {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_hopping_algo,AUL,NO_PAR},{l1s_ctrl_snb_ul,   AUL,BURST_2},                                 {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_hopping_algo,AUL,NO_PAR},{l1s_read_tx_result,AUL,BURST_1},{l1s_ctrl_snb_ul,   AUL,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_hopping_algo,AUL,NO_PAR},{l1s_read_tx_result,AUL,BURST_2},{l1s_ctrl_snb_ul,   AUL,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 4
                                                                     {l1s_read_tx_result,AUL,BURST_3},{NULL,NO_PAR,NO_PAR}, // frame 5
                                                                     {l1s_read_tx_result,AUL,BURST_4},{NULL,NO_PAR,NO_PAR}  // frame 6
    }; 
                  
    /*-----------------------------------------------------------------------*/
    /* SPECIAL CASE: (ADL4,DDL4),(ADL5,DDL5),(ADL6,DDL6).                    */
    /*-----------------------------------------------------------------------*/
    /* frame 1        2        3        4        5        6        7         */  
    /*       |        |        |        |        |        |        |         */  
    /*       C(DUL,1) W(DUL,1) R(DUL,1) |        |        |        |         */
    /*                C(ADL,1) W(ADL,1) R(ADL,1) |        |        |         */
    /*                C(DUL,2) W(DUL,2) R(DUL,2) |        |        |         */
    /*                         C(ADL,2) W(ADL,2) R(ADL,2) |        |         */
    /*                         C(DUL,3) W(DUL,3) R(DUL,3) |        |         */
    /*                                  C(ADL,3) W(ADL,3) R(ADL,3) |         */
    /*                                  C(DUL,4) W(DUL,4) R(DUL,4) |         */
    /*                                           C(ADL,4) W(ADL,4) R(ADL,4)  */
    /*-----------------------------------------------------------------------*/
    const T_FCT  BLOC_DUL_ADL_MIXED[] = 
    { 
      {l1s_hopping_algo,AUL,NO_PAR},{l1s_ctrl_snb_ul,   DUL,BURST_1},                                                                                                  {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_hopping_algo,ADL,NO_PAR},{l1s_ctrl_snb_dl,   ADL,BURST_1},{l1s_ctrl_snb_ul,   DUL,BURST_2},                                                                 {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_hopping_algo,ADL,NO_PAR},{l1s_read_tx_result,DUL,BURST_1},{l1s_ctrl_snb_dl,   ADL,BURST_2},{l1s_ctrl_snb_ul,  DUL,BURST_3},                                 {NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_hopping_algo,ADL,NO_PAR},{l1s_read_dedic_dl, ADL,BURST_1},{l1s_read_tx_result,DUL,BURST_2},{l1s_ctrl_snb_dl,  ADL,BURST_3},{l1s_ctrl_snb_ul,   DUL,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 4
      {l1s_hopping_algo,ADL,NO_PAR},{l1s_read_dedic_dl, ADL,BURST_2},{l1s_read_tx_result,DUL,BURST_3},{l1s_ctrl_snb_dl,  ADL,BURST_4},                                 {NULL,NO_PAR,NO_PAR}, // frame 5
                                                                                                      {l1s_read_dedic_dl,ADL,BURST_3},{l1s_read_tx_result,DUL,BURST_4},{NULL,NO_PAR,NO_PAR}, // frame 6
                                                                                                      {l1s_read_dedic_dl,ADL,BURST_4},                                 {NULL,NO_PAR,NO_PAR}  // frame 7
    };
    
    /*----------------------------------------------------*/
    /* ABORT: used to abort a running task when a new     */
    /* task with higher priority occurs.                  */
    /*----------------------------------------------------*/
    const T_FCT  BLOC_ABORT[] =      
    { 
      {l1s_abort,NO_PAR,NO_PAR},     {NULL,NO_PAR,NO_PAR}, // frame 1
      {l1s_reset_tx_ptr,NO_PAR,NO_PAR}, {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_dummy,NO_PAR,NO_PAR},{NULL,NO_PAR,NO_PAR}  // frame 3
    }; 
     
    /*----------------------------------------------------*/
    /* TASK: RACH in access mode...                       */
    /*----------------------------------------------------*/
    const T_FCT  BLOC_RAACC[] =      
    { 
      {l1s_ctrl_rach,RAACC,NO_PAR},     {NULL,NO_PAR,NO_PAR}, // frame 1
                                        {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_tx_result,RAACC,NO_PAR},{NULL,NO_PAR,NO_PAR}  // frame 3
    }; 

    /*----------------------------------------------------*/
    /* TASK: TCH                                          */
    /*----------------------------------------------------*/
    /*       C W R                                        */  
    /*----------------------------------------------------*/
    const T_FCT  BLOC_TCHTF[] =      
    {
      {l1s_hopping_algo,TCHTF,NO_PAR},{l1s_ctrl_tchtf,TCHTF,NO_PAR},   {NULL,NO_PAR}, // frame 1
                                                                       {NULL,NO_PAR}, // frame 2
                                     {l1s_read_dedic_dl,TCHTF,NO_PAR}, {NULL,NO_PAR}  // frame 3
    };
    
    const T_FCT  BLOC_TCHTH[] =      
    {
      {l1s_hopping_algo,TCHTH,NO_PAR},{l1s_ctrl_tchth,TCHTH,NO_PAR},   {NULL,NO_PAR}, // frame 1
                                                                       {NULL,NO_PAR}, // frame 2
                                     {l1s_read_dedic_dl,TCHTH,NO_PAR}, {NULL,NO_PAR}  // frame 3
    };
    
    const T_FCT  BLOC_TCHD[] =      
    {
                                     {l1s_ctrl_tchtd,TCHD,NO_PAR},    {NULL,NO_PAR}, // frame 1
                                                                      {NULL,NO_PAR}, // frame 2
                                     {l1s_read_dummy,TCHD,NO_PAR},    {NULL,NO_PAR}  // frame 3
    };

    const T_FCT  BLOC_TCHA[] =      
    {
      {l1s_hopping_algo,TCHA,NO_PAR},{l1s_ctrl_tcha,TCHA,NO_PAR},     {NULL,NO_PAR}, // frame 1
                                                                      {NULL,NO_PAR}, // frame 2
                                     {l1s_read_dedic_dl,TCHA,NO_PAR}, {NULL,NO_PAR}  // frame 3
    };

    /*----------------------------------------------------*/
    /* TASK: Frequency Burst search in dedic/SDCCH...     */
    /*----------------------------------------------------*/
    const T_FCT  BLOC_FB51[] =      
    { 
      {l1s_ctrl_fb,FB51,NO_PAR},    {NULL,NO_PAR,NO_PAR}, // frame 1
                                    {NULL,NO_PAR,NO_PAR}, // frame 2
      {l1s_read_mon_result,FB51, 1},{NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_read_mon_result,FB51, 2},{NULL,NO_PAR,NO_PAR}, // frame 4
      {l1s_read_mon_result,FB51, 3},{NULL,NO_PAR,NO_PAR}, // frame 5
      {l1s_read_mon_result,FB51, 4},{NULL,NO_PAR,NO_PAR}, // frame 6
      {l1s_read_mon_result,FB51, 5},{NULL,NO_PAR,NO_PAR}, // frame 7
      {l1s_read_mon_result,FB51, 6},{NULL,NO_PAR,NO_PAR}, // frame 8
      {l1s_read_mon_result,FB51, 7},{NULL,NO_PAR,NO_PAR}, // frame 9
      {l1s_read_mon_result,FB51, 8},{NULL,NO_PAR,NO_PAR}, // frame 10
      {l1s_read_mon_result,FB51, 9},{NULL,NO_PAR,NO_PAR}, // frame 11
      {l1s_read_mon_result,FB51,10},{NULL,NO_PAR,NO_PAR}, // frame 12
      {l1s_read_mon_result,FB51,11},{NULL,NO_PAR,NO_PAR}, // frame 13
      {l1s_read_mon_result,FB51,12},{NULL,NO_PAR,NO_PAR}  // frame 14
    };

    /*----------------------------------------------------*/
    /* TASK: SB51, Synchro Burst reading. Dedic/SDCCH.    */
    /*----------------------------------------------------*/
    /*           C W W R     -> SB                        */  
    /*----------------------------------------------------*/
    const T_FCT BLOC_SB51[] = 
    {
      {l1s_ctrl_sbgen,SB51,NO_PAR},     {NULL,NO_PAR,NO_PAR}, // frame 1
                                        {NULL,NO_PAR,NO_PAR}, // frame 2
                                        {NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_read_mon_result,SB51,NO_PAR},{NULL,NO_PAR,NO_PAR}  // frame 4
    };

    /*----------------------------------------------------*/
    /* TASK: SBCNF51, Synchro confirmation. Dedic/SDCCH.  */
    /*----------------------------------------------------*/
    /*           C W W R     -> SBCONF                    */  
    /*----------------------------------------------------*/
    const T_FCT BLOC_SBCNF51[] = 
    {
      {l1s_ctrl_sbgen,SBCNF51,NO_PAR},     {NULL,NO_PAR,NO_PAR}, // frame 1
                                           {NULL,NO_PAR,NO_PAR}, // frame 2
                                           {NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_read_mon_result,SBCNF51,NO_PAR},{NULL,NO_PAR,NO_PAR}  // frame 4
    };

    /*----------------------------------------------------*/
    /* TASK: FB26, Frequency Burst search in dedic/TCH... */
    /*----------------------------------------------------*/
    /*           C W W R                                  */  
    /*----------------------------------------------------*/
    const T_FCT BLOC_FB26[] =      
    { 
      {l1s_ctrl_fb26,FB26,NO_PAR},      {NULL,NO_PAR,NO_PAR}, // frame 1
                                        {NULL,NO_PAR,NO_PAR}, // frame 2
                                        {NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_read_mon_result,FB26,NO_PAR},{NULL,NO_PAR,NO_PAR}  // frame 4
    };

    /*----------------------------------------------------*/
    /* TASK: SB26, Synchro. Burst reading in dedic/TCH... */
    /*----------------------------------------------------*/
    /*           C W W W R                                */  
    /*----------------------------------------------------*/
    const T_FCT BLOC_SB26[] =      
    { 
      {l1s_ctrl_sb26,SB26,NO_PAR},      {NULL,NO_PAR,NO_PAR}, // frame 1
                                        {NULL,NO_PAR,NO_PAR}, // frame 2
                                        {NULL,NO_PAR,NO_PAR}, // frame 3
                                        {NULL,NO_PAR,NO_PAR}, // frame 4
      {l1s_read_mon_result,SB26,NO_PAR},{NULL,NO_PAR,NO_PAR}  // frame 5
    };

    /*----------------------------------------------------*/
    /* TASK: SBCNF26, Synchro. Burst reading in dedic/TCH.*/
    /*----------------------------------------------------*/
    /*           C W W W R                                */  
    /*----------------------------------------------------*/
    const T_FCT BLOC_SBCNF26[] =      
    { 
      {l1s_ctrl_sb26,SBCNF26,NO_PAR},      {NULL,NO_PAR,NO_PAR}, // frame 1
                                           {NULL,NO_PAR,NO_PAR}, // frame 2
                                           {NULL,NO_PAR,NO_PAR}, // frame 3
                                           {NULL,NO_PAR,NO_PAR}, // frame 4
      {l1s_read_mon_result,SBCNF26,NO_PAR},{NULL,NO_PAR,NO_PAR}  // frame 5
    };

    /*----------------------------------------------------*/
    /* TASK: HWTEST after power-on...                     */
    /*----------------------------------------------------*/
    const T_FCT  BLOC_HWTEST[] =      
    { 
      {l1s_ctrl_hwtest,HWTEST,NO_PAR},     {NULL,NO_PAR,NO_PAR}, // frame 1
                                           {NULL,NO_PAR,NO_PAR}, // frame 2
                                           {NULL,NO_PAR,NO_PAR}, // frame 3
      {l1s_read_hwtest,HWTEST,NO_PAR},     {NULL,NO_PAR,NO_PAR}  // frame 4
    }; 

  #else                                                                         
    extern   T_FCT  BLOC_FB[];
    extern   T_FCT  BLOC_SB[];
    extern   T_FCT  BLOC_BCCHS[];
    extern   T_FCT  BLOC_BCCHN[];
    extern   T_FCT  BLOC_BCCHN_TOP[];
    extern   T_FCT  BLOC_EP[]; 
    extern   T_FCT  BLOC_SYNCHRO[];      
    extern   T_FCT  BLOC_ADC[];      
    extern   T_FCT  BLOC_SMSCB[];
    extern   T_FCT  BLOC_NP[];
    extern   T_FCT  BLOC_ALLC[];      
    extern   T_FCT  BLOC_DDL[];
    extern   T_FCT  BLOC_DUL[];
    extern   T_FCT  BLOC_ADL[];
    extern   T_FCT  BLOC_AUL[];
    extern   T_FCT  BLOC_DUL_ADL_MIXED[];
    extern   T_FCT  BLOC_ABORT[];
    extern   T_FCT  BLOC_RAACC[];
    extern   T_FCT  BLOC_TCHTF[];      
    extern   T_FCT  BLOC_TCHTH[];      
    extern   T_FCT  BLOC_TCHTD[];      
    extern   T_FCT  BLOC_TCHA[];      
    extern   T_FCT  BLOC_FB51[];      
    extern   T_FCT BLOC_SB51[]; 
    extern   T_FCT BLOC_SBCNF51[]; 
    extern   T_FCT BLOC_FB26[];      
    extern   T_FCT BLOC_SB26[];      
    extern   T_FCT BLOC_SBCNF26[]; 
    extern   T_FCT BLOC_HWTEST[]; 
#if ((REL99 == 1) && (FF_BHO == 1))
  extern T_FCT BLOC_FBSB[];
#endif  
    #if (L1_GPRS)
      extern T_FCT  BLOC_BCCHN_TRAN[];
    #endif

  #endif
#endif //ndef L1_MFTAB_H




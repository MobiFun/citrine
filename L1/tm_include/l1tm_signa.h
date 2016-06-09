/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1TM_SIGNA.H
 *
 *        Filename l1tm_signa.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

  #define P_TMODE ( P_L1C + 3 ) // keep compatibility with GPRS code


  //TestMode
  #define TMODE_RXLEV_REQ                  ( ( P_TMODE << 8 ) | 1 )
  #define TMODE_RXLEV_IND                  ( ( P_TMODE << 8 ) | 2 )
  #define TMODE_STOP_RXLEV_CON             ( ( P_TMODE << 8 ) | 3 )
  #define TMODE_FB0_REQ                    ( ( P_TMODE << 8 ) | 4 )
  #define TMODE_FB1_REQ                    ( ( P_TMODE << 8 ) | 5 )
  #define TMODE_SB_REQ                     ( ( P_TMODE << 8 ) | 6 )
  #define TMODE_FB_CON                     ( ( P_TMODE << 8 ) | 7 )
  #define TMODE_SB_CON                     ( ( P_TMODE << 8 ) | 8 )
  #define TMODE_BCCHS_CON                  ( ( P_TMODE << 8 ) | 9 )
  #define TMODE_RA_START                   ( ( P_TMODE << 8 ) | 10 )
  #define TMODE_RA_DONE                    ( ( P_TMODE << 8 ) | 11 )
  #define TMODE_SCELL_NBCCH_REQ            ( ( P_TMODE << 8 ) | 12 )
  #define TMODE_STOP_SCELL_BCCH_REQ        ( ( P_TMODE << 8 ) | 13 )
  #define TMODE_NEW_SCELL_REQ              ( ( P_TMODE << 8 ) | 14 )
  #define TMODE_TCH_REQ                    ( ( P_TMODE << 8 ) | 15 )
  #define TMODE_IMMED_ASSIGN_REQ           ( ( P_TMODE << 8 ) | 16 )
  #define TMODE_IMMED_ASSIGN_CON           ( ( P_TMODE << 8 ) | 17 )
  #define TMODE_STOP_RX_TX                 ( ( P_TMODE << 8 ) | 18 )
  #define TMODE_SACCH_INFO                 ( ( P_TMODE << 8 ) | 19 )
  #define TESTMODE_PRIM                    ( ( P_TMODE << 8 ) | 20 )
  #define TMODE_FB_SB_REQ                  ( ( P_TMODE << 8 ) | 21 )
  #define TMODE_NETWORK_SYNC_IND           ( ( P_TMODE << 8 ) | 22 )
  #define TMODE_TCH_INFO                   ( ( P_TMODE << 8 ) | 23 )
  #if L1_GPRS
    #define TMODE_PDTCH_ASSIGN_REQ           ( ( P_TMODE << 8 ) | 24 )
    #define TMODE_PDTCH_INFO                 ( ( P_TMODE << 8 ) | 25 )
  #endif

//  #define TMODE_RXLEV_REQ                  ( ( P_TMODE << 8 ) | 24 )
//  #define TMODE_RXLEV_IND                  ( ( P_TMODE << 8 ) | 25 )

/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_SIGNA.H
 *
 *        Filename l1_signa.h
 *  Copyright 2003 (C) Texas Instruments 
 *
 ************* Revision Controle System Header *************/

#define P_L1C  0
#define P_DLL  ( P_L1C + 1 )

#if ((REL99 == 1) && (FF_BHO == 1))
  #define P_MSB       P_L1C 
  #define P_SHIFT1    8
  #define P_SHIFT2    0
  #define P_MPHC_UL   0
  #define P_MPHC_DL   0
#endif

// Messages Test/PWRMNGT <-> L1A

/* Message used for software dynamic configuration */
#define TST_SW_CONFIG_REQ                ( ( P_L1C << 8 ) | 117)

#if (OP_L1_STANDALONE == 1)
  /* Message used for hardware dynamic configuration */
  #define TST_HW_CONFIG_REQ              ( ( P_L1C << 8 ) | 116)
#endif

#define TST_TEST_HW_REQ                  ( ( P_L1C << 8 ) | 1  ) // build: trigger
#define TST_TEST_HW_CON                  ( ( P_L1C << 8 ) | 2  )
#define TST_TIMESTAMP_MSG                ( ( P_L1C << 8 ) | 3  ) // build: trigger
#define TST_SLEEP_REQ                    ( ( P_L1C << 8 ) | 4  ) // build: T_TST_SLEEP_REQ


// Messages L3 <-> L1A
#define MPHC_RXLEV_REQ                   ( ( P_L1C << 8 ) | 11 ) // build: T_FULL_LIST_MEAS
#define MPHC_RXLEV_IND                   ( ( P_L1C << 8 ) | 12 )

#define MPHC_STOP_RXLEV_REQ              ( ( P_L1C << 8 ) | 13 ) // build: trigger
#define MPHC_STOP_RXLEV_CON              ( ( P_L1C << 8 ) | 14 )

#define MPHC_NETWORK_SYNC_REQ            ( ( P_L1C << 8 ) | 15 ) // build: T_MPHC_NETWORK_SYNC_REQ
#define MPHC_NETWORK_SYNC_IND            ( ( P_L1C << 8 ) | 16 )

#define MPHC_STOP_NETWORK_SYNC_REQ       ( ( P_L1C << 8 ) | 17 ) // build: trigger
#define MPHC_STOP_NETWORK_SYNC_CON       ( ( P_L1C << 8 ) | 18 )

#define MPHC_NEW_SCELL_REQ               ( ( P_L1C << 8 ) | 19 ) // build: T_MPHC_NEW_SCELL_REQ
#define MPHC_NEW_SCELL_CON               ( ( P_L1C << 8 ) | 20 )

#define MPHC_START_CCCH_REQ              ( ( P_L1C << 8 ) | 21 ) // build: T_MPHC_START_CCCH_REQ
#define MPHC_STOP_CCCH_REQ               ( ( P_L1C << 8 ) | 22 ) // build: trigger
#define MPHC_STOP_CCCH_CON               ( ( P_L1C << 8 ) | 23 )

#define MPHC_SCELL_NBCCH_REQ             ( ( P_L1C << 8 ) | 24 ) // build: T_MPHC_SCELL_NBCCH_REQ
#define MPHC_SCELL_EBCCH_REQ             ( ( P_L1C << 8 ) | 25 ) // build: T_MPHC_SCELL_EBCCH_REQ
#define MPHC_STOP_SCELL_BCCH_REQ         ( ( P_L1C << 8 ) | 26 ) // build: trigger
#define MPHC_STOP_SCELL_BCCH_CON         ( ( P_L1C << 8 ) | 27 )

#define MPHC_NCELL_BCCH_REQ              ( ( P_L1C << 8 ) | 28 ) // build: T_MPHC_NCELL_BCCH_REQ
#define MPHC_NCELL_BCCH_IND              ( ( P_L1C << 8 ) | 29 )
#define MPHC_STOP_NCELL_BCCH_REQ         ( ( P_L1C << 8 ) | 30 ) // build: T_MPHC_STOP_NCELL_BCCH_REQ
#define MPHC_STOP_NCELL_BCCH_CON         ( ( P_L1C << 8 ) | 31 )

#define MPHC_NCELL_SYNC_REQ              ( ( P_L1C << 8 ) | 32 ) // build: T_MPHC_NCELL_SYNC_REQ
#define MPHC_NCELL_SYNC_IND              ( ( P_L1C << 8 ) | 33 )
#define MPHC_STOP_NCELL_SYNC_REQ         ( ( P_L1C << 8 ) | 34 ) // build: T_MPHC_STOP_NCELL_SYNC_REQ
#define MPHC_STOP_NCELL_SYNC_CON         ( ( P_L1C << 8 ) | 35 )

#define MPHC_RXLEV_PERIODIC_REQ          ( ( P_L1C << 8 ) | 36 ) // build: T_MPHC_RXLEV_PERIODIC_REQ
#define MPHC_RXLEV_PERIODIC_IND          ( ( P_L1C << 8 ) | 37 )
#define MPHC_STOP_RXLEV_PERIODIC_REQ     ( ( P_L1C << 8 ) | 38 ) // build: trigger
#define MPHC_STOP_RXLEV_PERIODIC_CON     ( ( P_L1C << 8 ) | 39 )

#define MPHC_CONFIG_CBCH_REQ             ( ( P_L1C << 8 ) | 40 ) // build: T_MPHC_CONFIG_CBCH_REQ
#define MPHC_CBCH_SCHEDULE_REQ           ( ( P_L1C << 8 ) | 41 ) // build: T_MPHC_CBCH_SCHEDULE_REQ
#define MPHC_CBCH_UPDATE_REQ             ( ( P_L1C << 8 ) | 42 ) // build: T_MPHC_CBCH_UPDATE_REQ
#define MPHC_CBCH_INFO_REQ               ( ( P_L1C << 8 ) | 43 ) // build: T_MPHC_CBCH_INFO_REQ
#define MPHC_STOP_CBCH_REQ               ( ( P_L1C << 8 ) | 44 ) // build: T_MPHC_STOP_CBCH_REQ
#define MPHC_STOP_CBCH_CON               ( ( P_L1C << 8 ) | 45 )

#define MPHC_RA_REQ                      ( ( P_L1C << 8 ) | 46 ) // build: T_MPHC_RA_REQ
#define MPHC_RA_CON                      ( ( P_L1C << 8 ) | 47 )

#define MPHC_STOP_RA_REQ                 ( ( P_L1C << 8 ) | 48 ) // build: trigger
#define MPHC_STOP_RA_CON                 ( ( P_L1C << 8 ) | 49 )

#define MPHC_DATA_IND                    ( ( P_L1C << 8 ) | 50 )

#define MPHC_IMMED_ASSIGN_REQ            ( ( P_L1C << 8 ) | 51 )  // build: T_MPHC_IMMED_ASSIGN_REQ
#define MPHC_CHANNEL_ASSIGN_REQ          ( ( P_L1C << 8 ) | 52 )  // build: T_MPHC_CHANNEL_ASSIGN_REQ
#define MPHC_ASYNC_HO_REQ                ( ( P_L1C << 8 ) | 53 )  // build: T_MPHC_ASYNC_HO_REQ
#define MPHC_SYNC_HO_REQ                 ( ( P_L1C << 8 ) | 54 )  // build: T_MPHC_SYNC_HO_REQ
#define MPHC_PRE_SYNC_HO_REQ             ( ( P_L1C << 8 ) | 55 )  // build: T_MPHC_PRE_SYNC_HO_REQ
#define MPHC_PSEUDO_SYNC_HO_REQ          ( ( P_L1C << 8 ) | 56 )  // build: T_MPHC_PSEUDO_SYNC_HO_REQ
#define MPHC_STOP_DEDICATED_REQ          ( ( P_L1C << 8 ) | 57 )  // build: trigger
#define MPHC_STOP_DEDICATED_CON          ( ( P_L1C << 8 ) | 128 ) // build: trigger

#define MPHC_CHANGE_FREQUENCY_CON        ( ( P_L1C << 8 ) | 58 )
#define MPHC_ASYNC_HO_CON                ( ( P_L1C << 8 ) | 59 )
#define MPHC_CHANNEL_ASSIGN_CON          ( ( P_L1C << 8 ) | 60 )
#define MPHC_CHANNEL_MODE_MODIFY_CON     ( ( P_L1C << 8 ) | 61 )
#define MPHC_HANDOVER_FAIL_CON           ( ( P_L1C << 8 ) | 62 )
#define MPHC_IMMED_ASSIGN_CON            ( ( P_L1C << 8 ) | 63 )
#define MPHC_PRE_SYNC_HO_CON             ( ( P_L1C << 8 ) | 64 )
#define MPHC_SET_CIPHERING_CON           ( ( P_L1C << 8 ) | 65 )
#define MPHC_SYNC_HO_CON                 ( ( P_L1C << 8 ) | 66 )
#define MPHC_TA_FAIL_IND                 ( ( P_L1C << 8 ) | 67 )

#define MPHC_HANDOVER_FINISHED           ( ( P_L1C << 8 ) | 68 )

#define MPHC_CHANGE_FREQUENCY            ( ( P_L1C << 8 ) | 69 ) // build: T_MPHC_CHANGE_FREQUENCY
#define MPHC_CHANNEL_MODE_MODIFY_REQ     ( ( P_L1C << 8 ) | 70 ) // build: T_MPHC_CHANNEL_MODE_MODIFY_REQ
#define MPHC_HANDOVER_FAIL_REQ           ( ( P_L1C << 8 ) | 71 ) // build: trigger
#define MPHC_SET_CIPHERING_REQ           ( ( P_L1C << 8 ) | 72 ) // build: T_MPHC_SET_CIPHERING_REQ

#define MPHC_MEAS_REPORT                 ( ( P_L1C << 8 ) | 73 ) // build: T_MPHC_MEAS_REPORT
#define MPHC_UPDATE_BA_LIST              ( ( P_L1C << 8 ) | 74 ) // build: T_NEW_BA_LIST

#define MPHC_NCELL_FB_SB_READ            ( ( P_L1C << 8 ) | 75 ) // build: T_MPHC_NCELL_FB_SB_READ
#define MPHC_NCELL_SB_READ               ( ( P_L1C << 8 ) | 76 ) // build: T_MPHC_NCELL_SB_READ
#define MPHC_NCELL_BCCH_READ             ( ( P_L1C << 8 ) | 77 ) // build: T_MPHC_NCELL_BCCH_READ

// Messages L1S -> L1A
#define L1C_VALID_MEAS_INFO              ( ( P_L1C << 8 ) | 80 )
#define L1C_FB_INFO                      ( ( P_L1C << 8 ) | 81 )
#define L1C_SB_INFO                      ( ( P_L1C << 8 ) | 82 )
#if ((REL99 == 1) && (FF_BHO == 1))
#define L1C_FBSB_INFO                    ( ( ( ( P_MSB << P_SHIFT1 ) | 118 ) << P_SHIFT2 ) | P_MPHC_DL )
#endif
#define L1C_SBCONF_INFO                  ( ( P_L1C << 8 ) | 83 )
#define L1C_BCCHS_INFO                   ( ( P_L1C << 8 ) | 84 )
#define L1C_BCCHN_INFO                   ( ( P_L1C << 8 ) | 85 )
#define L1C_NP_INFO                      ( ( P_L1C << 8 ) | 86 )
#define L1C_EP_INFO                      ( ( P_L1C << 8 ) | 87 )
#define L1C_ALLC_INFO                    ( ( P_L1C << 8 ) | 88 )
#define L1C_RXLEV_PERIODIC_DONE          ( ( P_L1C << 8 ) | 89 )
#define L1C_CB_INFO                      ( ( P_L1C << 8 ) | 90 )
#define L1C_RA_DONE                      ( ( P_L1C << 8 ) | 91 )
#define L1C_SACCH_INFO                   ( ( P_L1C << 8 ) | 92 )
#define L1C_MEAS_DONE                    ( ( P_L1C << 8 ) | 93 )
#define L1C_DEDIC_DONE                   ( ( P_L1C << 8 ) | 94 )
#define L1C_HANDOVER_FINISHED            ( ( P_L1C << 8 ) | 95 )
#define L1C_REDEF_DONE                   ( ( P_L1C << 8 ) | 96 )
#define L1C_STOP_DEDICATED_DONE          ( ( P_L1C << 8 ) | 129 )

// Messages O&M <-> L1A
#define OML1_CLOSE_TCH_LOOP_REQ          ( ( P_L1C << 8 ) | 97 ) // build: T_OML1_CLOSE_TCH_LOOP_REQ
#define OML1_OPEN_TCH_LOOP_REQ           ( ( P_L1C << 8 ) | 98 ) // build: trigger
#define OML1_START_DAI_TEST_REQ          ( ( P_L1C << 8 ) | 99 ) // build: T_OML1_START_DAI_TEST_REQ
#define OML1_STOP_DAI_TEST_REQ           ( ( P_L1C << 8 ) | 100 ) // build: trigger

#define OML1_CLOSE_TCH_LOOP_CON          ( ( P_L1C << 8 ) | 101 )
#define OML1_OPEN_TCH_LOOP_CON           ( ( P_L1C << 8 ) | 102 )
#define OML1_START_DAI_TEST_CON          ( ( P_L1C << 8 ) | 103 )
#define OML1_STOP_DAI_TEST_CON           ( ( P_L1C << 8 ) | 104 )


// Message Custom I/F <- L1S
#define CST_ADC_RESULT                   ( ( P_L1C << 8 ) | 105 )


// Messages for trace
#define L1_STATS_REQ                     ( ( P_L1C << 8 ) | 107 ) // build: T_L1_STATS_REQ
#define L1_DUMMY_FOR_SIM                 ( ( P_L1C << 8 ) | 108 ) // build: trigger

#define TRACE_DSP_DEBUG                  ( ( P_L1C << 8 ) | 106 )
#define TRACE_CONFIG                     ( ( P_L1C << 8 ) | 123 )
#define TRACE_CONDENSED_PDTCH            ( ( P_L1C << 8 ) | 124 )
#define TRACE_INFO                       ( ( P_L1C << 8 ) | 125 )
#define QUICK_TRACE                      ( ( P_L1C << 8 ) | 126 )
#define TRACE_DSP_AMR_DEBUG              ( ( P_L1C << 8 ) | 127 )

#define MPHC_NETWORK_LOST_IND            ( ( P_L1C << 8 ) | 110 ) // build: trigger

// Messages MMI <-> L1A
#define MMI_ADC_REQ                      ( ( P_L1C << 8 ) | 111 ) // build: T_MMI_ADC_REQ
#define MMI_STOP_ADC_REQ                 ( ( P_L1C << 8 ) | 112 ) // build: trigger
#define MMI_STOP_ADC_CON                 ( ( P_L1C << 8 ) | 113 )

#define L1_TEST_HW_INFO                  ( ( P_L1C << 8 ) | 119 )

// Multi-band selection E-GSM900/DCS1800/PCS1900/GSM850
  #define MPHC_INIT_L1_REQ               ( ( P_L1C << 8 ) | 114 ) // build: T_MPHC_INIT_L1_REQ
  #define MPHC_INIT_L1_CON               ( ( P_L1C << 8 ) | 115 )

// Message RR -> L1A for Enhanced meas.
#if (L1_12NEIGH ==1)
#define  MPHC_NCELL_LIST_SYNC_REQ        ( ( P_L1C << 8 ) | 122 ) // build: T_MPHC_NCELL_LIST_SYNC_REQ   
#endif

// Messages L2 <-> L1A
#define PH_DATA_IND                      ( ( P_DLL << 8 ) | 109 )


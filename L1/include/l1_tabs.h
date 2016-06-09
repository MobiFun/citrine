/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_TABS.H
 *
 *        Filename l1_tabs.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/
/***********************************************************
 * Content:
 *   This file contains the miscelaneous ROM tables.
 ***********************************************************/

#ifndef L1_TABS_H
#define L1_TABS_H

  #ifdef L1_ASYNC_C
    /*-----------------------------------------------------------------*/
    /* Idle Tasks info. (Paging position, extended Paging position...) */
    /*-----------------------------------------------------------------*/
    /* REM:                                                            */
    /* The "working area" field gives the starting position of an area */
    /* it will be used for neighbour:  - FB search,                    */
    /*                                 - SB reading,                   */
    /* The value given for each parameter set takes into account the   */
    /* size of the "FB search" task and the CBCH task.                 */                                 
    /*-----------------------------------------------------------------*/
    //  NP or EP task size: 1 + 4 + 1 = 6.
    //  BCCHS task size: 1 + 4 + 1 = 6.
    //  FB task size: 1 + 12 + 1 = 14.    --+-- FB + SB task take 15 TDMA (pipeline overlay).
    //  SB task size: 1 + 2 + 1 = 4.      --+
    //  CNF, SB task size: 1 + 2 + 1 = 4.
    //  BC (Broad. Channel): 1 + 4 + 1 = 6     
     
    const T_IDLE_TASK_INFO IDLE_INFO_NCOMB[(MAX_AG_BLKS_RES_NCOMB+1) * (MAX_PG_BLOC_INDEX_NCOMB+1)] =
    // BS_CCCH_SDCCH_COMB = False, BCCH not combined.
    { 
      // BS_AG_BLKS_RES = 0.
      // -------------------
      // Paging, Ext Paging
      {  CCCH_0,     CCCH_2  },  // Paging Block Index = 0.
      {  CCCH_1,     CCCH_3  },  // Paging Block Index = 1.
      {  CCCH_2,     CCCH_4  },  // Paging Block Index = 2.
      {  CCCH_3,     CCCH_5  },  // Paging Block Index = 3.
      {  CCCH_4,     CCCH_6  },  // Paging Block Index = 4.
      {  CCCH_5,     CCCH_7  },  // Paging Block Index = 5.
      {  CCCH_6,     CCCH_8  },  // Paging Block Index = 6.
      {  CCCH_7,     CCCH_0  },  // Paging Block Index = 7.
      {  CCCH_8,     CCCH_1  },  // Paging Block Index = 8.

      // BS_AG_BLKS_RES = 1.
      // -------------------
      // Paging, Ext Paging
      {  CCCH_1,     CCCH_3  },  // Paging Block Index = 0.
      {  CCCH_2,     CCCH_4  },  // Paging Block Index = 1.
      {  CCCH_3,     CCCH_5  },  // Paging Block Index = 2.
      {  CCCH_4,     CCCH_6  },  // Paging Block Index = 3.
      {  CCCH_5,     CCCH_7  },  // Paging Block Index = 4.
      {  CCCH_6,     CCCH_8  },  // Paging Block Index = 5.
      {  CCCH_7,     CCCH_1  },  // Paging Block Index = 6.
      {  CCCH_8,     CCCH_2  },  // Paging Block Index = 7.
      {  NULL,       NULL    },  // Paging Block Index = 8.

      // BS_AG_BLKS_RES = 2.
      // -------------------
      // Paging, Ext Paging
      {  CCCH_2,     CCCH_4  },  // Paging Block Index = 0.
      {  CCCH_3,     CCCH_5  },  // Paging Block Index = 1.
      {  CCCH_4,     CCCH_6  },  // Paging Block Index = 2.
      {  CCCH_5,     CCCH_7  },  // Paging Block Index = 3.
      {  CCCH_6,     CCCH_8  },  // Paging Block Index = 4.
      {  CCCH_7,     CCCH_2  },  // Paging Block Index = 5.
      {  CCCH_8,     CCCH_3  },  // Paging Block Index = 6.
      {  NULL,       NULL    },  // Paging Block Index = 7.
      {  NULL,       NULL    },  // Paging Block Index = 8.

      // BS_AG_BLKS_RES = 3.
      // -------------------
      // Paging, Ext Paging, 
      {  CCCH_3,     CCCH_5  },  // Paging Block Index = 0.
      {  CCCH_4,     CCCH_6  },  // Paging Block Index = 1.
      {  CCCH_5,     CCCH_7  },  // Paging Block Index = 2.
      {  CCCH_6,     CCCH_8  },  // Paging Block Index = 3.
      {  CCCH_7,     CCCH_3  },  // Paging Block Index = 4.
      {  CCCH_8,     CCCH_4  },  // Paging Block Index = 5.
      {  NULL,       NULL    },  // Paging Block Index = 6.
      {  NULL,       NULL    },  // Paging Block Index = 7.
      {  NULL,       NULL    },  // Paging Block Index = 8.

      // BS_AG_BLKS_RES = 4.
      // -------------------
      // Paging, Ext Paging
      {  CCCH_4,     CCCH_6  },  // Paging Block Index = 0.
      {  CCCH_5,     CCCH_7  },  // Paging Block Index = 1.
      {  CCCH_6,     CCCH_8  },  // Paging Block Index = 2.
      {  CCCH_7,     CCCH_4  },  // Paging Block Index = 3.
      {  CCCH_8,     CCCH_5  },  // Paging Block Index = 4.
      {  NULL,       NULL    },  // Paging Block Index = 5.
      {  NULL,       NULL    },  // Paging Block Index = 6.
      {  NULL,       NULL    },  // Paging Block Index = 7.
      {  NULL,       NULL    },  // Paging Block Index = 8.

      // BS_AG_BLKS_RES = 5.
      // -------------------
      // Paging, Ext Paging
      {  CCCH_5,     CCCH_7  },  // Paging Block Index = 0.
      {  CCCH_6,     CCCH_8  },  // Paging Block Index = 1.
      {  CCCH_7,     CCCH_5  },  // Paging Block Index = 2.
      {  CCCH_8,     CCCH_6  },  // Paging Block Index = 3.
      {  NULL,       NULL    },  // Paging Block Index = 4.
      {  NULL,       NULL    },  // Paging Block Index = 5.
      {  NULL,       NULL    },  // Paging Block Index = 6.
      {  NULL,       NULL    },  // Paging Block Index = 7.
      {  NULL,       NULL    },  // Paging Block Index = 8.

      // BS_AG_BLKS_RES = 6.
      // -------------------
      // Paging, Ext Paging, 
      {  CCCH_6,     CCCH_8  },  // Paging Block Index = 0.
      {  CCCH_7,     CCCH_6  },  // Paging Block Index = 1.
      {  CCCH_8,     CCCH_7  },  // Paging Block Index = 2.
      {  NULL,       NULL    },  // Paging Block Index = 3.
      {  NULL,       NULL    },  // Paging Block Index = 4.
      {  NULL,       NULL    },  // Paging Block Index = 5.
      {  NULL,       NULL    },  // Paging Block Index = 6.
      {  NULL,       NULL    },  // Paging Block Index = 7.
      {  NULL,       NULL    },  // Paging Block Index = 8.

      // BS_AG_BLKS_RES = 7.
      // -------------------
      // Paging, Ext Paging
      {  CCCH_7,     CCCH_7  },  // Paging Block Index = 0.
      {  CCCH_8,     CCCH_8  },  // Paging Block Index = 1.
      {  NULL,       NULL    },  // Paging Block Index = 2.
      {  NULL,       NULL    },  // Paging Block Index = 3.
      {  NULL,       NULL    },  // Paging Block Index = 4.
      {  NULL,       NULL    },  // Paging Block Index = 5.
      {  NULL,       NULL    },  // Paging Block Index = 6.
      {  NULL,       NULL    },  // Paging Block Index = 7.
      {  NULL,       NULL    }   // Paging Block Index = 8.
    };

    const T_IDLE_TASK_INFO IDLE_INFO_COMB[(MAX_AG_BLKS_RES_COMB+1) * (MAX_PG_BLOC_INDEX_COMB+1)] =
    // BS_CCCH_SDCCH_COMB = TRUE, BCCH combined.
    { 
      // BS_AG_BLKS_RES = 0.
      // -------------------
      // Paging, Ext Paging, offset, working_area
      {  CCCH_0,     CCCH_2  },  // Paging Block Index = 0.
      {  CCCH_1,     CCCH_0  },  // Paging Block Index = 1.
      {  CCCH_2,     CCCH_1  },  // Paging Block Index = 2.

      // BS_AG_BLKS_RES = 1.
      // -------------------
      // Paging, Ext Paging, offset, working_area
      {  CCCH_1,     CCCH_1  },  // Paging Block Index = 0.
      {  CCCH_2,     CCCH_2  },  // Paging Block Index = 1.
      {  NULL,       NULL    },  // Paging Block Index = 2.

      // BS_AG_BLKS_RES = 2.
      // -------------------
      // Paging, Ext Paging, offset, working_area
      {  CCCH_2,     CCCH_2  },  // Paging Block Index = 0.
      {  NULL,       NULL    },  // Paging Block Index = 1.
      {  NULL,       NULL    }   // Paging Block Index = 2.
    };

      
    /*-------------------------------------*/
    /* Table giving the number of Paging   */
    /* blocks in a MF51.                   */
    /* (called "N div BS_PA_MFRMS" in      */
    /* GSM05.02, Page 21).                 */
    /*-------------------------------------*/

    // BS_CCCH_SDCCH_COMB = False, BCCH not combined.
    const UWORD8 NBPCH_IN_MF51_NCOMB[(MAX_AG_BLKS_RES_NCOMB+1)] =
    {
      9,    // BS_AG_BLKS_RES = 0. 
      8,    // BS_AG_BLKS_RES = 1. 
      7,    // BS_AG_BLKS_RES = 2. 
      6,    // BS_AG_BLKS_RES = 3. 
      5,    // BS_AG_BLKS_RES = 4. 
      4,    // BS_AG_BLKS_RES = 5. 
      3,    // BS_AG_BLKS_RES = 6. 
      2     // BS_AG_BLKS_RES = 7. 
    };    

    // BS_CCCH_SDCCH_COMB = True, BCCH combined.
    const UWORD8 NBPCH_IN_MF51_COMB[(MAX_AG_BLKS_RES_COMB+1)] =
    {
      3,    // BS_AG_BLKS_RES = 0. 
      2,    // BS_AG_BLKS_RES = 1. 
      1     // BS_AG_BLKS_RES = 2. 
    };    

    // Initial value for Downlink Signalling failure Counter (DSC).
    const UWORD8 DSC_INIT_VALUE[MAX_BS_PA_MFRMS-1] =
    {
      45,   // BS_PA_MFRMS = 2.
      30,   // BS_PA_MFRMS = 3.
      23,   // BS_PA_MFRMS = 4.
      18,   // BS_PA_MFRMS = 5.
      15,   // BS_PA_MFRMS = 6.
      13,   // BS_PA_MFRMS = 7.
      11,   // BS_PA_MFRMS = 8.
      10    // BS_PA_MFRMS = 9.
    };

    // REM: 2nd block of SDCCH is always at the same position as the first block
    //      but 1 mf51 later.
    // REM: monitoring during SDCCH used a fixe area (FB51/SB51/SBCNF51 tasks).
    //      Here is given the area starting position. This position is chosen
    //      to allow the equations for SBCNF51 occurence as it is in the l1s
    //      scheduler (the area do not overlap the end of 102 multiframe 
    //      structure).
    // Table for SDCCH description, Down Link & Up link, Not combined case.
     const T_SDCCH_DESC SDCCH_DESC_NCOMB[8] =
    {
      //  "dl_D" , "dl_A"  , "ul_D"  , "ul_A".        , "monit. area"
      {  51 - 12 , 32 - 12 , 15 - 12 ,  47 - 12       ,  70 - 12     }, // SDCCH, D0
      {  55 - 12 , 36 - 12 , 19 - 12 ,  51 - 12       ,  74 - 12     }, // SDCCH, D1
      {  59 - 12 , 40 - 12 , 23 - 12 ,  55 - 12       ,  78 - 12     }, // SDCCH, D2
      {  12 - 12 , 44 - 12 , 27 - 12 ,  59 - 12       ,  82 - 12     }, // SDCCH, D3
      {  16 - 12 , 83 - 12 , 31 - 12 ,  98 - 12       ,  35 - 12     }, // SDCCH, D4
      {  20 - 12 , 87 - 12 , 35 - 12 , 102 - 12       ,  39 - 12     }, // SDCCH, D5
      {  24 - 12 , 91 - 12 , 39 - 12 ,   4 - 12 + 102 ,  43 - 12     }, // SDCCH, D6
      {  28 - 12 , 95 - 12 , 43 - 12 ,   8 - 12 + 102 ,  47 - 12     }  // SDCCH, D7
    };  

    // REM: monitoring during SDCCH used a fixe area (FB51/SB51/SBCNF51 tasks).
    //      Here is given the area starting position. This position is chosen
    //      to allow the equations for SBCNF51 occurence as it is in the l1s
    //      scheduler (the area do not overlap the end of 102 multiframe 
    //      structure).
    // Table for SDCCH description, Down Link & Up link, Combined case.
    const T_SDCCH_DESC SDCCH_DESC_COMB[4] =
    {
      // "dl_D"  , "dl_A"  , "ul_D"  , "ul_A".       , "monit. area"
      {  73 - 37 , 42 - 37 , 37 - 37 , 57 - 37       ,  92 - 37     }, // SDCCH, D0
      {  77 - 37 , 46 - 37 , 41 - 37 , 61 - 37       ,  96 - 37     }, // SDCCH, D1
      {  83 - 37 , 93 - 37 , 47 - 37 , 6  - 37 + 102 ,  51 - 37     }, // SDCCH, D2
      {  87 - 37 , 97 - 37 , 51 - 37 , 10 - 37 + 102 ,  55 - 37     }  // SDCCH, D3
    };                                                               

    // Table for HOPPING SEQUENCE GENERATION ALGORITHM.
    const UWORD8 RNTABLE[114] =
    {
       48,  98,  63,   1,  36,  95,  78, 102,  94,  73,
        0,  64,  25,  81,  76,  59, 124,  23, 104, 100,
      101,  47, 118,  85,  18,  56,  96,  86,  54,   2,
       80,  34, 127,  13,   6,  89,  57, 103,  12,  74,
       55, 111,  75,  38, 109,  71, 112,  29,  11,  88,
       87,  19,   3,  68, 110,  26,  33,  31,   8,  45,
       82,  58,  40, 107,  32,   5, 106,  92,  62,  67,
       77, 108, 122,  37,  60,  66, 121,  42,  51, 126,
      117, 114,   4,  90,  43,  52,  53, 113, 120,  72,
       16,  49,   7,  79, 119,  61,  22,  84,   9,  97,
       91,  15,  21,  24,  46,  39,  93, 105,  65,  70,
      125,  99,  17, 123
    };


    // Table giving the RACH slot positions when COMBINED.
    // Rem: all is shifted left by 1 to map the position of the possible "contoles".
    const UWORD8 COMBINED_RA_DISTRIB[51] =
    { 
      0, 0, 0, 
      1, 1, 
      0, 0, 0, 0, 0, 0, 0, 0, 
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 
      0, 0, 0, 0, 0
    };

  #if !L1_GPRS
    const T_TASK_MFTAB  TASK_ROM_MFTAB[NBR_DL_L1S_TASKS] =
    {
      { BLOC_HWTEST,   BLOC_HWTEST_SIZE   },  // HWTEST 
      { BLOC_ADC    ,  BLOC_ADC_SIZE      },  // ADC in CS_MODE0
      { NULL,          0                  },  // DEDIC   (not meaningfull)
      { BLOC_RAACC,    BLOC_RAACC_SIZE    },  // RAACC                 
      { NULL,          0                  },  // RAHO    (not meaningfull)             
      { NULL,          0                  },  // NSYNC   (not meaningfull)             
      { BLOC_FBNEW,    BLOC_FBNEW_SIZE    },  // FBNEW 
      { BLOC_SBCONF,   BLOC_SBCONF_SIZE   },  // SBCONF 
      { BLOC_SB2,      BLOC_SB2_SIZE      },  // SB2 
      { BLOC_FB26,     BLOC_FB26_SIZE     },  // FB26               
      { BLOC_SB26,     BLOC_SB26_SIZE     },  // SB26
      { BLOC_SBCNF26,  BLOC_SBCNF26_SIZE  },  // SBCNF26
      { BLOC_FB51,     BLOC_FB51_SIZE     },  // FB51               
      { BLOC_SB51,     BLOC_SB51_SIZE     },  // SB51               
      { BLOC_SBCNF51,  BLOC_SBCNF51_SIZE  },  // SBCNF51               
      { BLOC_BCCHN,    BLOC_BCCHN_SIZE    },  // BCCHN   
      { BLOC_ALLC,     S_RECT4_SIZE       },  // ALLC
      { BLOC_EBCCHS,   S_RECT4_SIZE       },  // EBCCHS 
      { BLOC_NBCCHS,   S_RECT4_SIZE       },  // NBCCHS 
      { BLOC_SMSCB,    BLOC_SMSCB_SIZE    },  // SMSCB
      { BLOC_NP,       S_RECT4_SIZE       },  // NP  
      { BLOC_EP,       S_RECT4_SIZE       },  // EP  
      { BLOC_ADL,      S_RECT4_SIZE       },  // ADL
      { BLOC_AUL,      S_RECT4_SIZE       },  // AUL                 
      { BLOC_DDL,      S_RECT4_SIZE       },  // DDL
      { BLOC_DUL,      S_RECT4_SIZE       },  // DUL     
      { BLOC_TCHD,     BLOC_TCHT_SIZE     },  // TCHD
      { BLOC_TCHA,     BLOC_TCHA_SIZE     },  // TCHA
      { BLOC_TCHTF,    BLOC_TCHT_SIZE     },  // TCHTF
      { BLOC_TCHTH,    BLOC_TCHT_SIZE     },  // TCHTH
      { BLOC_BCCHN_TOP,BLOC_BCCHN_TOP_SIZE},  // BCCHN_TOP  
#if ((REL99 == 1) && (FF_BHO == 1))
    { BLOC_FBSB,     BLOC_FBSB_SIZE     },  // FBSB   
#endif
      { BLOC_SYNCHRO,  BLOC_SYNCHRO_SIZE  }   // SYNCHRO
    };

    const UWORD8 DSP_TASK_CODE[NBR_DL_L1S_TASKS] =
    {
      CHECKSUM_DSP_TASK,// HWTEST 
      0,                // DEDIC (not meaningfull)
      0,                // ADC   (not meaningfull)
      RACH_DSP_TASK,    // RAACC     
      RACH_DSP_TASK,    // RAHO
      0,                // NSYNC (not meaningfull)
      FB_DSP_TASK,      // FBNEW    
      SB_DSP_TASK,      // SBCONF
      SB_DSP_TASK,      // SB2     
      TCH_FB_DSP_TASK,  // FB26               
      TCH_SB_DSP_TASK,  // SB26               
      TCH_SB_DSP_TASK,  // SBCNF26               
      FB_DSP_TASK,      // FB51               
      SB_DSP_TASK,      // SB51               
      SB_DSP_TASK,      // SBCNF51               
      NBN_DSP_TASK,     // BCCHN         
      ALLC_DSP_TASK,    // ALLC      
      NBS_DSP_TASK,     // EBCCHS       
      NBS_DSP_TASK,     // NBCCHS       
      DDL_DSP_TASK,     // Temporary (BUG IN SIMULATOR)  CB_DSP_TASK,    // SMSCB
      NP_DSP_TASK,      // NP        
      EP_DSP_TASK,      // EP        
      ADL_DSP_TASK,     // ADL     
      AUL_DSP_TASK,     // AUL     
      DDL_DSP_TASK,     // DDL     
      DUL_DSP_TASK,     // DUL     
      TCHD_DSP_TASK,    // TCHD 
      TCHA_DSP_TASK,    // TCHA
      TCHT_DSP_TASK,    // TCHTF
      TCHT_DSP_TASK,    // TCHTH
      NBN_DSP_TASK,     // BCCHN_TOP == BCCHN
#if ((REL99 == 1) && (FF_BHO == 1))
    FBSB_DSP_TASK,    // FBSB   
#endif
      0,                // SYNCHRO (not meaningfull)
    };                
  #else
    const T_TASK_MFTAB  TASK_ROM_MFTAB[NBR_DL_L1S_TASKS] =
    {
      { BLOC_HWTEST,       BLOC_HWTEST_SIZE  },       // HWTEST 
      { BLOC_ADC,          BLOC_ADC_SIZE     },       // ADC in CS_MODE0
      { NULL,              0                 },       // DEDIC   (not meaningfull)
      { BLOC_RAACC,        BLOC_RAACC_SIZE   },       // RAACC                 
      { NULL,              0                 },       // RAHO    (not meaningfull)
      { NULL,              0                 },       // NSYNC   (not meaningfull)             
      { BLOC_POLL ,        BLOC_POLL_SIZE    },       // POLL
      { BLOC_PRACH,        BLOC_PRACH_SIZE   },       // PRACH  
      { BLOC_ITMEAS,       BLOC_ITMEAS_SIZE  },       // ITMEAS 
      { BLOC_FBNEW,        BLOC_FBNEW_SIZE   },       // FBNEW 
      { BLOC_SBCONF,       BLOC_SBCONF_SIZE  },       // SBCONF 
      { BLOC_SB2,          BLOC_SB2_SIZE     },       // SB2 
      { BLOC_PTCCH,        BLOC_PTCCH_SIZE   },       // PTCCH  
      { BLOC_FB26,         BLOC_FB26_SIZE    },       // FB26               
      { BLOC_SB26,         BLOC_SB26_SIZE    },       // SB26
      { BLOC_SBCNF26,      BLOC_SBCNF26_SIZE },       // SBCNF26
      { BLOC_FB51,         BLOC_FB51_SIZE    },       // FB51               
      { BLOC_SB51,         BLOC_SB51_SIZE    },       // SB51               
      { BLOC_SBCNF51,      BLOC_SBCNF51_SIZE },       // SBCNF51
      { BLOC_PDTCH,        BLOC_PDTCH_SIZE   },       // PDTCH   
      { BLOC_BCCHN,        BLOC_BCCHN_SIZE   },       // BCCHN   
      { BLOC_ALLC,         S_RECT4_SIZE      },       // ALLC
      { BLOC_EBCCHS,       S_RECT4_SIZE      },       // EBCCHS 
      { BLOC_NBCCHS,       S_RECT4_SIZE      },       // NBCCHS 
      { BLOC_ADL,          S_RECT4_SIZE      },       // ADL
      { BLOC_AUL,          S_RECT4_SIZE      },       // AUL                 
      { BLOC_DDL,          S_RECT4_SIZE      },       // DDL
      { BLOC_DUL,          S_RECT4_SIZE      },       // DUL     
      { BLOC_TCHD,         BLOC_TCHT_SIZE    },       // TCHD
      { BLOC_TCHA,         BLOC_TCHA_SIZE    },       // TCHA
      { BLOC_TCHTF,        BLOC_TCHT_SIZE    },       // TCHTF
      { BLOC_TCHTH,        BLOC_TCHT_SIZE    },       // TCHTH
      { BLOC_PALLC,        BLOC_PCCCH_SIZE   },       // PALLC
      { BLOC_SMSCB,        BLOC_SMSCB_SIZE   },       // SMSCB
      { BLOC_PBCCHS,       BLOC_PBCCHS_SIZE  },       // PBCCHS
      { BLOC_PNP,          BLOC_PCCCH_SIZE   },       // PNP
      { BLOC_PEP,          BLOC_PCCCH_SIZE   },       // PEP
      { BLOC_SINGLE,       BLOC_SINGLE_SIZE  },       // SINGLE        
      { BLOC_PBCCHN_TRAN,  BLOC_PBCCHN_TRAN_SIZE },   // PBCCHN_TRAN   
      { BLOC_PBCCHN_IDLE,  BLOC_PBCCHN_IDLE_SIZE },   // PBCCHN_IDLE   
      { BLOC_BCCHN_TRAN,   BLOC_BCCHN_TRAN_SIZE },    // BCCHN_TRAN  
      { BLOC_NP,           S_RECT4_SIZE      },       // NP  
      { BLOC_EP,           S_RECT4_SIZE      },       // EP  
      { BLOC_BCCHN_TOP,    BLOC_BCCHN_TOP_SIZE},      // BCCHN_TOP  
#if ((REL99 == 1) && (FF_BHO == 1))
      { BLOC_FBSB,         BLOC_FBSB_SIZE        },   // FBSB   
#endif
      { BLOC_SYNCHRO,      BLOC_SYNCHRO_SIZE }        // SYNCHRO
    };

    const UWORD8 DSP_TASK_CODE[NBR_DL_L1S_TASKS] =
    {
      CHECKSUM_DSP_TASK,// HWTEST 
      0,                // ADC   (not meaningfull)
      0,                // DEDIC (not meaningfull)
      RACH_DSP_TASK,    // RAACC     
      RACH_DSP_TASK,    // RAHO
      0,                // NSYNC  (not meaningfull)
      0,                // POLL   (not meaningfull)
      0,                // PRACH  (not meaningfull)
      0,                // ITMEAS 
      FB_DSP_TASK,      // FBNEW    
      SB_DSP_TASK,      // SBCONF
      SB_DSP_TASK,      // SB2     
      PTCCHU_DSP_TASK,  // PTCCH  
      TCH_FB_DSP_TASK,  // FB26               
      TCH_SB_DSP_TASK,  // SB26               
      TCH_SB_DSP_TASK,  // SBCNF26               
      FB_DSP_TASK,      // FB51               
      SB_DSP_TASK,      // SB51               
      SB_DSP_TASK,      // SBCNF51               
      0,                // PDTCH  (not meaningfull)
      NBN_DSP_TASK,     // BCCHN         
      ALLC_DSP_TASK,    // ALLC      
      NBS_DSP_TASK,     // EBCCHS       
      NBS_DSP_TASK,     // NBCCHS       
      ADL_DSP_TASK,     // ADL     
      AUL_DSP_TASK,     // AUL     
      DDL_DSP_TASK,     // DDL     
      DUL_DSP_TASK,     // DUL     
      TCHD_DSP_TASK,    // TCHD 
      TCHA_DSP_TASK,    // TCHA
      TCHT_DSP_TASK,    // TCHTF
      TCHT_DSP_TASK,    // TCHTH
      0,                // PALLC  (not meaningfull)
      DDL_DSP_TASK,     // Temporary (BUG IN SIMULATOR)  CB_DSP_TASK,    // SMSCB
      DDL_DSP_TASK,     // PBCCHS (In order to allow PBCCHS running in CS or Idle mode, we have to specify a valid DSP task in order to request a PBCCHS with the GSM scheduler)
      0,                // PNP    (not meaningfull)
      0,                // PEP    (not meaningfull)
      0,                // SINGLE (not meaningfull) 
      0,                // PBCCHN_TRAN (not meaningfull)    
      DDL_DSP_TASK,     // PBCCHN_IDLE (only for GSM scheduler the task used is the same as SMSCB task)
      NBN_DSP_TASK,     // BCCHN_TRAN == BCCHN
      NP_DSP_TASK,      // NP        
      EP_DSP_TASK,      // EP        
      NBN_DSP_TASK,     // BCCHN_TOP == BCCHN
#if ((REL99 == 1) && (FF_BHO == 1))
      FBSB_DSP_TASK,    // FBSB   
#endif
      0                 // SYNCHRO (not meaningfull)
    };                

  #endif

    const UWORD8 REPORTING_PERIOD[] =
    {
      255,            // INVALID_CHANNEL -> invalid reporting period
      104,            // TCH_F              
      104,            // TCH_H              
      102,            // SDCCH_4            
      102             // SDCCH_8            
    }; 
    
    const UWORD8 TOA_PERIOD_LEN[] =
    {
      0,              // CS_MODE0 not used for histogram filling       
      12,             // CS_MODE histogram length            
      12,             // I_MODE histogram length            
      12,             // CON_EST_MODE1 histogram length  
     144,             // CON_EST_MODE2 histogram length      
      36,             // DEDIC_MODE (Full rate) histogram length        
      42,             // DEDIC_MODE (Half rate) histogram length        
      #if L1_GPRS
        16,           // PACKET TRANSFER MODE histogram length
      #endif
    }; 
    
   // #if (STD == GSM) 
      const UWORD8 MIN_TXPWR_GSM[] =
      {
        0,  // unused.
        0,  // Power class = 1, unused for GSM900
        2,  // Power class = 2.
        3,  // Power class = 3.
        5,  // Power class = 4.
        7   // Power class = 5.
      };
   // #elif (STD == PCS1900)
      const UWORD8 MIN_TXPWR_PCS[] =
      {
        0,  // unused.
        0,  // Power class = 1.
        3,  // Power class = 2.
       30   // Power class = 3.
      };
   // #elif (STD == DCS1800)
      const UWORD8 MIN_TXPWR_DCS[] =
      {
        0,  // unused.
        0,  // Power class = 1.
        3,  // Power class = 2.
       29   // Power class = 3.
      };

       const UWORD8 MIN_TXPWR_GSM850[] =
      {
        0,  // unused.
        0,  // Power class = 1, unused for GSM900
        2,  // Power class = 2.
        3,  // Power class = 3.
        5,  // Power class = 4.
        7   // Power class = 5.
      };

  //  #elif (STD == DUAL)
   //   const UWORD8 MIN_TXPWR_GSM[] =
   //  {
   //     0,  // unused.
   //     0,  // Power class = 1, unused for GSM900
   //     2,  // Power class = 2.
   //     3,  // Power class = 3.
   //     5,  // Power class = 4.
   //     7   // Power class = 5.
   //   };
   //   const UWORD8 MIN_TXPWR_DCS[] =
   //   {
   //     0,  // unused.
   //     0,  // Power class = 1.
   //     3,  // Power class = 2.
   //    29   // Power class = 3.
   //   };
  //  #endif

  const UWORD8 GAUG_VS_PAGING_RATE[] =
  {
    4,   // bs_pa_mfrms = 2, 1 gauging every 4 Paging blocs
    3,   // bs_pa_mfrms = 3, 1 gauging every 3 Paging blocs
    2,   // bs_pa_mfrms = 4, 1 gauging every 2 Paging blocs
    1,   // bs_pa_mfrms = 5, 1 gauging every 1 Paging bloc
    1,   // bs_pa_mfrms = 6, 1 gauging every 1 Paging bloc
    1,   // bs_pa_mfrms = 7, 1 gauging every 1 Paging bloc
    1,   // bs_pa_mfrms = 8, 1 gauging every 1 Paging bloc
    1    // bs_pa_mfrms = 9, 1 gauging every 1 Paging bloc
  };   
    
  #else
    extern T_IDLE_TASK_INFO IDLE_INFO_COMB[(MAX_AG_BLKS_RES_COMB+1) * (MAX_PG_BLOC_INDEX_COMB+1)];
    extern T_IDLE_TASK_INFO IDLE_INFO_NCOMB[(MAX_AG_BLKS_RES_NCOMB+1) * (MAX_PG_BLOC_INDEX_NCOMB+1)];
    extern UWORD8           NBPCH_IN_MF51_NCOMB[(MAX_AG_BLKS_RES_NCOMB+1)];
    extern UWORD8           NBPCH_IN_MF51_COMB[(MAX_AG_BLKS_RES_COMB+1)];
    extern UWORD8           DSC_INIT_VALUE[MAX_BS_PA_MFRMS-2];
    extern T_SDCCH_DESC     SDCCH_DESC_NCOMB[];
    extern T_SDCCH_DESC     SDCCH_DESC_COMB[];
    extern UWORD8           RNTABLE[114];
    extern UWORD8           COMBINED_RA_DISTRIB[51];
    extern T_TASK_MFTAB     TASK_ROM_MFTAB[NBR_DL_L1S_TASKS];
    extern UWORD8           DSP_TASK_CODE[NBR_DL_L1S_TASKS];
    extern UWORD8           REPORTING_PERIOD[];
    extern UWORD8           TOA_PERIOD_LEN[];
    extern UWORD8           MIN_TXPWR_GSM[];
    extern UWORD8           MIN_TXPWR_DCS[];
    extern UWORD8           MIN_TXPWR_PCS[];
    extern UWORD8           MIN_TXPWR_GSM850[];
    extern UWORD8           GAUG_VS_PAGING_RATE[];
  #endif
#endif //L1_TABS_H

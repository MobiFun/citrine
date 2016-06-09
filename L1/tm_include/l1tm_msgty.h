 /************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1TM_MSGTY.H
 *
 *        Filename l1tm_msgty.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/


/***********************************************************************/
/*                           TESTMODE 3.X                              */
/***********************************************************************/


typedef struct
{ 
  UWORD8 cid;
  UWORD8 str_len_in_bytes;

  // all primitive types should be a unique struct within
  // the union u.
  union
    {
    struct
    {
      WORD16 index;
      UWORD16 value;
    } tm_params;
    struct
    {
      WORD8 index;
      UWORD8 table[TM_PAYLOAD_UPLINK_SIZE_MAX];
    } tm_table;
    struct
    {
      UWORD32 address;
      UWORD8 table[TM_PAYLOAD_UPLINK_SIZE_MAX];
    } mem_write;
    struct
    {
      UWORD32 src;
      UWORD32 length;
    } mem_read;
    struct
    {
      UWORD8 packet[128];
    } ffs;
  } u;
}
T_TESTMODE_PRIM;

typedef struct
{ 
  UWORD32          arfcn;
  UWORD32          number_of_measurements;
  UWORD8           place_of_measurement;
  UWORD32          num_loop;  
  UWORD32          agc;
}
T_TMODE_PM_REQ;

typedef struct
{
  UWORD16       power_array_size;
  T_POWER_ARRAY power_array[1];
}
T_TMODE_RXLEV_REQ;

typedef struct
{ 
  UWORD32   dummy; 
}
T_TMODE_FB0_REQ;

typedef struct
{ 
  UWORD32   dummy; 
}
T_TMODE_FB1_REQ;

typedef struct
{ 
  UWORD32   dummy;
}
T_TMODE_SB_REQ;

typedef struct
{ 
  UWORD32   dummy; 
}
T_TMODE_FB_SB_REQ;

typedef struct
{
  BOOL          fb_flag;    //TRUE if FB found, otherwise FALSE 
  WORD8         ntdma;      //tdma between window start and beginning of FB (0..23)
  UWORD8        neigh_id;
  UWORD32       pm_fullres;
  UWORD32       toa;
  WORD16        angle;
  UWORD32       snr;
}
T_TMODE_FB_CON;

typedef struct
{
  UWORD16     radio_freq;
  BOOL        sb_flag;
  UWORD32     fn_offset;
  UWORD32     time_alignmt;
  UWORD8      bsic;
  UWORD8      neigh_id;
  UWORD8      attempt;
  UWORD32     pm_fullres;
  UWORD32     toa;
  WORD16      angle;
  UWORD32     snr;  
}
T_TMODE_NCELL_SYNC_IND;

typedef struct
{
  UWORD32     fn_offset;
  UWORD32     time_alignmt;
  UWORD8      bsic;
}
T_TMODE_NEW_SCELL_REQ;

typedef struct
{
  UWORD16        radio_freq;
  UWORD8         l2_channel;
  BOOL           error_flag;
  T_RADIO_FRAME  l2_frame;
  UWORD8         tc;
  UWORD32        fn;
  UWORD8         neigh_id;
}
T_TMODE_BCCHS_CON; 

typedef struct
{
  UWORD32 dummy;
}
T_TMODE_STOP_SCELL_BCCH_REQ;

typedef struct
{
  UWORD32 dummy;
}
T_TMODE_SCELL_NBCCH_REQ;

typedef struct 
{
  UWORD32   fn;
  UWORD8    channel_request;
} 
T_TMODE_RA_DONE;

typedef struct
{
  UWORD32   dummy;
}
T_TMODE_RA_START;

typedef struct
{
  #if (CODE_VERSION == SIMULATION)
    UWORD8  ul_dl;
  #else
    UWORD32 dummy;
  #endif
}  
T_TMODE_IMMED_ASSIGN_REQ;

typedef struct
{
  UWORD8  A[22+1];
}
T_TMODE_RADIO_FRAME;

typedef struct
{
  UWORD16      radio_freq;
  UWORD8       l2_channel;
  UWORD8       error_cause;
  T_TMODE_RADIO_FRAME l2_frame;
  UWORD8       bsic;
  UWORD8       tc;
}
T_TMODE_SACCH_INFO;

typedef struct
{
  UWORD32  pm_fullres;
  UWORD32  snr;
  UWORD32  toa;
  WORD16   angle;
  UWORD32  qual_nbr_meas_full; // Fullset: nbr meas. of rxqual.
  UWORD32  qual_full;          // Fullset: rxqual meas.
}
T_TMODE_TCH_INFO;

typedef struct
{
  UWORD32 none;
}
T_TMODE_STOP_RX_TX;

#if L1_GPRS
  typedef struct
  {
    #if (CODE_VERSION == SIMULATION)
      UWORD8 multislot_class;
      UWORD8 dl_ts_alloc;
      UWORD8 ul_ts_alloc;
      UWORD8 ul_alloc_length;
      BOOL   mon_enable;
      BOOL   pm_enable;
    #else
      UWORD32 dummy;
    #endif
  }  
  T_TMODE_PDTCH_ASSIGN_REQ;

  typedef struct
  {
    UWORD32  pm_fullres;
    UWORD32  snr;
    UWORD32  toa;
    WORD16   angle;
    BOOL     crc_error_tbl[8];
  }
  T_TMODE_PDTCH_INFO;
#endif



/**************** ENUMs ***********************/

// TestMode Error Codes
enum
{
  E_OK         =   0,   // Function completed successfully.
  E_FINISHED   =   1,   // Previously started operation has finished.
  E_TESTMODE   =   2,   // Function not legal in this GGT test mode.
  E_BADINDEX   =   3,   // The index is undefined.
  E_INVAL      =   4,   // Invalid Argument (out of range or other).
  E_BADSIZE    =   7,   // Some table or list parameter was wrong in size
  E_AGAIN      =   8,   // Not ready, try again later.
  E_NOSYS      =   9,   // Function not implemented.
  E_NOSUBSYS   =  10,   // Sub-Function not implemented.
  E_BADCID     =  14,   // Invalid CID.
  E_CHECKSUM   =  15,   // Checksum Error.
  E_PACKET     =  16,    // Packet format is bad (wrong number of arguments).
  E_FORWARD    =  31   // Command parsed successfully, but further processing necessary
};

// CID's 
enum 
{
  TM_INIT                    = 0x20,
  TM_MODE_SET                = 0x21,
  VERSION_GET                = 0x22,
  RF_ENABLE                  = 0x23,
  STATS_READ                 = 0x24,
  STATS_CONFIG_WRITE         = 0x25,
  STATS_CONFIG_READ          = 0x26,
  RF_PARAM_WRITE             = 0x30,
  RF_PARAM_READ              = 0x31,
  RF_TABLE_WRITE             = 0x32,
  RF_TABLE_READ              = 0x33,
  RX_PARAM_WRITE             = 0x34,
  RX_PARAM_READ              = 0x35,
  TX_PARAM_WRITE             = 0x36,
  TX_PARAM_READ              = 0x37,
  TX_TEMPLATE_WRITE          = 0x38,
  TX_TEMPLATE_READ           = 0x39,
  MEM_WRITE                  = 0x40,
  MEM_READ                   = 0x41,
  CODEC_WRITE                = 0x42,
  CODEC_READ                 = 0x43,
  MISC_PARAM_WRITE           = 0x44,
  MISC_PARAM_READ            = 0x45,
  MISC_TABLE_WRITE           = 0x46,
  MISC_TABLE_READ            = 0x47,
  MISC_ENABLE                = 0x48,
  SPECIAL_PARAM_WRITE        = 0x50,
  SPECIAL_PARAM_READ         = 0x51,
  SPECIAL_TABLE_WRITE        = 0x52,
  SPECIAL_TABLE_READ         = 0x53,
  SPECIAL_ENABLE             = 0x54,

  #if (CODE_VERSION != SIMULATION)
    TPU_TABLE_WRITE            = 0x55,
    TPU_TABLE_READ             = 0x56,
  #endif

  TM_FFS                     = 0x70
};

// TestMode function enum's
enum RF_PARAM 
{
  BCCH_ARFCN         = 1,
  TCH_ARFCN          = 2,
  MON_ARFCN          = 3,
  #if L1_GPRS
    PDTCH_ARFCN        = 4,
  #endif
  STD_BAND_FLAG      = 7,
  AFC_ENA_FLAG       = 8,
  AFC_DAC_VALUE      = 9,
  INITIAL_AFC_DAC    = 10
  #if L1_GPRS
    ,MULTISLOT_CLASS    = 20
  #endif
};

enum RF_TABLE 
{
  RX_AGC_TABLE              = 8,
  AFC_PARAMS                = 9,
  RX_AGC_GLOBAL_PARAMS      = 12,
  RX_IL_2_AGC_MAX           = 13,
  RX_IL_2_AGC_PWR           = 14,
  RX_IL_2_AGC_AV            = 15,
  TX_LEVELS                 = 16, // 16=GSM900, 32=DCS1800, 48=PCS1900
  TX_CAL_CHAN               = 17, // 17=GSM900, 33=DCS1800, 49=PCS1900

#if (ORDER2_TX_TEMP_CAL==1)
  TX_CAL_TEMP               = 20, // 20=GSM900, 36=DCS1800, 52=PCS1900
#else
  TX_CAL_TEMP               = 18, // 18=GSM900, 34=DCS1800, 50=PCS1900
#endif

  TX_CAL_EXTREME            = 19, // 19=GSM900, 35=DCS1800, 51=PCS1900
  RX_CAL_CHAN               = 25, // 25=GSM900, 41=DCS1800, 57=PCS1900
  RX_CAL_TEMP               = 26, // 26=GSM900, 42=DCS1800, 58=PCS1900
  RX_CAL_LEVEL              = 27, // 27=GSM900, 43=DCS1800, 59=PCS1900
  RX_AGC_PARAMS             = 31, // 31=GSM900, 47=DCS1800, 63=PCS1900
  RX_AGC_PARAMS_PCS         = 63,
  #if (RF_FAM == 35)
    RX_PLL_TUNING_TABLE     = 65, 
  #endif 
  TX_DATA_BUFFER            = 80
  #if L1_GPRS
   ,RLC_TX_BUFFER_CS1       = 81,
    RLC_TX_BUFFER_CS2       = 82,
    RLC_TX_BUFFER_CS3       = 83,
    RLC_TX_BUFFER_CS4       = 84
  #endif
};

enum RX_PARAM 
{
  RX_AGC_GAIN             = 1,
  RX_TIMESLOT             = 2,
  RX_AGC_ENA_FLAG         = 8,
  RX_PM_ENABLE            = 9,
  RX_FRONT_DELAY          = 10,
  RX_FLAGS_CAL            = 14,
  RX_FLAGS_PLATFORM       = 15,
  RX_FLAGS_IQ_SWAP        = 17,
  RX_FLAGS_ALL            = 18
  #if L1_GPRS
    ,RX_GPRS_SLOTS          = 28,
    RX_GPRS_CODING          = 29
  #endif
};

enum TX_PARAM 
{
  TX_PWR_LEVEL            = 1,
  TX_APC_DAC              = 4,
  TX_RAMP_TEMPLATE        = 5,
  TX_CHAN_CAL_TABLE       = 6,
  TX_RESERVED             = 7,
  TX_BURST_TYPE           = 8,
  TX_BURST_DATA           = 9,
  TX_TIMING_ADVANCE       = 10,
  TX_TRAINING_SEQ         = 11,
  TX_PWR_SKIP             = 13,
  TX_FLAGS_CAL            = 14,
  TX_FLAGS_PLATFORM       = 15,
  TX_FLAGS_IQ_SWAP        = 17,
  TX_FLAGS_ALL            = 18
  #if L1_GPRS
    ,TX_GPRS_POWER0         = 20,
    TX_GPRS_POWER1          = 21,
    TX_GPRS_POWER2          = 22,
    TX_GPRS_POWER3          = 23,
    TX_GPRS_POWER4          = 24,
    TX_GPRS_POWER5          = 25,
    TX_GPRS_POWER6          = 26,
    TX_GPRS_POWER7          = 27,
    TX_GPRS_SLOTS           = 28,
    TX_GPRS_CODING          = 29
  #endif
};

enum MISC_PARAM 
{
  GPIOSTATE0                = 8,
  GPIODIR0                  = 9,
  GPIOSTATE1                = 10,
  GPIODIR1                  = 11,
  GPIOSTATE0P               = 12,
  GPIODIR0P                 = 13,
  GPIOSTATE1P               = 14,
  GPIODIR1P                 = 15,
  ADC_INTERVAL              = 18,
  ADC_ENA_FLAG              = 19,
  CONVERTED_ADC0            = 20,
  CONVERTED_ADC1            = 21,
  CONVERTED_ADC2            = 22,
  CONVERTED_ADC3            = 23,
  CONVERTED_ADC4            = 24,
  CONVERTED_ADC5            = 25,
  CONVERTED_ADC6            = 26,
  CONVERTED_ADC7            = 27,
  CONVERTED_ADC8            = 28,
  RAW_ADC0                  = 30,
  RAW_ADC1                  = 31,
  RAW_ADC2                  = 32,
  RAW_ADC3                  = 33,
  RAW_ADC4                  = 34,
  RAW_ADC5                  = 35,
  RAW_ADC6                  = 36,
  RAW_ADC7                  = 37,
  RAW_ADC8                  = 38,
  ADC0_COEFF_A              = 50,
  ADC1_COEFF_A              = 51,
  ADC2_COEFF_A              = 52,
  ADC3_COEFF_A              = 53,
  ADC4_COEFF_A              = 54,
  ADC5_COEFF_A              = 55,
  ADC6_COEFF_A              = 56,
  ADC7_COEFF_A              = 57,
  ADC8_COEFF_A              = 58,
  ADC0_COEFF_B              = 60,
  ADC1_COEFF_B              = 61,
  ADC2_COEFF_B              = 62,
  ADC3_COEFF_B              = 63,
  ADC4_COEFF_B              = 64,
  ADC5_COEFF_B              = 65,
  ADC6_COEFF_B              = 66,
  ADC7_COEFF_B              = 67,
  ADC8_COEFF_B              = 68,
  SLEEP_MODE                = 80,
  CURRENT_TM_MODE           = 127
};

enum STATS_CONFIG 
{
  LOOPS                     = 16,
  AUTO_RESULT_LOOPS         = 17,
  AUTO_RESET_LOOPS          = 18,
  #if L1_GPRS
    STAT_GPRS_SLOTS           = 20,
  #endif
  STAT_TYPE                 = 24,
  STAT_BITMASK              = 25
};

enum STATS_READ 
{
  ACCUMULATED_RX_STATS      = 1,
  MOST_RECENT_RX_STATS      = 2
};

enum BITMASK 
{
  RSSI                      = 0x0001,
  DSP_PM                    = 0x0002,
  ANGLE_MEAN                = 0x0004,
  ANGLE_VAR                 = 0x0008,
  SNR_MEAN                  = 0x0010,
  SNR_VAR                   = 0x0020,
  TOA_MEAN                  = 0x0040,
  TOA_VAR                   = 0x0080,
  RESERVED1                 = 0x0100,
  RESERVED2                 = 0x0200,
  ANGLE_MIN                 = 0x0400,
  ANGLE_MAX                 = 0x0800,
  FRAME_NUMBER              = 0x1000,
  RUNS                      = 0x2000,
  SUCCESSES                 = 0x4000,
  BSIC                      = 0x8000
};

enum RF_ENABLE_E 
{
  STOP_ALL                  =  0,
  RX_TCH                    =  1,
  TX_TCH                    =  2,
  RX_TX_TCH                 =  3,
  #if L1_GPRS
    RX_TX_PDTCH              =  4,
  #endif
  RX_TCH_CONT               =  8,
  TX_TCH_CONT               =  9,
  BCCH_LOOP                 = 10,
  SB_LOOP                   = 11,
  FB1_LOOP                  = 12,
  FB0_LOOP                  = 13,
  SINGLE_PM                 = 15,
  #if L1_GPRS
    RX_TX_PDTCH_MON           = 16,
  #endif
  #if (RF_FAM == 35)
    RX_PLL_TUNING             =  17,
  #endif
  RX_TX_MON_TCH             = 19,
  RX_TX_MON                 = 27
};
  
enum VERSION_GET_E 
{
  BBCHIP_MODULE_REV         = 0x10,
  CHIPID_MODULE_REV         = 0x14,
  CHIPVER_MODULE_REV        = 0x15,
  DSPSW_MODULE_REV          = 0x22,
  ANALOGCHIP_MODULE_REV     = 0x30,
  GSM_MODULE_REV            = 0x80,
  LAYER1_MODULE_REV         = 0x84,
  RFDRIVER_MODULE_REV       = 0x88,
  TM_API_MODULE_REV         = 0xE0,
  L1_TM_CORE_MODULE_REV     = 0xE1,
  STD_MODULE_REV            = 0xE2,
  DSP_MODULE_REV            = 0xE3,
  BOARD_MODULE_REV          = 0xE4,
  RF_MODULE_REV             = 0xE5 
};


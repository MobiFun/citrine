/*
+-------------------------------------------------------------------+
| PROJECT: GSM-PS                     $Workfile:: pcmdata.c        $|
| $Author: mmj                        $Revision: 1.1.1.2           $|
| CREATED: 26.11.1998                 $Modtime:: 23.03.00 16:23    $|
+-------------------------------------------------------------------+

   MODULE  : PCM

   PURPOSE : This Modul defines the data & values for the permanent 
             memory configuration driver.
*/			 


/*==== INCLUDES ===================================================*/

#define __STANDARD_H__ /* Avoid to define UBYTE, UINT16 and UINT32. */

#define NEQ !=

#include "../../include/config.h"
#include "../../riviera/rv/general.h"
#include "pcm.h" 

/* FreeCalypso configuration */
#define	STD	6

/*==== VARIABLES ==================================================*/
/*
 * RAM for the copy of PCM data
 */

UBYTE pcm_mem [PCM_SIZE];

UBYTE std = STD;
// set treshold for initial power measurements (= -106 dBm)
USHORT min_rxlev = 20;


#define START_MSCAP    0
#define START_IMEI     START_MSCAP    + LEN_MSCAP
#define START_IMSI     START_IMEI     + LEN_IMEI
#define START_SMS      START_IMSI     + LEN_IMSI
#define START_CLASS2   START_SMS      + LEN_SMS
#define START_CLASS3   START_CLASS2   + LEN_CLASS2
#define START_MSSUP    START_CLASS3   + LEN_CLASS3
#define START_CLNG     START_MSSUP    + LEN_MSSUP
#define START_MSSET    START_CLNG     + LEN_CLNG
#define START_HZCACHE  START_MSSET    + LEN_MSSET
#define START_LDN      START_HZCACHE  + LEN_HZCACHE
#define START_LRN      START_LDN      + LEN_LDN
#define START_LMN      START_LRN      + LEN_LRN
#define START_UPN      START_LMN      + LEN_LMN
#define START_MBN      START_UPN      + LEN_UPN
#define START_VMN      START_MBN      + LEN_MBN
#define START_CTIM     START_VMN      + LEN_VMN
#define START_CCNT     START_CTIM     + LEN_CTIM
#define START_ECC      START_CCNT     + LEN_CCNT
#define START_ORG      START_ECC      + LEN_ECC
#define START_CCP      START_ORG      + LEN_ORG
#define START_EXT1     START_CCP      + LEN_CCP
#define START_SIMLCK   START_EXT1     + LEN_EXT1
#define START_MAIN     START_SIMLCK   + LEN_SIMLCK
#define START_SFK      START_MAIN     + LEN_MAIN
#define START_FAULT    START_SFK      + LEN_SFK
#define START_DEBUG    START_FAULT    + LEN_FAULT
#define START_POWER    START_DEBUG    + LEN_DEBUG
#define START_KEYB     START_POWER    + LEN_POWER
#define START_RADIO    START_KEYB     + LEN_KEYB
#define START_CGMI     START_RADIO    + LEN_RADIO
#define START_INF0     START_CGMI     + LEN_CGMI
#define START_CGMM     START_INF0     + LEN_INF0
#define START_CGMR     START_CGMM     + LEN_CGMM
#define START_CGSN     START_CGMR     + LEN_CGMR
#define START_SMSPRFL  START_CGSN     + LEN_CGSN
#define START_PLMN     START_SMSPRFL  + LEN_SMSPRFL
#define START_BCCHINFO START_PLMN     + LEN_PLMN
#define START_ALS      START_BCCHINFO + LEN_BCCHINFO
#define START_LOCGPRS  START_ALS      + LEN_ALS
#define START_KCGPRS   START_LOCGPRS  + LEN_LOCGPRS
#define START_IMSIGPRS START_KCGPRS   + LEN_KCGPRS


const T_PCM_DESCRIPTION pcm_table[] =
{
  {"/pcm/" "MSCAP"   , START_MSCAP   , SIZE_EF_MSCAP    + 2, NR_EF_MSCAP    },
  {"/pcm/" "IMEI"    , START_IMEI    , SIZE_EF_IMEI     + 2, NR_EF_IMEI     },
  {"/pcm/" "IMSI"    , START_IMSI    , SIZE_EF_IMSI     + 2, NR_EF_IMSI     },
  {"/pcm/" "SMS "    , START_SMS     , SIZE_EF_SMS      + 2, NR_EF_SMS      },
  {"/pcm/" "CLASS2"  , START_CLASS2  , SIZE_EF_CLASS2   + 2, NR_EF_CLASS2   },
  {"/pcm/" "CLASS3"  , START_CLASS3  , SIZE_EF_CLASS3   + 2, NR_EF_CLASS3   },
  {"/pcm/" "MSSUP"   , START_MSSUP   , SIZE_EF_MSSUP    + 2, NR_EF_MSSUP    },
  {"/pcm/" "CLNG"    , START_CLNG    , SIZE_EF_CLNG     + 2, NR_EF_CLNG     },
  {"/pcm/" "MSSET"   , START_MSSET   , SIZE_EF_MSSET    + 2, NR_EF_MSSET    },
  {"/pcm/" "HZCACHE" , START_HZCACHE , SIZE_EF_HZCACHE  + 2, NR_EF_HZCACHE  },
  {"/pcm/" "LDN"     , START_LDN     , SIZE_EF_LDN      + 2, NR_EF_LDN      },
  {"/pcm/" "LRN"     , START_LRN     , SIZE_EF_LRN      + 2, NR_EF_LRN      },
  {"/pcm/" "LMN"     , START_LMN     , SIZE_EF_LMN      + 2, NR_EF_LMN      },
  {"/pcm/" "UPN"     , START_UPN     , SIZE_EF_UPN      + 2, NR_EF_UPN      },
  {"/pcm/" "MBN"     , START_MBN     , SIZE_EF_MBN      + 2, NR_EF_MBN      },
  {"/pcm/" "VMN"     , START_VMN     , SIZE_EF_VMN      + 2, NR_EF_VMN      },
  {"/pcm/" "CTIM"    , START_CTIM    , SIZE_EF_CTIM     + 2, NR_EF_CTIM     },
  {"/pcm/" "CCNT"    , START_CCNT    , SIZE_EF_CCNT     + 2, NR_EF_CCNT     },
  {"/pcm/" "ECC"     , START_ECC     , SIZE_EF_ECC      + 2, NR_EF_ECC      },
  {"/pcm/" "ORG"     , START_ORG     , SIZE_EF_ORG      + 2, NR_EF_ORG      },
  {"/pcm/" "CCP"     , START_CCP     , SIZE_EF_CCP      + 2, NR_EF_CCP      },
  {"/pcm/" "EXT1"    , START_EXT1    , SIZE_EF_EXT1     + 2, NR_EF_EXT1     },
  {"/pcm/" "SIMLCK"  , START_SIMLCK  , SIZE_EF_SIMLCK   + 2, NR_EF_SIMLCK   },
  {"/pcm/" "MAIN"    , START_MAIN    , SIZE_EF_MAIN     + 2, NR_EF_MAIN     },
  {"/pcm/" "SFK"     , START_SFK     , SIZE_EF_SFK      + 2, NR_EF_SFK      },
  {"/pcm/" "FAULT"   , START_FAULT   , SIZE_EF_FAULT    + 2, NR_EF_FAULT    },
  {"/pcm/" "DEBUG"   , START_DEBUG   , SIZE_EF_DEBUG    + 2, NR_EF_DEBUG    },
  {"/pcm/" "POWER"   , START_POWER   , SIZE_EF_POWER    + 2, NR_EF_POWER    },
  {"/pcm/" "KEYB"    , START_KEYB    , SIZE_EF_KEYB     + 2, NR_EF_KEYB     },
  {"/pcm/" "RADIO"   , START_RADIO   , SIZE_EF_RADIO    + 2, NR_EF_RADIO    },
  {"/pcm/" "CGMI"    , START_CGMI    , SIZE_EF_CGMI     + 2, NR_EF_CGMI     },
  {"/pcm/" "INF0"    , START_INF0    , SIZE_EF_INF0     + 2, NR_EF_INF0     },
  {"/pcm/" "CGMM"    , START_CGMM    , SIZE_EF_CGMM     + 2, NR_EF_CGMM     },
  {"/pcm/" "CGMR"    , START_CGMR    , SIZE_EF_CGMR     + 2, NR_EF_CGMR     },
  {"/pcm/" "CGSN"    , START_CGSN    , SIZE_EF_CGSN     + 2, NR_EF_CGSN     },
  {"/pcm/" "SMSPRFL" , START_SMSPRFL , SIZE_EF_SMSPRFL  + 2, NR_EF_SMSPRFL  },
  {"/pcm/" "PLMN"    , START_PLMN    , SIZE_EF_PLMN     + 2, NR_EF_PLMN     },
  {"/pcm/" "BCCHINF" , START_BCCHINFO, SIZE_EF_BCCHINFO + 2, NR_EF_BCCHINFO },
  {"/pcm/" "ALS"     , START_ALS     , SIZE_EF_ALS      + 2, NR_EF_ALS      },
  {"/pcm/" "LOCGPRS" , START_LOCGPRS , SIZE_EF_LOCGPRS  + 2, NR_EF_LOCGPRS  },
  {"/pcm/" "KCGPRS"  , START_KCGPRS  , SIZE_EF_KCGPRS   + 2, NR_EF_KCGPRS   },
  {"/pcm/" "IMSIGPRS", START_IMSIGPRS, SIZE_EF_IMSIGPRS + 2, NR_EF_IMSIGPRS },
  {0                , 0             , 0                   , 0              }
};


const UBYTE pcm_default_values[] = 
{                               
  /********************************************************************
   * Field MOBILE CAPABILITIES - MSCAP
   ********************************************************************/
  /*
   * FreeCalypso: the change of L1 to the reconstructed TCS211 version
   * has fixed the EFR codec, so we can re-enable it now, but AMR is
   * still broken, hence we are going to advertise as non-AMR-capable
   * despite running on AMR-capable silicon.
   *
   * If you would like to experiment with different codec
   * configurations, you can do so without having to recompile
   * and reflash the firmware each time: just write a /pcm/MSCAP
   * file into FFS with whatever setting you wish to try.
   */
  #if 0 //((DSP == 34) || (DSP == 35) || (DSP == 36)) // ROM Codes including AMR feature.
    #if (STD == 1) // GSM 900
      #if defined (FAX_AND_DATA) 
        0xB1,   0xC7,   0x00,                     
      #else                                             
        0x31,   0x00,   0x00,                     
      #endif                                            
    #elif ((STD == 3) || (STD == 4)) // DCS 1800 or PCS 1900
      #if defined (FAX_AND_DATA) 
        0xB7,   0xC7,   0x00,                     
      #else       
        #if defined (TM_SPECIAL)
          0x31,   0x00,   0x00,
        #else
          0x37,   0x00,   0x00,
        #endif
      #endif
    #elif (STD == 5) // Dualband GSM 900 / DCS 1800
      #if defined (FAX_AND_DATA) 
        0xB7,   0xC7,   0x00,                     
      #else                                             
        0x37,   0x00,   0x00,                     
      #endif                                            
    #elif (STD == 6) // Dualband GSM 900 / E-GSM / DCS 1800
      #if defined (FAX_AND_DATA) 
        0xB7,   0xC7,   0x00,                     
      #else                                             
        0x37,   0x00,   0x00,                     
      #endif                                            
    #endif // STD = 1, 3, 4, 5 or 6
  #else // DSP = 16, 17, 30, 31, 32 or 33
    #if (STD == 1) // GSM 900
      #if defined (FAX_AND_DATA) 
        #if (OP_WCP == 1)
        0x85,   0xC7,   0x00,
        #else
        0x81,   0xC7,   0x00,                     
        #endif
      #else                                             
        0x01,   0x00,   0x00,                     
      #endif                                            
    #elif ((STD == 3) || (STD == 4)) // DCS 1800 or PCS 1900
      #if defined (FAX_AND_DATA) 
        #if (OP_WCP == 1)
        0x85,   0xC7,   0x00,
        #else
        0x87,   0xC7,   0x00,                     
        #endif
      #else       
        #if defined (TM_SPECIAL)
          0x01,   0x00,   0x00,
        #else
          0x07,   0x00,   0x00,
        #endif
      #endif
    #elif (STD == 5) // Dualband GSM 900 / DCS 1800
      #if defined (FAX_AND_DATA) 
        #if (OP_WCP == 1)
        0x85,   0xC7,   0x00,
        #else
        0x87,   0xC7,   0x00,                     
        #endif
      #else                                             
        0x07,   0x00,   0x00,                     
      #endif                                            
    #elif (STD == 6) // Dualband GSM 900 / E-GSM / DCS 1800
      #if defined (FAX_AND_DATA) 
        #if (OP_WCP == 1)
        0x85,   0xC7,   0x00,
        #else
        0x87,   0xC7,   0x00,                     
        #endif
      #else                                             
        0x07,   0x00,   0x00,                     
      #endif                                            
    #endif // STD = 1, 3, 4, 5 or 6
  #endif // DSP
  0x00,   0x00,   0x00,                                                 
        
  /********************************************************************
   * Field INTERNATIONAL MOBILE EQUIPMENT ID - IMEI
   ********************************************************************/
  0x44,   0x06,   0x91,   0x91,   0x57,   0x70,   0x95,   0x00,

  /********************************************************************
   * Field INTERNATIONAL MOBILE SUBSCRIBER ID - IMSI
   ********************************************************************/
  0x0F,
  0x44,   0x06,   0x91,   0x91,   0x57,   0x70,   0x95,   0xF0,

  /********************************************************************
   * Field SHORT MESSAGE SERVICE - SMS
   ********************************************************************/
  0x10,   0x11,   0x12,   0x13,   0x14,   0x15,   0x16,   0x17,
  0x18,   0x19,   0x1A,   0x1B,   0x1C,   0x1D,   0x1E,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,
  0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,   0x1F,

  /********************************************************************
   * Field MOBILE STATION CLASSMARK 2 & 3 - CLASS2 & CLASS3
   ********************************************************************/
  #if (STD == 1) // GSM 900
    0x33,   0x18,   0x01,
    0x00,   0x00,   0x10,
  #elif ((STD == 3) || (STD == 4)) // DCS 1800 or PCS 1900
    0x30,   0x18,   0x01,
    0x00,   0x00,   0x10,
  #elif (STD == 5) // Dualband GSM 900 / DCS 1800
    0x30,   0x18,   0x81,
    0x50,   0x14,   0x10,
  #elif (STD == 6) // Dualband GSM 900 / E-GSM / DCS 1800
    0x30,   0x19,   0x81,
    0x60,   0x14,   0x10,
  #endif // STD =1, 3, 4, 5 or 6

  /********************************************************************
   * Field MOBILE SETUP - MSSUP
   ********************************************************************/
  0xB7,   0x46,   0x47,   0x1F,   0x00,

  /********************************************************************
   * Field CURRENT LANGUAGE - CLNG
   ********************************************************************/
  'a',    'u',

  /********************************************************************
   * Field MOBILE STATION SETTTINGS - MSSET
   ********************************************************************/
  0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,
  0x38,   0x39,

  /********************************************************************
   * Field HOMEZONE CACHE record 1 - HZCACHE
   ********************************************************************/
  0xFF,   0xFF,   0xFF, 
  
  /********************************************************************
   * Field HOMEZONE CACHE record 2 - HZCACHE
   ********************************************************************/
  0xFF,   0xFF,   0xFF,

  /********************************************************************
   * Field HOMEZONE CACHE record 3 - HZCACHE
   ********************************************************************/
  0xFF,   0xFF,   0xFF,

  /********************************************************************
   * Field HOMEZONE CACHE record 4 - HZCACHE
   ********************************************************************/
  0xFF,   0xFF,   0xFF,

  /********************************************************************
   * Field HOMEZONE CACHE record 5 - HZCACHE
   ********************************************************************/
  0xFF,   0xFF,   0xFF,

  /********************************************************************
   * Field LAST MOC NUMBERS - LDN
   ********************************************************************/
  0x40,   0x41,   0x42,   0x43,   0x44,   0x45,   0x46,   0x47,
  0x48,   0x49,   0x4A,   0x4B,   0x4C,   0x4D,   0x4E,   0x4F,   
  0x4F,   0x4F,   0x4F,   0x4F,   0x00,   0x00,

  /********************************************************************
   * Field LAST MTC NUMBERS - LRN
   ********************************************************************/
  0x50,   0x51,   0x52,   0x53,   0x54,   0x55,   0x56,   0x57,
  0x58,   0x59,   0x5A,   0x5B,   0x5C,   0x5D,   0x5E,   0x5F,
  0x5F,   0x5F,   0x5F,   0x5F,   0x5F,   0x00,   0x00,

  /********************************************************************
   * Field LAST MTC MISSED NUMBERS - LMN
   ********************************************************************/
  0x40,   0x41,   0x42,   0x43,   0x44,   0x45,   0x46,   0x47,
  0x48,   0x49,   0x4A,   0x4B,   0x4C,   0x4D,   0x4E,   0x4F,   
  0x4F,   0x4F,   0x4F,   0x4F,   0x00,

  /********************************************************************
   * Field USER PERSONAL NUMBERS - UPN
   ********************************************************************/
  0x60,   0x61,   0x62,   0x63,   0x64,   0x65,   0x66,   0x67,
  0x68,   0x69,   0x6A,   0x6B,   0x6C,   0x6D,   0x6E,   0x6F,   
  0x6F,   0x6F,   0x6F,   0x6F,   0x6F,   0x6F,   0x6F,   0x6F,

  /********************************************************************
   * Field MAILBOX NUMBERS - MBN
   ********************************************************************/
  // Record 1
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,
  0x00,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,

  // Record 2
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,
  0x00,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,

  // Record 3
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,
  0x00,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,

  // Record 4
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,
  0x00,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,

  /********************************************************************
   * Field VOICE MAIL NUMBER - VMN                                               
   ********************************************************************/
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   
  0xFF,   0x81,   

  /********************************************************************
   * Field CALL TIMER - CTIM
   ********************************************************************/
  0x70,   0x71,   0x72,   0x73,   0x74,   0x75,   0x76,   0x77,
  0x78,   0x79,   0x7A,   0x7B,   0x7C,   0x7D,   0x7E,   0x7F,
  0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,
  0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,
  0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,
  0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,   0x7F,

  /********************************************************************
   * Field CALL COUNTER - CCNT
   ********************************************************************/
  0x80,   0x81,   0x82,   0x83,   0x84,   0x85,   0x86,   0x87,
  0x88,   0x89,   0x8A,   0x8B,   0x8C,   0x8D,   0x8E,   0x8F,   
  0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,
  0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,
  0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,
  0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,   0x8F,
  0x8F,   0x8F,   0x8F,   0x8F,   
        
  /********************************************************************
   * Field EMERGENCY CALL CODES - ECC
   ********************************************************************/
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,

  /********************************************************************
   * Field ORGANIZER AND ALARM - ORG
   ********************************************************************/
  0xA0,   0xA1,   0xA2,   0xA3,   0xA4,   0xA5,   0xA6,   0xA7,
  0xA8,   0xA9,   0xAA,   0xAB,   0xAC,   0xAD,   0xAE,   0xAF,
  0xAF,   0xAF,   0xAF,   0xAF,   0xAF,   0x00,   0x00,

  /********************************************************************
   * Field CAPABILITY AND CONFIGURATION PARAMETERS - CCP
   ********************************************************************/
  0xB0,   0xB1,   0xB2,   0xB3,   0xB4,   0xB5,   0xB6,
        
  /********************************************************************
   * Field EXTENSION 1 - EXT1
   ********************************************************************/
  0xC0,   0xC1,   0xC2,   0xC3,   0xC4,   0xC5,   0xC6,   0xC7,
  0xC8,   0xC9,   0xCA,   0xCB,   0xCC,   

  /********************************************************************
   * Field SIM LOCK - SIMLCK
   ********************************************************************/
  0x00,   0x00,   0xD2,   0xD3,
  0x11,   0x11,   0x11,   0x11,   0x11,   0x11,   0x11,   0x11,    
  0x21,   0x43,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   
  0x0F,   
  0x22,   0x24,   0x24,   0x24,   0x44,   0x15,   0x12,   0x45,
  0x22,   0x22,   0x22,   0x22,   0x44,   0x15,   0x66,   
  0xFF,   0xFF,
       
  /********************************************************************
   * Field MAINTENANCE INFORMATION - MAIN
   ********************************************************************/
  0xE0,   0xE1,   0xE2,   0xE3,   0xE4,   0xE5,   0xE6,   0xE7,

  /********************************************************************
   * Field SPECIAL FUNCTION KEY - SFK
   ********************************************************************/
  0xE8,   0xE9,   0xEA,   0xEB,   0xEC,   0xED,   0xEE,   0xEF,

  /********************************************************************
   * Field FAULT CONDITIONS - FAULT
   ********************************************************************/
  0xF0,   0xF1,   0xF2,   0xF3,   0xF4,   0xF5,   0xF6,   0xF7,

  /********************************************************************
   * Field DEBUG INFORMATION - DEBUG
   ********************************************************************/
  0xF8,   0xF9,   0xFA,   0xFB,   0xFC,   0xFD,   0xFE,   0xFF,

  /********************************************************************
   * Field POWER MANAGEMENT - POWER
   ********************************************************************/
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,

  /********************************************************************
   * Field KEYBOARD MAPPING - KEYB
   ********************************************************************/
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,

  /********************************************************************
   * Field RADIO PARAMETERS - RADIO
   ********************************************************************/
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,

  /********************************************************************
   * Field MANUFACTURER - CGMI
   ********************************************************************/
#if (OP_WCP == 1)
  'T',    'e',    'x',    'a',    's',    ' ',    'I',    'n',
  's',    't',    'r',    'u',    'm',    'e',    'n',    't',
  's',    0xFF,   0xFF,   0xFF, 
#else
  '<',    'm',    'a',    'n',    'u',    'f',    'a',    'c',
  't',    'u',    'r',    'e',    'r',    '>',    0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF, 
#endif
  /********************************************************************
   * Field IDENTIFICATION INFORMATION - INF0
   ********************************************************************/
  // Record 1
  '<',    'm',    'a',    'n',    'u',    'f',    'a',    'c',
  't',    'u',    'r',    'e',    'r',    '1',    '>',    0xFF,
  0xFF,   0xFF,   0xFF,   0xFF, 

  // Record 2
  '<',    'm',    'a',    'n',    'u',    'f',    'a',    'c',
  't',    'u',    'r',    'e',    'r',    '2',    '>',    0xFF,
  0xFF,   0xFF,   0xFF,   0xFF, 

  /********************************************************************
   * Field MODEL - CGMM
   ********************************************************************/
#if (OP_WCP == 1)
  'O',    'M',    'A',    'P',    '7',    '1',    '0',    0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,
#else
  '<',    'm',    'o',    'd',    'e',    'l',    '>',    0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,
#endif

  /********************************************************************
   * Field REVISION - CGMR
   ********************************************************************/
#if (OP_WCP == 1)
  '1',    '4',    '0',     '.',    '5',    '4',    '2',    '.',
  '8',    '2',    '7',    0xFF,    0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,
#else
  '<',    'r',    'e',    'v',    'i',    's',    'i',    'o',
  'n',    '>',    0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,
#endif
  /********************************************************************
   * Field PRODUCT SERIAL NUMBER - CGSN
   ********************************************************************/
  '<',    's',    'e',    'r',    'i',    'a',    'l',    ' ',
  'n',    'u',    'm',    'b',    'e',    'r',    '>',    0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,

  /********************************************************************
   * Field SMS PROFILE - SMSPRFL
   ********************************************************************/
  // Record 1
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   

  // Record 2
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   

  /********************************************************************
   * Field PLMN IDENTIFIER - PLMN
   ********************************************************************/
  // Record 1
  0xFF,   0xFF,   0xFF,   0xFF,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,     

  // Record 2
  0xFF,   0xFF,   0xFF,   0xFF,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,

  /********************************************************************
   * Field BCCH INFORMATION - BCCHINF
   ********************************************************************/
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

  /********************************************************************
   * Field ALTERNATIVE LINE SERVICE - ALS
   ********************************************************************/
  0x00,   0x00,

  /********************************************************************
   * Field LOCATION INFORMATION (GPRS) - LOCGPRS
   ********************************************************************/
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0xFF,   0xFF,   0xFF,   0xFE,   0xFF,   0xFF,

  /********************************************************************
   * Field CIPHERING KEY (GPRS) - KEYGPRS
   ********************************************************************/
  0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
  0x07,

  /********************************************************************
   * Field IMSI (GPRS) - IMSIGPRS
   ********************************************************************/
  0x00,
  0xF1,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF
        
}; 

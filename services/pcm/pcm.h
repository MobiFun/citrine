/*
+--------------------------------------------------------------------+
| PROJECT: GSM-MFW (?)                  $Workfile:: pcm.h           $|
| $Author: mmj                          $Revision::  1              $|
| CREATED: 07.10.98                     $Modtime:: 15.12.99 10:59   $|
| STATE  : code                                                      |
+--------------------------------------------------------------------+

	PURPOSE :  Types definitions for the permanent memory configuration
*/

#ifndef DEF_PCM
#define DEF_PCM

#include "../../riviera/rv/general.h"

#define LEN_MSCAP    (NR_EF_MSCAP    * SIZE_EF_MSCAP)    + 2
#define LEN_IMEI     (NR_EF_IMEI     * SIZE_EF_IMEI)     + 2
#define LEN_IMSI     (NR_EF_IMSI     * SIZE_EF_IMSI)     + 2
#define LEN_SMS      (NR_EF_SMS      * SIZE_EF_SMS)      + 2
#define LEN_CLASS2   (NR_EF_CLASS2   * SIZE_EF_CLASS2)   + 2
#define LEN_CLASS3   (NR_EF_CLASS3   * SIZE_EF_CLASS3)   + 2
#define LEN_MSSUP    (NR_EF_MSSUP    * SIZE_EF_MSSUP)    + 2
#define LEN_CLNG     (NR_EF_CLNG     * SIZE_EF_CLNG)     + 2
#define LEN_MSSET    (NR_EF_MSSET    * SIZE_EF_MSSET)    + 2
#define LEN_HZCACHE  (NR_EF_HZCACHE  * SIZE_EF_HZCACHE)  + 2
#define LEN_LDN      (NR_EF_LDN      * SIZE_EF_LDN)      + 2
#define LEN_LRN      (NR_EF_LRN      * SIZE_EF_LRN)      + 2
#define LEN_LMN      (NR_EF_LMN      * SIZE_EF_LMN)      + 2
#define LEN_UPN      (NR_EF_UPN      * SIZE_EF_UPN)      + 2
#define LEN_MBN      (NR_EF_MBN      * SIZE_EF_MBN)      + 2
#define LEN_VMN      (NR_EF_VMN      * SIZE_EF_VMN)      + 2
#define LEN_CTIM     (NR_EF_CTIM     * SIZE_EF_CTIM)     + 2
#define LEN_CCNT     (NR_EF_CCNT     * SIZE_EF_CCNT)     + 2
#define LEN_ECC      (NR_EF_ECC      * SIZE_EF_ECC)      + 2
#define LEN_ORG      (NR_EF_ORG      * SIZE_EF_ORG)      + 2
#define LEN_CCP      (NR_EF_CCP      * SIZE_EF_CCP)      + 2
#define LEN_EXT1     (NR_EF_EXT1     * SIZE_EF_EXT1)     + 2
#define LEN_SIMLCK   (NR_EF_SIMLCK   * SIZE_EF_SIMLCK)   + 2
#define LEN_MAIN     (NR_EF_MAIN     * SIZE_EF_MAIN)     + 2
#define LEN_SFK      (NR_EF_SFK      * SIZE_EF_SFK)      + 2
#define LEN_FAULT    (NR_EF_FAULT    * SIZE_EF_FAULT)    + 2
#define LEN_DEBUG    (NR_EF_DEBUG    * SIZE_EF_DEBUG)    + 2
#define LEN_POWER    (NR_EF_POWER    * SIZE_EF_POWER)    + 2
#define LEN_KEYB     (NR_EF_KEYB     * SIZE_EF_KEYB)     + 2
#define LEN_RADIO    (NR_EF_RADIO    * SIZE_EF_RADIO)    + 2
#define LEN_CGMI     (NR_EF_CGMI     * SIZE_EF_CGMI)     + 2
#define LEN_INF0     (NR_EF_INF0     * SIZE_EF_INF0)     + 2
#define LEN_CGMM     (NR_EF_CGMM     * SIZE_EF_CGMM)     + 2
#define LEN_CGMR     (NR_EF_CGMR     * SIZE_EF_CGMR)     + 2
#define LEN_CGSN     (NR_EF_CGSN     * SIZE_EF_CGSN)     + 2
#define LEN_SMSPRFL  (NR_EF_SMSPRFL  * SIZE_EF_SMSPRFL)  + 2
#define LEN_PLMN     (NR_EF_PLMN     * SIZE_EF_PLMN)     + 2
#define LEN_BCCHINFO (NR_EF_BCCHINFO * SIZE_EF_BCCHINFO) + 2
#define LEN_ALS      (NR_EF_ALS      * SIZE_EF_ALS)      + 2
#define LEN_LOCGPRS  (NR_EF_LOCGPRS  * SIZE_EF_LOCGPRS)  + 2
#define LEN_KCGPRS   (NR_EF_KCGPRS   * SIZE_EF_KCGPRS)   + 2
#define LEN_IMSIGPRS (NR_EF_IMSIGPRS * SIZE_EF_IMSIGPRS) + 2

#define PCM_SIZE (LEN_MSCAP    + LEN_IMEI   + LEN_IMSI    + LEN_SMS    + \
                  LEN_CLASS2   + LEN_CLASS3 + LEN_MSSUP   + LEN_CLNG   + \
                  LEN_MSSET    + LEN_HZCACHE+ LEN_LDN    + LEN_LRN     + LEN_LMN    + \
                  LEN_UPN      + LEN_MBN    + LEN_VMN     + LEN_CTIM   + \
                  LEN_CCNT     + LEN_ECC    + LEN_ORG     + LEN_CCP    + \
                  LEN_EXT1     + LEN_SIMLCK + LEN_MAIN    + LEN_SFK    + \
                  LEN_FAULT    + LEN_DEBUG  + LEN_POWER   + LEN_KEYB   + \
                  LEN_RADIO    + LEN_CGMI   + LEN_INF0    + LEN_CGMM   + \
                  LEN_CGMR     + LEN_CGSN   + LEN_SMSPRFL + LEN_PLMN   + \
                  LEN_BCCHINFO + LEN_ALS    + LEN_LOCGPRS + LEN_KCGPRS + \
                  LEN_IMSIGPRS)
                  
/********************************************************************
 *
 * Parameters written in flash memory
 *
 ********************************************************************/

#define PCM_SERIAL_CONFIG (0)

/********************************************************************
 *
 * Access functions to bit fields
 *
 ********************************************************************/

#define EXTERN extern 

#ifndef __BF_FUNCCALL__
  #define FldGet(f,n) ((((ULONG)f)&((ULONG)n##m))>>n##s)
  #define FldSet(f,n,v) ((((ULONG)f)&(~((ULONG)n##m)))|(((ULONG)v)<<n##s))
#else
  ULONG FldGet (ULONG f, ULONG m, int s)
  { return ((f&m)>>s); }
  ULONG FldSet (ULONG f, ULONG m, int s, ULONG v)
  { return ((f&(~m))|((v<<s)&m)); }
#endif

/********************************************************************
 *
 * Field Identitifer
 *
 ********************************************************************/

#define EF_MSCAP_ID     "MSCAP"       /* Mobile Capabilities        */
#define EF_IMEI_ID      "IMEI"        /* Int. Mobile Equipment Id.  */
#define EF_IMSI_ID      "IMSI"        /* Int. Mobile Subscriber Id. */
#define EF_SMS_ID       "SMS"         /* Short Message Service      */
#define EF_CLASS2_ID    "CLASS2"      /* MS Classmark 2             */
#define EF_CLASS3_ID    "CLASS3"      /* MS Classmark 3             */
#define EF_MSSUP_ID     "MSSUP"       /* Mobile Setup               */
#define EF_CLNG_ID      "CLNG"        /* Current Language           */
#define EF_MSSET_ID     "MSSET"       /* MS Settings                */
#define EF_HZCACHE_ID   "HZCACHE"     /* Homezone cache             */
#define EF_LDN_ID       "LDN"         /* Last MOC Numbers           */
#define EF_LRN_ID       "LRN"         /* Last MTC Numbers           */
#define EF_LMN_ID       "LMN"         /* Last MTC Missed Numbers    */
#define EF_UPN_ID       "UPN"         /* User Personal Numbers      */
#define EF_MBN_ID       "MBN"         /* Mailbox Numbers            */
#define EF_VMN_ID       "VMN"         /* Voice Mail Number          */
#define EF_CTIM_ID      "CTIM"        /* Call Timer                 */
#define EF_CCNT_ID      "CCNT"        /* Call Counter               */
#define EF_ECC_ID       "ECC"         /* Emergency Call Codes       */
#define EF_ORG_ID       "ORG"         /* Organizer and Alarm        */
#define EF_CCP_ID       "CCP"         /* Cap and Config Params      */
#define EF_EXT1_ID      "EXT1"        /* Extension 1                */
#define EF_SIMLCK_ID    "SIMLCK"      /* SIM Lock                   */
#define EF_SIMLCKEXT_ID "SIMLCKEXT"   /* Extended SIM Lock          */
#define EF_MAIN_ID      "MAIN"        /* Maintenance Information    */
#define EF_SFK_ID       "SFK"         /* Special Function Keys      */
#define EF_FAULT_ID     "FAULT"       /* Fault Conditions           */
#define EF_DEBUG_ID     "DEBUG"       /* Debug Information          */
#define EF_POWER_ID     "POWER"       /* Power Management           */
#define EF_KEYB_ID      "KEYB"        /* Keyboard Mapping           */
#define EF_RADIO_ID     "RADIO"       /* Radio Parameters           */
#define EF_CGMI_ID      "CGMI"        /* Manufacturer               */
#define EF_INF0_ID      "INF0"        /* Identification Information */
#define EF_CGMM_ID      "CGMM"        /* Model                      */
#define EF_CGMR_ID      "CGMR"        /* Revision                   */
#define EF_CGSN_ID      "CGSN"        /* Product Serial Number      */
#define EF_SMSPRFL_ID   "SMSPRFL"     /* SMS Profile                */
#define EF_PLMN_ID      "PLMN"        /* PLMN Identifier            */
#define EF_BCCHINFO_ID  "BCCHINF"     /* BCCH Information           */
#define EF_ALS_ID       "ALS"         /* Alternative Line Service   */
#define EF_LOCGPRS_ID   "LOCGPRS"     /* Location Info. (GPRS)      */
#define EF_KCGPRS_ID    "KCGPRS"      /* Ciphering Key (GPRS)       */
#define EF_IMSIGPRS_ID  "IMSIGPRS"    /* IMSI (GPRS)                */

/********************************************************************
 *
 * Return Values
 *
 ********************************************************************/

#define drv_Return_Type      UBYTE
#define PCM_OK               0
#define PCM_INITIALIZED      1
#define PCM_INVALID_FILE     2
#define PCM_INVALID_SIZE     3
#define PCM_INVALID_CKSM     4
#define PCM_INVALID_RECORD   5
#define PCM_NVRAM_ACCS_FAIL  6
#define PCM_ERASE_ERROR	     7
#define PCM_WRITE_ERROR	     8

/*
 * Field Info Structure
 */
typedef struct pcm_FileInfo_Type
{
  UBYTE  *FileLocation;
  USHORT FileSize;
  UBYTE  Version;
} pcm_FileInfo_Type;

typedef struct
{
    char   *identifier;
    USHORT start;
    USHORT length;
    USHORT records;
} T_PCM_DESCRIPTION;


/********************************************************************
 *
 * Field MOBILE CAPABILITIES - MSCAP
 *
 ********************************************************************/

typedef struct pcm_EFmscap_Type
{
    UBYTE chnMode;  /* channel modes     */
    UBYTE datCap1;  /* data capabilities */
    UBYTE datCap2;  /* data capabilities */
    UBYTE featLst1; /* feature list      */
    UBYTE featLst2; /* feature list      */
    UBYTE featLst3; /* feature list      */
} EF_MSCAP;
                            
#define SIZE_EF_MSCAP 6            
#define NR_EF_MSCAP   1            

/*
 * chnMode bits             
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |L1    |Tm    |afs   |ahs   |spV3  |efrV2 |hr    |spV1  |
 *   -------------------------------------------------------
 */
#define spchSupV1   0x00000001,0        /* speech support (vers. 1) */
#define spchSupV1m  0x00000001
#define spchSupV1s  0
#define hrSup       0x00000002,1        /* HR support               */
#define hrSupm      0x00000002
#define hrSups      1
#define HR_EFRSup   0x00000006,1        /* HR and EFR support       */
#define HR_EFRSupm  0x00000006
#define HR_EFRSups  1
#define EFRSupV2    0x00000004,2        /* enhanced FR support (v2) */
#define EFRSupV2m   0x00000004
#define EFRSupV2s   2
#define EFRSupV3    0x00000008,3        /* speech support (vers 3)  */
#define EFRSupV3m   0x00000008
#define EFRSupV3s   3
#define VocSup      0x0000000F,0        /* vocoder support          */
#define VocSupm     0x0000000F
#define VocSups     0
#define AHS         0x00000010,4        /* adaptive multirate half rate speech */
#define AHSm        0x00000010
#define AHSs        4
#define AFS         0x00000020,5        /* adaptive multirate full rate speech */
#define AFSm        0x00000020
#define AFSs        5
#define TestMobile  0x00000040,6        /* Test Mobile              */
#define TestMobilem 0x00000040
#define TestMobiles 6
#define L1Ver       0x00000080,7        /* Layer 1 Version          */
#define L1Verm      0x00000080
#define L1Vers      7
 
/* 
 * datCap1 bits
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |14.4  |tfax  |ntfax |tsyn  |syn   |asyn  |rlp   |ds    |
 *   -------------------------------------------------------
 */
#define datSup      0x00000001,0        /* Data support             */
#define datSupm     0x00000001
#define datSups     0
#define RLPSup      0x00000002,1        /* RLP data (NT Async)      */
#define RLPSupm     0x00000002
#define RLPSups     1
#define AsySup      0x00000004,2        /* T Async data support     */
#define AsySupm     0x00000004
#define AsySups     2
#define NTSynSup    0x00000008,3        /* NT Sync data support     */
#define NTSynSupm   0x00000008
#define NTSynSups   3
#define TSynSup     0x00000010,4        /* T Sync data support      */
#define TSynSupm    0x00000010
#define TSynSups    4
#define NTFaxSup    0x00000020,5        /* NT Fax support           */
#define NTFaxSupm   0x00000020
#define NTFaxSups   5
#define TFaxSup     0x00000040,6        /* T Fax support            */
#define TFaxSupm    0x00000040
#define TFaxSups    6
#define Dr14_4Sup   0x00000080,7        /* Data rate 14.4 support   */
#define Dr14_4Supm  0x00000080
#define Dr14_4Sups  7

/* 
 * datCap2 bits
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |DHR   |      |      |NAS   |TPD   |NTPD  |TP    |NTP   |
 *   -------------------------------------------------------
 */
#define NTPackSup   0x00000001,0        /* NT Packet Service        */
#define NTPackSupm  0x00000001
#define NTPackSups  0
#define TPackSup    0x00000002,1        /* T Packet Service         */
#define TPackSupm   0x00000002
#define TPackSups   1
#define NTPadSup    0x00000004,2        /* NT PAD Access Service    */
#define NTPadSupm   0x00000004
#define NTPadSups   2
#define TPadSup     0x00000008,3        /* T PAD Access Service     */
#define TPadSupm    0x00000008
#define TPadSups    3
#define NAltSrvSup  0x00000010,4        /* No Alternate Services    */
#define NAltSrvSupm 0x00000010
#define NAltSrvSups 4
#define DHRSup      0x00000080,7        /* Data Halfrate support    */
#define DHRSupm     0x00000080
#define DHRSups     7


/********************************************************************
 *
 * Field INTERNATIONAL MOBILE EQUIPMENT ID - IMEI
 *
 ********************************************************************/

typedef struct pcm_EFimei_Type
{
    UBYTE tac1;
    UBYTE tac2;
    UBYTE tac3;
    UBYTE fac;
    UBYTE snr1;
    UBYTE snr2;
    UBYTE snr3; 
    UBYTE svn;
} EF_IMEI;

#define SIZE_EF_IMEI 8
#define NR_EF_IMEI   1


/********************************************************************
 *
 * Field INTERNATIONAL MOBILE SUBSCRIBER ID - IMSI
 *
 ********************************************************************/

typedef struct pcm_EFimsi_Type
{
    UBYTE len;	   /* length IMSI */
    UBYTE IMSI[8]; /* IMSI        */
} EF_IMSI;

#define SIZE_EF_IMSI 9
#define NR_EF_IMSI   1


/********************************************************************
 *
 * Field SHORT MESSAGE SERVICE - SMS
 *
 ********************************************************************/

typedef struct pcm_EFsms_Type
{
    UBYTE stat;     /* status    */
    UBYTE rmd[175]; /* remainder */
} EF_SMS;  

#define SIZE_EF_SMS 176
#define NR_EF_SMS     1

/* 
 * stat bits
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |reserved                          |status              |
 *   -------------------------------------------------------
 */


/********************************************************************
 *
 * Field MOBILE STATION CLASSMARK 2 - CLASS2
 *
 ********************************************************************/

typedef struct pcm_EFclass2_Type
{
    UBYTE byte1; /* class2 byte 1 */
    UBYTE byte2; /* class2 byte 2 */
    UBYTE byte3; /* class2 byte 3 */
} EF_CLASS2; 
          
#define SIZE_EF_CLASS2 3
#define NR_EF_CLASS2   1

/* 
 * byte1 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |-     |rev          |es    |a5/1  |rfpwr               |
 *   -------------------------------------------------------
 */
#define rfPwrCap    0x00000007,0        /* rf power capability          */
#define rfPwrCapm   0x00000007
#define rfPwrCaps   0
#define a51         0x00000008,3        /* rf power capability          */
#define a51m        0x00000008
#define a51s        3
#define esInd       0x00000010,4        /* ES indicator                 */
#define esIndm      0x00000010
#define esInds      4
#define revLev      0x00000060,5        /* revision level               */
#define revLevm     0x00000060
#define revLevs     5

/* 
 * byte2 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |-     |ps    |ss           |sm    |reserved     |frq   |
 *   -------------------------------------------------------
 */
#define freqCap     0x00000001,0        /* frequency capability         */
#define freqCapm    0x00000001
#define freqCaps    0
#define smCap       0x00000008,3        /* SM capability                */
#define smCapm      0x00000008
#define smCaps      3
#define ssScrInd    0x00000030,4        /* SS Screen Indicator          */
#define ssScrIndm   0x00000030
#define ssScrInds   4
#define psCap       0x00000040,6        /* PS capability                */
#define psCapm      0x00000040
#define psCaps      6

/* 
 * byte3 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |CM3   |reserved                          |A5/3  |A5/2  |
 *   -------------------------------------------------------
 */
#define a52         0x00000001,0        /* encryption algorithm A5/2    */
#define a52m        0x00000001
#define a52s        0
#define a53         0x00000002,1        /* encryption algorithm A5/3    */
#define a53m        0x00000002
#define a53s        1
#define cm3         0x000000f8,7        /* CM3                          */
#define cm3m        0x000000f8
#define cm3s        7


/********************************************************************
 *
 * Field MOBILE STATION CLASSMARK 3 - CLASS3
 *
 ********************************************************************/

typedef struct pcm_EFclass3_Type
{
    UBYTE byte1; /* class3 byte 1 */
    UBYTE byte2; /* class3 byte 2 */
    UBYTE byte3; /* class3 byte 3 */
} EF_CLASS3;

#define SIZE_EF_CLASS3 3
#define NR_EF_CLASS3   1

/* 
 * byte1 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |-     |bnd3  |bnd2  |bnd1  |a5/7  |a5/6  |a5/5  |a5/4  |
 *   -------------------------------------------------------
 */
#define a54         0x00000001,0        /* encryption algorithm A5/4    */
#define a54m        0x00000001
#define a54s        0
#define a55         0x00000002,1        /* encryption algorithm A5/5    */
#define a55m        0x00000002
#define a55s        1
#define a56         0x00000004,2        /* encryption algorithm A5/6    */
#define a56m        0x00000004
#define a56s        2
#define a57         0x00000008,3        /* encryption algorithm A5/7    */
#define a57m        0x00000008
#define a57s        3
#define bnd1        0x00000010,4        /* Band 1                       */
#define bnd1m       0x00000010
#define bnd1s       4
#define bnd2        0x00000020,5        /* Band 2                       */
#define bnd2m       0x00000020
#define bnd2s       5
#define bnd3        0x00000040,6        /* Band 3                       */
#define bnd3m       0x00000040
#define bnd3s       6

/* 
 * byte2 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |rfcap2                     |rfcap1                     |
 *   -------------------------------------------------------
 */
#define rfCap1      0x0000000f,0        /* associated RF capability 1   */
#define rfCap1m     0x0000000f
#define rfCap1s     0
#define rfCap2      0x000000f0,4        /* associated RF capability 2   */
#define rfCap2m     0x000000f0
#define rfCap2s     4

/* 
 * byte3 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |0     |0     |0     |xm    |0     |0     |0     |0     |
 *   -------------------------------------------------------
 */


/********************************************************************
 *
 * Field MOBILE SETUP - MSSUP
 *
 ********************************************************************/

typedef struct pcm_EFmssup_Type
{
    UBYTE lng1;  /* language 1      */
    UBYTE lng2;  /* language 2      */
    UBYTE lng3;  /* language 3      */
    UBYTE feat1; /* features byte 1 */
    UBYTE feat2; /* features byte 2 */
} EF_MSSUP;
       
#define SIZE_EF_MSSUP 5
#define NR_EF_MSSUP   1

/* 
 * lng1 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |por   |swe   |spa   |ita   |dut   |ger   |fre   |eng   |
 *   -------------------------------------------------------
 */
#define eng         0x00000001,0        /* English                      */
#define engm        0x00000001
#define engs        0
#define fre         0x00000002,1        /* French                       */
#define frem        0x00000002
#define fres        1
#define ger         0x00000004,2        /* German                       */
#define germ        0x00000004
#define gers        2
#define dut         0x00000008,3        /* Dutch                        */
#define dutm        0x00000008
#define duts        3
#define ita         0x00000010,4        /* Italian                      */
#define itam        0x00000010
#define itas        4
#define spa         0x00000020,5        /* Spanish                      */
#define spam        0x00000020
#define spas        5
#define swe         0x00000040,6        /* Swedish                      */
#define swem        0x00000040
#define swes        6
#define por         0x00000080,7        /* Portuguese                   */
#define porm        0x00000080
#define pors        7

/* 
 * lng2 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |rus   |pol   |slo   |hun   |tur   |gre   |nor   |fin   |
 *   -------------------------------------------------------
 */
#define fin         0x00000001,0        /* Finnish                      */
#define finm        0x00000001
#define fins        0
#define nor         0x00000002,1        /* Norwegian                    */
#define norm        0x00000002
#define nors        1
#define gre         0x00000004,2        /* Greek                        */
#define grem        0x00000004
#define gres        2
#define tur         0x00000008,3        /* Turkish                      */
#define turm        0x00000008
#define turs        3
#define hun         0x00000010,4        /* Hungarian                    */
#define hunm        0x00000010
#define huns        4
#define slo         0x00000020,5        /* Slovenian                    */
#define slom        0x00000020
#define slos        5
#define pol         0x00000040,6        /* Polish                       */
#define polm        0x00000040
#define pols        6
#define rus         0x00000080,7        /* Russian                      */
#define rusm        0x00000080
#define russ        7

/* 
 * lng3 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |-     |ara   |tai   |man   |can   |chi   |cze   |ind   |
 *   -------------------------------------------------------
 */
#define ind         0x00000001,0        /* Indonesian                   */
#define indm        0x00000001
#define inds        0
#define cze         0x00000002,1        /* Czech                        */
#define czem        0x00000002
#define czes        1
#define chi         0x00000004,2        /* Chinese                      */
#define chim        0x00000004
#define chis        2
#define can         0x00000008,3        /* Cantonese                    */
#define canm        0x00000008
#define cans        3
#define man         0x00000010,4        /* Mandarin                     */
#define manm        0x00000010
#define mans        4
#define tai         0x00000020,5        /* Taiwanese                    */
#define taim        0x00000020
#define tais        5
#define ara         0x00000040,6        /* Arabic                       */
#define aram        0x00000040
#define aras        6

/* 
 * feat1 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |stk   |irda  |etc   |ussd  |cb    |cf    |dtmf  |aoc   |
 *   -------------------------------------------------------
 */
#define AoC         0x00000001,0        /* Advice of Charge             */
#define AoCm        0x00000001
#define AoCs        0
#define DTMF        0x00000002,1        /* DTMF                         */
#define DTMFm       0x00000002
#define DTMFs       1
#define CF          0x00000004,2        /* Call Forwarding              */
#define CFm         0x00000004
#define CFs         2
#define CB          0x00000008,3        /* Call Barring                 */       
#define CBm         0x00000008
#define CBs         3
#define USSD        0x00000010,4        /* USSD                         */
#define USSDm       0x00000010
#define USSDs       4
#define ETC         0x00000020,5        /* ETC                          */
#define ETCm        0x00000020
#define ETCs        5
#define IRDA        0x00000040,6        /* IRDA                         */
#define IRDAm       0x00000040
#define IRDAs       6


/********************************************************************
 *
 * Field CURRENT LANGUAGE - CLNG
 *
 ********************************************************************/

#define SIZE_EF_CLNG_DATA 2 /* two-letter abbreviation of the language */

typedef struct pcm_EFclng_Type
{
    UBYTE data[SIZE_EF_CLNG_DATA]; /* abbreviation of the language */
} EF_CLNG;
       
#define SIZE_EF_CLNG (SIZE_EF_CLNG_DATA)
#define NR_EF_CLNG   1


/********************************************************************
 *
 * Field MOBILE STATION SETTTINGS - MSSET
 *
 ********************************************************************/

typedef struct pcm_EFmsset_Type
{
    UBYTE buzzer1;        /* buzzer byte 1        */
    UBYTE buzzer2;        /* buzzer byte 2        */
    UBYTE buzzer3;        /* buzzer byte 3        */
    UBYTE audio;          /* audio                */
    UBYTE misc;           /* miscellaneous        */
    UBYTE display;        /* display              */
    UBYTE language;       /* language             */
    UBYTE recent_ldn_ref; /* recent ldn reference */
    UBYTE recent_lrn_ref; /* recent lrn reference */
    UBYTE recent_upn_ref; /* recent upn reference */
} EF_MSSET;
   
#define SIZE_EF_MSSET 10                      
#define NR_EF_MSSET    1                      

/* 
 * buzzer1 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |vib          |callvol             |calltype            |
 *   -------------------------------------------------------
 */
#define calltype  0x00000007,0        /* ringer type incoming calls   */
#define calltypem 0x00000007
#define calltypes 0
#define callvol   0x00000038,3        /* ringer volume incoming calls */
#define callvolm  0x00000038
#define callvols  3
#define vib       0x000000c0,6        /* vibrator                     */
#define vibm      0x000000c0
#define vibs      6

/* 
 * buzzer2 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |reserved     |msgvol              |msgtype             |
 *   -------------------------------------------------------
 */
#define msgtype   0x00000007,0        /* ringer type messages         */
#define msgtypem  0x00000007
#define msgtypes  0
#define msgvol    0x00000038,3        /* ringer volume messages       */
#define msgvolm   0x00000038
#define msgvols   3

/* 
 * buzzer3 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |reserved                          |batw  |keytone      |
 *   -------------------------------------------------------
 */
#define keytone   0x00000003,0        /* key tone mode                */
#define keytonem  0x00000003
#define keytones  0
#define batw      0x00000004,2        /* low battery warning          */
#define batwm     0x00000004
#define batws     2

/* 
 * audio bits                        
 *   _________________________________________________________
 *  |8       |7     |6     |5     |4     |3     |2     |1     |
 *  |--------|------|------|------|------|------|------|------|
 *  |VoiceRec|Ext   |Outvol              |lnamp               |
 *   ---------------------------------------------------------
 */
#define lnamp     0x00000007,0        /* microphone amplification     */
#define lnampm    0x00000007
#define lnamps    0
#define outvol    0x00000038,3        /* output volume                */
#define outvolm   0x00000038
#define outvols   3
#define ext       0x00000040,6        /* external audio               */
#define extm      0x00000040
#define exts      6
#define voicerec  0x00000080,7        /* voice recording              */
#define voicerecm 0x00000080
#define voicerecs 7

/* 
 * miscellaneous bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |      |      |redial       |calinf|clip  |clir  |pmod  |
 *   -------------------------------------------------------
 */
#define pmod      0x00000001,0        /* PLMN selection mode          */
#define pmodm     0x00000001
#define pmods     0
#define clir      0x00000002,1        /* CLIR                         */
#define clirm     0x00000002
#define clirs     1
#define clip      0x00000004,2        /* CLIP                         */
#define clipm     0x00000004
#define clips     2
#define calinf    0x00000008,3        /* call information display     */
#define calinfm   0x00000008
#define calinfs   3
#define redial    0x00000030,4        /* redial mode                  */
#define redialm   0x00000030
#define redials   4

/* 
 * display bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |bckdr               |brgt         |ctrt                |
 *   -------------------------------------------------------
 */
#define ctrt      0x00000007,0        /* contrast                     */
#define ctrtm     0x00000007
#define ctrts     0
#define brgt      0x00000018,3        /* brightness                   */
#define brgtm     0x00000018
#define brgts     3
#define bckdr     0x000000E0,5        /* duration for back light      */
#define bckdrm    0x000000E0
#define bckdrs    5

/********************************************************************
 *
 * Field Homezone cache
 *
 ********************************************************************/

typedef struct pcm_EFhzcache_Type
{
    UBYTE 			cid[2];				
    UBYTE			zone;
} EF_HZCACHE;

#define SIZE_EF_HZCACHE 3
#define NR_EF_HZCACHE   5

/********************************************************************
 *
 * Field LAST MOC NUMBERS - LDN
 *
 ********************************************************************/

typedef struct pcm_EFldn_Type
{
    UBYTE calDrMsb;   /* call duration (MSB)                 */
    UBYTE calDrLsb;   /* call duration (LSB)                 */
    UBYTE year;       /* year  }                             */
    UBYTE month;      /* month } = date                      */
    UBYTE day;        /* day   }                             */
    UBYTE hour;       /* hour   }                            */
    UBYTE minute;     /* minute } = time                     */
    UBYTE second;     /* second }                            */
    UBYTE len;        /* length of BCD number                */
    UBYTE numTp;      /* TON and NPI                         */
    UBYTE dldNum[10]; /* called number                       */
    UBYTE ccp;        /* capability/configuration identifier */
    UBYTE ext1;       /* extension1 record identifier        */
} EF_LDN;

#define SIZE_EF_LDN 22
#define NR_EF_LDN    1

/* 
 * numTp bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |-     |ton                 |npi                        |
 *   -------------------------------------------------------
 */
#define numTp_npi   0x0000000F,0        /* numbering plan identification */
#define numTp_npim  0x0000000F
#define numTp_npis  0
#define numTp_ton   0x00000070,4        /* Type of number                */
#define numTp_tonm  0x00000070
#define numTp_tons  4


/********************************************************************
 *
 * Field LAST MTC NUMBERS - LRN
 *
 ********************************************************************/

typedef struct pcm_EFlrn_Type
{
    UBYTE calDrMsb;   /* call duration (MSB)                 */
    UBYTE calDrLsb;   /* call duration (LSB)                 */
    UBYTE year;       /* year  }                             */
    UBYTE month;      /* month } = date                      */
    UBYTE day;        /* day   }                             */
    UBYTE hour;       /* hour   }                            */
    UBYTE minute;     /* minute } = time                     */
    UBYTE second;     /* second }                            */
    UBYTE id;         /* identifier                          */
    UBYTE len;        /* length of BCD number                */
    UBYTE numTp;      /* TON and NPI - bitmap same as EF_LDN */
    UBYTE dldNum[10]; /* calling number                      */
    UBYTE ccp;        /* capability/configuration identifier */
    UBYTE ext1;       /* extension1 record identifier        */
} EF_LRN;

#define SIZE_EF_LRN 23
#define NR_EF_LRN    1

/* 
 * id bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |reserved                          |type                |
 *   -------------------------------------------------------
 */


/********************************************************************
 *
 * Field LAST MTC MISSED NUMBERS - LMN
 *
 ********************************************************************/

typedef struct pcm_EFlmn_Type
{
    UBYTE year;       /* year  }                             */
    UBYTE month;      /* month } = date                      */
    UBYTE day;        /* day   }                             */
    UBYTE hour;       /* hour   }                            */
    UBYTE minute;     /* minute } = time                     */
    UBYTE second;     /* second }                            */
    UBYTE id;         /* identifier - bitmap same as EF_LRN  */
    UBYTE len;        /* length of BCD number                */
    UBYTE numTp;      /* TON and NPI - bitmap same as EF_LDN */
    UBYTE dldNum[10]; /* calling number                      */
    UBYTE ccp;        /* capability/configuration identifier */
    UBYTE ext1;       /* extension1 record identifier        */
} EF_LMN;

#define SIZE_EF_LMN 21
#define NR_EF_LMN    1


/********************************************************************
 *
 * Field USER PERSONAL NUMBERS - UPN
 *
 ********************************************************************/

typedef struct pcm_EFupn_Type
{
    UBYTE alphId[10]; /* alpha identifier                    */
    UBYTE len;        /* length of BCD number                */
    UBYTE numTp;      /* TON and NPI - bitmap same as EF_LDN */
    UBYTE usrNum[10]; /* number                              */
    UBYTE ccp;        /* capability/configuration identifier */
    UBYTE ext1;       /* extension1 record identifier        */
} EF_UPN;

#define SIZE_EF_UPN 24
#define NR_EF_UPN    1


/********************************************************************
 *
 * Field MAILBOX NUMBERS - MBN
 *
 ********************************************************************/

typedef struct pcm_EFmbn_Type
{
    UBYTE alphId[10]; /* alpha identifier                    */
    UBYTE len;        /* length of BCD number                */
    UBYTE numTp;      /* TON and NPI - bitmap same as EF_LDN */
    UBYTE mbNum[10];  /* number                              */
} EF_MBN;

#define SIZE_EF_MBN 22
#define NR_EF_MBN    4


/********************************************************************
 *
 * Field VOICE MAIL NUMBER - VMN                                               
 *
 ********************************************************************/

/* 
 * note that with new 04.08 the called party bcd number of the CC
 * protocol can have up to 43 octets, 3 are used for other things 
 * than the BCD coded digits
 */

#define MAX_CALLED_PARTY_BCD_NO_OCTETS 40

typedef struct pcm_EFvmn_Type
{
    UBYTE vmNum[MAX_CALLED_PARTY_BCD_NO_OCTETS + 1]; /* voice mail number */
    UBYTE numTp;                   /* TON and NPI - bitmap same as EF_LDN */
} EF_VMN;

#define SIZE_EF_VMN (MAX_CALLED_PARTY_BCD_NO_OCTETS + 1 + 1)
#define NR_EF_VMN   1


/********************************************************************
 *
 * Field CALL TIMER - CTIM
 *
 ********************************************************************/

typedef struct pcm_EFctim_Type
{
    UBYTE moVcDrHm[4]; /* MO voice duration home PLMN */
    UBYTE mtVcDrHm[4]; /* MT voice duration home PLMN */
    UBYTE moDtDrHm[4]; /* MO data  duration home PLMN */
    UBYTE mtDtDrHm[4]; /* MT data  duration home PLMN */
    UBYTE moFxDrHm[4]; /* MO fax   duration home PLMN */
    UBYTE mtFxDrHm[4]; /* MT fax   duration home PLMN */
    UBYTE moVcDrRm[4]; /* MO voice duration roaming   */
    UBYTE mtVcDrRm[4]; /* MT voice duration roaming   */
    UBYTE moDtDrRm[4]; /* MO data  duration roaming   */
    UBYTE mtDtDrRm[4]; /* MT data  duration roaming   */
    UBYTE moFxDrRm[4]; /* MO fax   duration roaming   */
    UBYTE mtFxDrRm[4]; /* MT fax   duration roaming   */
} EF_CTIM;

#define SIZE_EF_CTIM 48
#define NR_EF_CTIM    1


/********************************************************************
 *
 * Field CALL COUNTER - CCNT
 *
 ********************************************************************/

typedef struct pcm_EFccnt_Type
{
    UBYTE Total[4];    /* total duration             */
    UBYTE moVcDrHm[4]; /* MO voice counter home PLMN */
    UBYTE mtVcDrHm[4]; /* MT voice counter home PLMN */
    UBYTE moDtDrHm[4]; /* MO data  counter home PLMN */
    UBYTE mtDtDrHm[4]; /* MT data  counter home PLMN */
    UBYTE moFxDrHm[4]; /* MO fax   counter home PLMN */
    UBYTE mtFxDrHm[4]; /* MT fax   counter home PLMN */
    UBYTE moVcDrRm[4]; /* MO voice counter roaming   */
    UBYTE mtVcDrRm[4]; /* MT voice counter roaming   */
    UBYTE moDtDrRm[4]; /* MO data  counter roaming   */
    UBYTE mtDtDrRm[4]; /* MT data  counter roaming   */
    UBYTE moFxDrRm[4]; /* MO fax   counter roaming   */
    UBYTE mtFxDrRm[4]; /* MT fax   counter roaming   */
} EF_CCNT;

#define SIZE_EF_CCNT 52
#define NR_EF_CCNT    1


/********************************************************************
 *
 * Field EMERGENCY CALL CODES - ECC
 *
 ********************************************************************/

typedef struct pcm_EFecc_Type
{
    UBYTE ecc1[3]; /* emergency call code 1 */
    UBYTE ecc2[3]; /* emergency call code 2 */
    UBYTE ecc3[3]; /* emergency call code 3 */
    UBYTE ecc4[3]; /* emergency call code 4 */
    UBYTE ecc5[3]; /* emergency call code 5 */
} EF_ECC;

#define SIZE_EF_ECC 15
#define NR_EF_ECC    1


/********************************************************************
 *
 * Field ORGANIZER AND ALARM - ORG
 *
 ********************************************************************/

typedef struct pcm_EForg_Type
{
    UBYTE date[6];     /* year, month, day, hour, minute, second */
    UBYTE alrm;        /* alarm                                  */
    UBYTE alphMem[16]; /* alpha memo                             */
} EF_ORG;
                  
#define SIZE_EF_ORG 23
#define NR_EF_ORG    1

/* 
 * alrm bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |reserved                   |type                |stat  |
 *   -------------------------------------------------------
 */
#define alrm_stat        0x00000001,0
#define alrm_statm       0x00000001
#define alrm_stats       0
#define alrm_type        0x0000000E,1
#define alrm_typem       0x0000000E
#define alrm_types       1


/********************************************************************
 *
 * Field CAPABILITY AND CONFIGURATION PARAMETERS - CCP
 *
 ********************************************************************/

typedef struct pcm_EFccp_Type
{
    UBYTE usrRate;   /* user rate          */
    UBYTE bearServ;  /* bearer service     */
    UBYTE conElem;   /* connection element */
    UBYTE stopBits;  /* stop bits          */
    UBYTE dataBits;  /* data bits          */
    UBYTE parity;    /* parity             */
    UBYTE flowCntrl; /* flow control       */
} EF_CCP;

#define SIZE_EF_CCP 7
#define NR_EF_CCP   1


/********************************************************************
 *
 * Field EXTENSION 1 - EXT1
 *
 ********************************************************************/

typedef struct pcm_EFext1_Type
{
    UBYTE recTp;      /* record type    */
    UBYTE extDat[11]; /* extension data */
    UBYTE id;         /* identifier     */
} EF_EXT1;

#define SIZE_EF_EXT1 13
#define NR_EF_EXT1    1

/* 
 * recTp bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |reserved                                 |type         |
 *   -------------------------------------------------------
 */


/********************************************************************
 *
 * Field SIM LOCK & EXTENDED SIM LOCK (use alternatively) - SIMLCK
 *
 ********************************************************************/

typedef struct pcm_EFsimlck_Type
{
    UBYTE locks1;   /* lock byte 1              */
    UBYTE locks2;   /* lock byte 2              */
    UBYTE cnt;      /* unlock attempt counter   */
    UBYTE maxcnt;   /* maximum attempt          */
    UBYTE PKey[8];  /* P control key            */
    UBYTE SPKey[8]; /* SP control key           */
    UBYTE NSKey[8]; /* NS control key           */
    UBYTE CKey[8];  /* C control key            */
    UBYTE NKey[8];  /* N control key            */
    UBYTE len_imsi; /* length of IMSI           */
    UBYTE imsi[15]; /* IMSI                     */
    UBYTE gidl1;    /* group identifier level 1 */
    UBYTE gidl2;    /* group identifier level 2 */
} EF_SIMLCK;

#define SIZE_EF_SIMLCK 62
#define NR_EF_SIMLCK    1


typedef struct pcm_EFsimlckext_Type
{
    UBYTE locks1;      /* lock byte 1              */
    UBYTE locks2;      /* lock byte 2              */
    UBYTE cnt;         /* unlock attempt counter   */
    UBYTE maxcnt;      /* maximum attempt          */
    UBYTE PKey[8];     /* P control key            */
    UBYTE SPKey[8];    /* SP control key           */
    UBYTE NSKey[8];    /* NS control key           */
    UBYTE CKey[8];     /* C control key            */
    UBYTE NKey[8];     /* N control key            */
    UBYTE len_p_imsi;  /* length of IMSI P-Lock    */
    UBYTE p_imsi[15];  /* IMSI P-Lock              */
    UBYTE len_sp_imsi; /* length of IMSI SP-Lock   */
    UBYTE sp_imsi[15]; /* IMSI SP-Lock             */
    UBYTE len_ns_imsi; /* length of IMSI NS-Lock   */
    UBYTE ns_imsi[15]; /* IMSI NS-Lock             */
    UBYTE len_c_imsi;  /* length of IMSI C-Lock    */
    UBYTE c_imsi[15];  /* IMSI C-Lock              */
    UBYTE len_n_imsi;  /* length of IMSI N-Lock    */
    UBYTE n_imsi[15];  /* IMSI N-Lock              */
    UBYTE len_u_imsi;  /* length of IMSI U-Lock    */
    UBYTE u_imsi[15];  /* IMSI U-Lock              */
    UBYTE gidl1;       /* group identifier level 1 */
    UBYTE gidl2;       /* group identifier level 2 */
} EF_SIMLCKEXT;

#define SIZE_EF_SIMLCKEXT 142
#define NR_EF_SIMLCKEXT     1

/* 
 * locks1 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |splock       |nslock       |nlock        |plock        |
 *   -------------------------------------------------------
 */
#define plock            0x00000003,0
#define plockm           0x00000003
#define plocks           0
#define nlock            0x0000000C,2
#define nlockm           0x0000000C
#define nlocks           2
#define nslock           0x00000030,4
#define nslockm          0x00000030
#define nslocks          4
#define spslock          0x000000C0,6
#define splockm          0x000000C0
#define splocks          6

/* 
 * locks2 bits                        
 *   _______________________________________________________
 *  |8     |7     |6     |5     |4     |3     |2     |1     |
 *  |------|------|------|------|------|------|------|------|
 *  |reserved                                 |clock        |
 *   -------------------------------------------------------
 */
#define clock            0x00000003,0
#define clockm           0x00000003
#define clocks           0


/********************************************************************
 *
 * Field MAINTENANCE INFORMATION - MAIN
 *
 ********************************************************************/

/*
 *  T.B.D.
 */

#define SIZE_EF_MAIN 8
#define NR_EF_MAIN   1


/********************************************************************
 *
 * Field SPECIAL FUNCTION KEY - SFK
 *
 ********************************************************************/

/*
 *  T.B.D.
 */

#define SIZE_EF_SFK 8
#define NR_EF_SFK   1


/********************************************************************
 *
 * Field FAULT CONDITIONS - FAULT
 *
 ********************************************************************/

/*
 *  T.B.D.
 */

#define SIZE_EF_FAULT 8
#define NR_EF_FAULT   1


/********************************************************************
 *
 * Field DEBUG INFORMATION - DEBUG
 *
 ********************************************************************/

/*
 *  T.B.D.
 */

#define SIZE_EF_DEBUG 8
#define NR_EF_DEBUG   1


/********************************************************************
 *
 * Field POWER MANAGEMENT - POWER
 *
 ********************************************************************/

/*
 *  T.B.D.
 */

#define SIZE_EF_POWER 8
#define NR_EF_POWER   1


/********************************************************************
 *
 * Field KEYBOARD MAPPING - KEYB
 *
 ********************************************************************/

typedef struct pcm_EFkbd_Type
{
    UBYTE logical_key [32]; /* logical key map */
    UBYTE raw_key [32];     /* raw key map     */
} EF_KBD;

#define SIZE_EF_KEYB 64
#define NR_EF_KEYB    1


/********************************************************************
 *
 * Field RADIO PARAMETERS - RADIO
 *
 ********************************************************************/

/*
 *  T.B.D.
 */

#define SIZE_EF_RADIO 8
#define NR_EF_RADIO   1


/********************************************************************
 *
 * Field MANUFACTURER - CGMI
 *
 ********************************************************************/

#define SIZE_EF_CGMI_DATA 20 /* value depends on manufacturer spec. */

typedef struct pcm_EFcgmi_Type
{
    UBYTE data[SIZE_EF_CGMI_DATA]; /* name of manufacturer */
} EF_CGMI;

#define SIZE_EF_CGMI (SIZE_EF_CGMI_DATA)
#define NR_EF_CGMI   1


/********************************************************************
 *
 * Field IDENTIFICATION INFORMATION - INF0
 *
 ********************************************************************/

#define SIZE_EF_INF0_DATA 20 /* value depends on manufacturer spec. */

typedef struct pcm_EFinf0_Type
{
    UBYTE data[SIZE_EF_INF0_DATA]; /* identification information */
} EF_INF0;

#define SIZE_EF_INF0 (SIZE_EF_INF0_DATA)
#define NR_EF_INF0   2


/********************************************************************
 *
 * Field MODEL - CGMM
 *
 ********************************************************************/

#define SIZE_EF_CGMM_DATA 20 /* value depends on manufacturer spec. */

typedef struct pcm_EFcgmm_Type
{
    UBYTE data[SIZE_EF_CGMM_DATA]; /* name of product */
} EF_CGMM;

#define SIZE_EF_CGMM (SIZE_EF_CGMM_DATA)
#define NR_EF_CGMM   1


/********************************************************************
 *
 * Field REVISION - CGMR
 *
 ********************************************************************/

#define SIZE_EF_CGMR_DATA 20 /* value depends on manufacturer spec. */

typedef struct pcm_EFcgmr_Type
{
    UBYTE data[SIZE_EF_CGMR_DATA]; /* version of product */
} EF_CGMR;

#define SIZE_EF_CGMR (SIZE_EF_CGMR_DATA)
#define NR_EF_CGMR   1


/********************************************************************
 *
 * Field PRODUCT SERIAL NUMBER - CGSN
 *
 ********************************************************************/

#define SIZE_EF_CGSN_DATA 20 /* value depends on manufacturer spec. */

typedef struct pcm_EFcgsn_Type
{
    UBYTE data[SIZE_EF_CGSN_DATA]; /* serial number of product */
} EF_CGSN;

#define SIZE_EF_CGSN (SIZE_EF_CGSN_DATA)
#define NR_EF_CGSN   1


/********************************************************************
 *
 * Field SMS PROFILE - SMSPRFL
 *
 ********************************************************************/

#define SIZE_EF_SMSPRFL_SCA   20
#define SIZE_EF_SMSPRFL_MIDS  40
#define SIZE_EF_SMSPRFL_DCSS  20
#define SIZE_EF_SMSPRFL_VPABS 14

typedef struct pcm_EFsmsprfl_Type
{
    UBYTE vldFlag;                          /* valid flag                       */
    UBYTE CSCAsca[SIZE_EF_SMSPRFL_SCA];     /* service center address           */  
    UBYTE CSCAlenSca;                       /* length of service center address */
    UBYTE CSCAton;                          /* type of number                   */
    UBYTE CSCAnpi;                          /* numbering plan identification    */
    UBYTE CSCBmode;                         /* cell broadcast mode              */
    UBYTE CSCBmids[SIZE_EF_SMSPRFL_MIDS];   /* message identifiers              */
    UBYTE CSCBdcss[SIZE_EF_SMSPRFL_DCSS];   /* data coding schemes              */
    UBYTE CSMPfo;                           /* first octet                      */
    UBYTE CSMPvprel;                        /* validity period relative         */
    UBYTE CSMPvpabs[SIZE_EF_SMSPRFL_VPABS]; /* validity period absolute         */
    UBYTE CSMPpid;                          /* protocol identifier              */
    UBYTE CSMPdcs;                          /* data coding scheme               */
} EF_SMSPRFL;

#define SIZE_EF_SMSPRFL (SIZE_EF_SMSPRFL_SCA   + \
                         SIZE_EF_SMSPRFL_MIDS  + \
                         SIZE_EF_SMSPRFL_DCSS  + \
                         SIZE_EF_SMSPRFL_VPABS + 9)
#define NR_EF_SMSPRFL   2

/* 
 * vldFlag values                        
 */
#define EF_SMSPRFL_VLD   0x00
#define EF_SMSPRFL_INVLD 0xFF


/********************************************************************
 *
 * Field PLMN IDENTIFIER - PLMN
 *
 ********************************************************************/

#define SIZE_EF_PLMN_MCC   2
#define SIZE_EF_PLMN_MNC   2
#define SIZE_EF_PLMN_LONG 20
#define SIZE_EF_PLMN_SHRT 10

typedef struct pcm_EFplmn_Type
{
    UBYTE mcc[SIZE_EF_PLMN_MCC];      /* mobile country code        */
    UBYTE mnc[SIZE_EF_PLMN_MNC];      /* mobile network code        */
    UBYTE lngNam[SIZE_EF_PLMN_LONG];  /* MT voice counter home PLMN */
    UBYTE shrtNam[SIZE_EF_PLMN_SHRT]; /* MO data counter home PLMN  */
} EF_PLMN;

#define SIZE_EF_PLMN (SIZE_EF_PLMN_MCC  + \
                      SIZE_EF_PLMN_MNC  + \
                      SIZE_EF_PLMN_LONG + \
                      SIZE_EF_PLMN_SHRT)
#define NR_EF_PLMN   2


/********************************************************************
 *
 * Field BCCH INFORMATION - BCCHINF
 *
 ********************************************************************/

typedef struct pcm_EFbcchinfo_Type
{
    UBYTE bcch_info[54]; /* content of bcch_info */
} EF_BCCHINFO;

#define SIZE_EF_BCCHINFO 54
#define NR_EF_BCCHINFO    1


/********************************************************************
 *
 * Field ALTERNATIVE LINE SERVICE - ALS
 *
 ********************************************************************/

typedef struct pcm_EFals_Type
{
    UBYTE selLine;  /* selected line */
    UBYTE statLine; /* status line   */
} EF_ALS;

#define SIZE_EF_ALS 2
#define NR_EF_ALS   1


/********************************************************************
 *
 * Field LOCATION INFORMATION (GPRS) - LOCGPRS
 *
 ********************************************************************/

typedef struct pcm_EFlocgprs_Type
{
    UBYTE ptmsi[4];           /* packet TMSI                 */
    UBYTE ptmsi_signature[3]; /* packet TMSI signature value */
    UBYTE rai[6];             /* routing area information    */
    UBYTE ra_status;          /* status of rai               */
} EF_LOCGPRS;

#define SIZE_EF_LOCGPRS sizeof (EF_LOCGPRS)
#define NR_EF_LOCGPRS   1


/********************************************************************
 *
 * Field CIPHERING KEY (GPRS) - KCGPRS
 *
 ********************************************************************/

typedef struct pcm_EFkcgprs_Type
{
    UBYTE kc[8]; /* currently used cyphering key        */
    UBYTE cksn;	 /* ciphering key sequence number of kc */
} EF_KCGPRS;

#define SIZE_EF_KCGPRS sizeof (EF_KCGPRS)
#define NR_EF_KCGPRS   1


/********************************************************************
 *
 * Field IMSI (GPRS) - IMSIGPRS
 *
 ********************************************************************/

typedef struct pcm_EFimsigprs_Type
{
    UBYTE len;	   /* length IMSI */
    UBYTE IMSI[8]; /* IMSI        */
} EF_IMSIGPRS;

#define SIZE_EF_IMSIGPRS sizeof (EF_IMSIGPRS)
#define NR_EF_IMSIGPRS   1


/********************************************************************
 *
 * Prototypes
 *
 ********************************************************************/

EXTERN unsigned char pcm_find_active_pcm_sector (void);

EXTERN void pcm_read_flash (UBYTE   *pcm_mem,
                           unsigned size,
                           UBYTE    pcm_sector);

EXTERN drv_Return_Type pcm_erase_flash_sector (UBYTE pcm_sector);

EXTERN drv_Return_Type pcm_write_flash (UBYTE    *pcm_mem,
                                        unsigned size,
                                        UBYTE    pcm_sector);

EXTERN drv_Return_Type pcm_Init (void);

EXTERN void pcm_Exit (void);

EXTERN drv_Return_Type pcm_ReadFile (UBYTE  *in_FileName,
                                     USHORT in_BufferSize,
                                     UBYTE  *out_BufferPtr,
                                     UBYTE  *out_VersionPtr);

EXTERN drv_Return_Type pcm_GetFileInfo (UBYTE             *in_FileName,
                                        pcm_FileInfo_Type *out_FileInfoPtr);

EXTERN drv_Return_Type pcm_ReadRecord (UBYTE  *in_FileName,
                                       USHORT in_Record,
                                       USHORT in_BufferSize,
                                       UBYTE  *out_BufferPtr,
                                       UBYTE  *out_VersionPtr,
                                       USHORT *out_MaxRecordsPtr);

EXTERN drv_Return_Type pcm_WriteFile (UBYTE  *in_FileName,
                                      USHORT in_BufferSize,
                                      UBYTE  *in_BufferPtr);

EXTERN drv_Return_Type pcm_WriteRecord (UBYTE  *in_FileName,
                                        USHORT in_Record,
                                        USHORT in_BufferSize,
                                        UBYTE  *in_BufferPtr);

EXTERN drv_Return_Type pcm_Flush (void);

#endif // ifndef DEF_PCM

/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-MFW
|  Modul   :  
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  Types definitions for the permanent memory configuration.
+----------------------------------------------------------------------------- 
*/ 
/********************************************************************************
 $History: pcm.h

	Jun 14, 2005 REF: MMI-FIX-30439 x0018858
   	Description: The homezone/cityzone tags were not being displayed properly.
   	Solution: Modified to save the tags properly in caches and also made the 
	appropriate modifications to update the same properly during mobility.

********************************************************************************/

#ifndef PCM_H
#define PCM_H

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

#define EF_MSCAP_ID     "MSCAP"         /* Mobile Capabilities      */
#define EF_IMEI_ID      "IMEI"          /* Int. Mobile Equipment Id */
#define EF_IMSI_ID      "IMSI"          /* Int. Mobile Subscriber Id */
#define EF_SMS_ID       "SMS"           /* Short Messages           */
#define EF_CLASS2_ID    "CLASS2"        /* MS Classmark 2           */
#define EF_CLASS3_ID    "CLASS3"        /* MS Classmark 3 (old def.)*/
#define EF_RFCAP_ID     "RFCAP"         /* RF Capabilities          */
#define EF_MSSUP_ID     "MSSUP"         /* Mobile Setup             */
#define EF_MSSET_ID     "MSSET"         /* Mobile Setting           */
//	Jun 14, 2005 REF: MMI-FIX-30439 x0018858
#define EF_HZCACHE_ID   "HZCACHE"     /* Homezone cache             */
#define EF_LDN_ID       "LDN"           /* Last Dialed Numbers      */
#define EF_LRN_ID       "LRN"           /* Last Received Numbers    */
#define EF_LMN_ID       "LMN"           /* Last Missed Numbers      */
#define EF_UPN_ID       "UPN"           /* User Personal Numbers    */
#define EF_MBN_ID       "MBN"           /* Mailbox Numbers          */
#define EF_VMN_ID       "VMN"           /* Voice Mail Number        */
#define EF_CLNG_ID      "CLNG"          /* Current LAnguage         */
#define EF_CTIM_ID      "CTIM"          /* Call Timer               */
#define EF_CCNT_ID      "CCNT"          /* Call Counter             */
#define EF_ECC_ID       "ECC"           /* Emergency Call Codes     */
#define EF_ORG_ID       "ORG"           /* Organizer and Alarm      */
#define EF_CCP_ID       "CCP"           /* Caps and Config Params   */
#define EF_EXT1_ID      "EXT1"          /* Extension 1              */
#define EF_SIMLCK_ID    "SIMLCK"        /* SIM Lock                 */
#define EF_SIMLCKEXT_ID "SIMLCKEXT"     /* SIM Lock Extended        */
#define EF_MAIN_ID      "MAIN"          /* Maintenance              */
#define EF_SFK_ID       "SFK"           /* Special Function Key     */
#define EF_FAULT_ID     "FAULT"         /* Fault Conditions         */
#define EF_DEBUG_ID     "DEBUG"         /* Debug Information        */
#define EF_POWER_ID     "POWER"         /* Power Management         */
#define EF_KEYB_ID      "KEYB"          /* Keyboard Mapping         */
#define EF_RADIO_ID     "RADIO"         /* Radio Parameter          */
#define EF_CGMI_ID      "CGMI"          /* Manufacturer             */
#define EF_INF0_ID      "INF0"          /* Manufacturer 1           */
#define EF_CGMM_ID      "CGMM"          /* Model                    */
#define EF_CGMR_ID      "CGMR"          /* Revision                 */
#define EF_CGSN_ID      "CGSN"          /* Product Serial Number    */
#define EF_SMSPRFL_ID   "SMSPRFL"       /* SMS Profile              */
#define EF_PLMN_ID      "PLMN"          /* PLMN Identifier          */
#define EF_BCCHINFO_ID  "BCCHINF"       /* BCCH Information         */
#define EF_ALS_ID       "ALS"           /* alternate line service   */
#define EF_LOCGPRS_ID   "LOCGPRS"       /* Location Inf. (GPRS)     */
#define EF_KCGPRS_ID    "KCGPRS"        /* Ciphering Key (GPRS)     */
#define EF_IMSIGPRS_ID  "IMSIGPRS"      /* IMSI check for GPRS      */

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
  UBYTE  * FileLocation;
  USHORT   FileSize;
  UBYTE    Version;
} pcm_FileInfo_Type;


/********************************************************************
 *
 * Field MOBILE CAPABILITIES
 *
 ********************************************************************/

typedef struct pcm_EFmscap_Type         /* Mobile Capabilities      */
{
  UBYTE chnMode;                        /* channel modes            */
  UBYTE datCap1;                        /* data capabilities        */
  UBYTE datCap2;                        /* data capabilities        */
  UBYTE featLst1;                       /* feature list             */
  UBYTE featLst2;                       /* feature list             */
  UBYTE featLst3;                       /* feature list             */
} EF_MSCAP;

#define SIZE_EF_MSCAP 6
#define NR_EF_MSCAP   1

/*
 * chnMode bits
 *  
 *  Octet 8     7     6     5     4     3     2     1
 *       L1    Tm   afs   ahs  spV3  efrV2   hr  spV1
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
 * datCap bits
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
#define TSynSup     0x00000010,4        /* NT Sync data support     */
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
 * Field IMEI
 *
 ********************************************************************/

typedef struct pcm_EFimei_Type          /* International ME Id      */
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
#define NR_EF_IMEI 1

/********************************************************************
 *
 * Field IMSI
 *
 ********************************************************************/

typedef struct pcm_EFimsi_Type       /* International Subscriber Id */
{
  UBYTE len;
  UBYTE IMSI[8];
} EF_IMSI;

#define SIZE_EF_IMSI 9
#define NR_EF_IMSI   1

/********************************************************************
 *
 * Field SHORT MESSAGE SERVICES
 *
 ********************************************************************/

typedef struct pcm_EFsms_Type           /* Short Messages           */
{
  UBYTE stat;
  UBYTE rmd[175];
} EF_SMS;  

#define SIZE_EF_SMS 176
#ifdef _SIMULATION_
#define NR_EF_SMS     3
#else
#define NR_EF_SMS     1
#endif

/********************************************************************
 *
 * Field MOBILE STATION CLASSMARK 2
 *
 ********************************************************************/

typedef struct pcm_EFclass2_Type      /* Mobile Station Classmark 2 */
{
  UBYTE  byte1;
  UBYTE  byte2;
  UBYTE  byte3;
} EF_CLASS2; 

#define SIZE_EF_CLASS2  3
#define NR_EF_CLASS2    1

/* 
 * byte1
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
 * byte2
 */
#define freqCap     0x00000001,0        /* frequency capability         */
#define freqCapm    0x00000001
#define freqCaps    0
#define vgcsCap     0x00000002,1        /* VGCS notification reception  */
#define vgcsCapm    0x00000002
#define vgcsCaps    1
#define vbsCap      0x00000004,2        /* VBS  notification reception  */
#define vbsCapm     0x00000004
#define vbsCaps     2
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
 * byte3
 */
#define a52         0x00000001,0        /* encryption algorithm A5/2    */
#define a52m        0x00000001
#define a52s        0
#define a53         0x00000002,1        /* encryption algorithm A5/3    */
#define a53m        0x00000002
#define a53s        1
#define cmspCap     0x00000004,2        /* CM service prompt            */
#define cmspCapm    0x00000004
#define cmspCaps    2
#define solsaCap    0x00000008,3        /* SoLSA                        */
#define solsaCapm   0x00000008
#define solsaCaps   3
#define ucs2Cap     0x00000010,4        /* UCS2 treatment               */
#define ucs2Capm    0x00000010
#define ucs2Caps    4
#define lcsvaCap    0x00000020,5        /* LCS VA capability            */
#define lcsvaCapm   0x00000020
#define lcsvaCaps   5
#define cm3bit      0x00000080,7        /* Classmark 3 available        */
#define cm3bitm     0x00000080
#define cm3bits     7


/********************************************************************
 *
 * Field MOBILE STATION CLASSMARK 3
 *
 ********************************************************************/

typedef struct pcm_EFclass3_Type    /* Mobile Station Classmark 3   */
{
    UBYTE byte1;
    UBYTE byte2;
    UBYTE byte3;
}EF_CLASS3;

#define SIZE_EF_CLASS3  3
#define NR_EF_CLASS3    1

/* 
 * byte1
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
 * byte2 
 */
#define rfCap1      0x0000000f,0        /* associated RF capability 1   */
#define rfCap1m     0x0000000f
#define rfCap1s     0
#define rfCap2      0x000000f0,4        /* associated RF capability 2   */
#define rfCap2m     0x000000f0
#define rfCap2s     4

/* 
 * byte2 
 */
#define ExtMeas     0x00000010,4        /* associated RF capability 1   */
#define ExtMeasm    0x00000010
#define ExtMeass    4

/********************************************************************
 *
 * Field RF Capabilities
 *
 ********************************************************************/

typedef struct pcm_EFrfcap_Type 
{
  UBYTE  setbands;        /* set frequency bands */
  UBYTE  bands;           /* supported frequency bands */
  UBYTE  power1;          /* power classes of GSM900 and DCS1800 */
  UBYTE  power2;          /* power classes of PCS1900 and GSM850 */
  UBYTE  power3;          /* power classes of GSM400 and EGDE */
  UBYTE  msGSM;           /* GSM multi slot capability and classes */
  UBYTE  msEDGE;          /* EDGE multi slot capability and classes */
  UBYTE  msHSCSD;         /* HSCSD multi slot capability and classes */
  UBYTE  msGPRS;          /* GPRS multi slot capability and classes */
  UBYTE  msECSD;          /* ECSD multi slot capability and classes */
  UBYTE  msEGPRS;         /* EGPRS multi slot capability and classes */
  UBYTE  capability1;      /* divers capabilities and options */
  UBYTE  capability2;      /* divers capabilities and options */
  UBYTE  switchmeasure;   /* switching time */
  UBYTE  encryption;      /* A5/n encryption algorithm availability */
  UBYTE  positioning;     /* supported positioning methods */
} EF_RFCAP;

#define SIZE_EF_RFCAP  16
#define NR_EF_RFCAP     1

/* 
 * setbands, bands
 */
#define rf_900        0x00000001,0  /* support of GSM 900   */
#define rf_900m       0x00000001
#define rf_900s       0
#define rf_1800       0x00000002,1  /* support of DCS 1800   */
#define rf_1800m      0x00000002
#define rf_1800s      1
#define rf_1900       0x00000004,2  /* support of PCS 1900   */
#define rf_1900m      0x00000004
#define rf_1900s      2
#define rf_EGSM       0x00000008,3  /* support of E-GSM     */
#define rf_EGSMm      0x00000008
#define rf_EGSMs      3
#define rf_850        0x00000010,4  /* support of GSM 850   */
#define rf_850m       0x00000010
#define rf_850s       4
#define rf_450        0x00000020,5  /* support of GSM 450   */
#define rf_450m       0x00000020
#define rf_450s       5
#define rf_480        0x00000040,6  /* support of GSM 480   */
#define rf_480m       0x00000040
#define rf_480s       6
#define rf_RGSM       0x00000080,7  /* support of R-GSM     */
#define rf_RGSMm      0x00000080
#define rf_RGSs       7

/* 
 * power1
 */
#define rfCap_1800    0x0000000f,0  /* associated RF capability of DCS 1800  */
#define rfCap_1800m   0x0000000f
#define rfCap_1800s   0
#define rfCap_900     0x000000f0,4  /* associated RF capability of GSM 900   */
#define rfCap_900m    0x000000f0
#define rfCap_900s    4
/* 
 * power2
 */
#define rfCap_850     0x0000000f,0  /* associated RF capability of GSM 850   */
#define rfCap_850m    0x0000000f
#define rfCap_850s    0
#define rfCap_1900    0x000000f0,4  /* associated RF capability of PCS 1900  */
#define rfCap_1900m   0x000000f0
#define rfCap_1900s   4
/* 
 * power3
 */
#define rfCap_EDGE2   0x00000003,0  /* associated RF capability 2 of EDGE    */
#define rfCap_EDGE2m  0x00000003
#define rfCap_EDGE2s  0
#define rfCap_EDGE1   0x0000000c,2  /* associated RF capability 1 of EDGE    */
#define rfCap_EDGE1m  0x0000000c
#define rfCap_EDGE1s  2
#define rfCap_400     0x000000f0,4  /* associated RF capability of GSM 450,480*/
#define rfCap_400m    0x000000f0
#define rfCap_400s    4
/* 
 * msGSM, msEDGE, msHSCSD, msGPRS, msECSD, msEGPRS
 */
#define rfCap_DTMSC     0x00000003,0  /* Multi Slot Sub-Class (only msGPRS+msEGPRS) */
#define rfCap_DTMSCm    0x00000003
#define rfCap_DTMSCs    0
#define rfCap_DTM       0x00000004,2  /* Support of DTM (only msGPRS+msEGPRS) */
#define rfCap_DTMm      0x00000004
#define rfCap_DTMs      2
#define rfCap_MSC       0x000000f8,3  /* Multi Slot Class */
#define rfCap_MSCm      0x000000f8
#define rfCap_MSCs      3
/* 
 * capability1
 */
#define rfCap_mac     0x00000001,0  /* Dynamic and fixed allocation */
#define rfCap_macm    0x00000001
#define rfCap_macs    0
#define rfCap_mod     0x00000002,1  /* EDGE modulation capability */
#define rfCap_modm    0x00000002
#define rfCap_mods    1
#define rfCap_cmsp    0x00000004,2  /* CM service prompt */
#define rfCap_cmspm   0x00000004
#define rfCap_cmsps   2
#define rfCap_solsa   0x00000008,3  /* SoLSA capability */
#define rfCap_solsam  0x00000008
#define rfCap_solsas  3
#define rfCap_lcsva   0x00000010,4  /* LCS value added location request noti. */
#define rfCap_lcsvam  0x00000010
#define rfCap_lcsvas  4
#define rfCap_ppsms   0x00000020,5  /* MT point to point SMS */
#define rfCap_ppsmsm  0x00000020
#define rfCap_ppsmss  5
#define rfCap_ps      0x00000040,6  /* pseudo synchronization capability */
#define rfCap_psm     0x00000040
#define rfCap_pss     6
#define rfCap_esind   0x00000080,7  /* controlled early class sending */
#define rfCap_esindm  0x00000080
#define rfCap_esinds  7
/* 
 * capability2
 */
#define rfCap_ssc     0x00000003,0  /* SS screening indicator */
#define rfCap_sscm    0x00000003
#define rfCap_sscs    0
#define rfCap_usc2    0x00000004,2  /* UCS2 encoded */
#define rfCap_usc2m   0x00000004
#define rfCap_usc2s   2
#define rfCap_vgcs    0x00000008,3  /* VGCS capability */
#define rfCap_vgcsm   0x00000008
#define rfCap_vgcss   3
#define rfCap_vbs     0x00000010,4  /* VBS capability */
#define rfCap_vbsm    0x00000010
#define rfCap_vbss    4
#define rfCap_compact   0x00000020,5  /* COMPACT interference measurement */
#define rfCap_compactm  0x00000020
#define rfCap_compacts  5
#define rfCap_extmeas   0x00000040,6  /* extendend measurement */
#define rfCap_extmeasm  0x00000040
#define rfCap_extmeass  6
#define rfCap_meas    0x00000080,7  /* values about measurement capability */
#define rfCap_measm   0x00000080
#define rfCap_meass   7
/* 
 * switchmeasure
 */
#define rfCap_smt     0x0000000f,0  /* time switch-power measurement */
#define rfCap_smtm    0x0000000f
#define rfCap_smts    0
#define rfCap_smst    0x000000f0,4  /* time switch-power measurement-switch */
#define rfCap_smstm   0x000000f0
#define rfCap_smsts   4
/* 
 * encryption
 */
#define rfCap_A5_7   0x00000002,1  /* encryption algorithm A5/7 */
#define rfCap_A5_7m  0x00000002
#define rfCap_A5_7s  1
#define rfCap_A5_6   0x00000004,2  /* encryption algorithm A5/6 */
#define rfCap_A5_6m  0x00000004
#define rfCap_A5_6s  2
#define rfCap_A5_5   0x00000008,3  /* encryption algorithm A5/5 */
#define rfCap_A5_5m  0x00000008
#define rfCap_A5_5s  3
#define rfCap_A5_4   0x00000010,4  /* encryption algorithm A5/4 */
#define rfCap_A5_4m  0x00000010
#define rfCap_A5_4s  4
#define rfCap_A5_3   0x00000020,5  /* encryption algorithm A5/3 */
#define rfCap_A5_3m  0x00000020
#define rfCap_A5_3s  5
#define rfCap_A5_2   0x00000040,6  /* encryption algorithm A5/2 */
#define rfCap_A5_2m  0x00000040
#define rfCap_A5_2s  6
#define rfCap_A5_1   0x00000080,7  /* encryption algorithm A5/1 */
#define rfCap_A5_1m  0x00000080
#define rfCap_A5_1s  7
/* 
 * positioning
 */
#define rfCap_eeda    0x00000002,1 /* EGPRS Extended Dynamic Allocation Capability */
#define rfCap_eedam   0x00000002
#define rfCap_eedas   1
#define rfCap_geda    0x00000004,2  /* GPRS Extended Dynamic Allocation Capability */
#define rfCap_gedam   0x00000004
#define rfCap_gedas   2
#define rfCap_cgps    0x00000008,3  /* conventional GPS */
#define rfCap_cgpsm   0x00000008
#define rfCap_cgpss   3
#define rfCap_bgps    0x00000010,4  /* based GPS */
#define rfCap_bgpsm   0x00000010
#define rfCap_bgpss   4
#define rfCap_agps    0x00000020,5  /* assisted GPS */
#define rfCap_agpsm   0x00000020
#define rfCap_agpss   5
#define rfCap_beotd   0x00000040,6  /* based E-OTD */
#define rfCap_beotdm  0x00000040
#define rfCap_beotds  6
#define rfCap_aeotd   0x00000080,7  /* assisted E-OTD */
#define rfCap_aeotdm  0x00000080
#define rfCap_aeotds  7


#ifdef TI_PS_CUSTOM_RFCAP_DEFAULT
EXTERN U8 custom_rfcap_default[SIZE_EF_RFCAP];
#endif /*  TI_PS_CUSTOM_RFCAP_DEFAULT */

/********************************************************************
 *
 * Field MOBILE SETUP
 *
 ********************************************************************/

typedef struct pcm_EFmssup_Type     /* Mobile Setup                 */
{
    UBYTE lng1;
    UBYTE lng2;
    UBYTE lng3;
    UBYTE feat1;
    UBYTE feat2;
}EF_MSSUP;

#define SIZE_EF_MSSUP 5
#define NR_EF_MSSUP   1

/* 
 * lng1
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
 * lng2
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
 * lng3
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
 * feat
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
 *          Field Current language (CLNG)
 *
 ********************************************************************/

#define SIZE_EF_CLNG_DATA 2

typedef struct pcm_EFclng_Type
{
  UBYTE data[SIZE_EF_CLNG_DATA]; /* current language in ME */
} EF_CLNG;

#define SIZE_EF_CLNG SIZE_EF_CLNG_DATA
#define NR_EF_CLNG   1
 
/********************************************************************
 *
 * Field MOBILE STATION SETTTINGS
 *
 ********************************************************************/

typedef struct pcm_EFmsset_Type     /* Mobile Setting               */
{
    UBYTE buzzer1;
    UBYTE buzzer2;
    UBYTE buzzer3;
    UBYTE audio;
    UBYTE misc;
    UBYTE display;
    UBYTE language;
    UBYTE recent_ldn_ref;
    UBYTE recent_lrn_ref;
    UBYTE recent_upn_ref;
} EF_MSSET;
   
#define SIZE_EF_MSSET 10
#define NR_EF_MSSET    1

/* 
 * buzzer 1 
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
 * buzzer 2
 */
#define msgtype   0x00000007,0        /* ringer type messages         */
#define msgtypem  0x00000007
#define msgtypes  0
#define msgvol    0x00000038,3        /* ringer volume messages       */
#define msgvolm   0x00000038
#define msgvols   3

/*
 * buzzer 3
 */
#define keytone   0x00000003,0        /* key tone mode                */
#define keytonem  0x00000003
#define keytones  0
#define batw      0x00000004,2        /* low battery warning          */
#define batwm     0x00000004
#define batws     2

/*
 * audio
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
 * miscellenous
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
 * display
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

//	Jun 14, 2005 REF: MMI-FIX-30439 x0018858
//Begin 30439
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
//end 30439

typedef struct pcm_EFldn_Type       /* Last Dialed Numbers          */
{
  UBYTE calDrMsb;
  UBYTE calDrLsb;
  UBYTE year;
  UBYTE month;
  UBYTE day;
  UBYTE hour;
  UBYTE minute;
  UBYTE second;
  UBYTE len;                        /* length of BCD number         */
  UBYTE numTp;
  UBYTE dldNum[10];                 /* dialed number                */
  UBYTE ccp;                        /* capability/configuration id  */
  UBYTE ext1;                       /* extension1 record identifier */
} EF_LDN;

#define SIZE_EF_LDN 22
#define NR_EF_LDN    1

/*
 * numTp
 */
#define numTp_npi   0x0000000F,0    /* numbering plan identification */
#define numTp_npim  0x0000000F
#define numTp_npis  0
#define numTp_ton   0x00000070,4    /* Type of number                */
#define numTp_tonm  0x00000070
#define numTp_tons  4

/********************************************************************
 *
 * Field LAST RECEIVED NUMBERS
 *
 ********************************************************************/

typedef struct pcm_EFlrn_Type       /* Last Received Numbers        */
{
  UBYTE calDrMsb;
  UBYTE calDrLsb;
  UBYTE year;
  UBYTE month;
  UBYTE day;
  UBYTE hour;
  UBYTE minute;
  UBYTE second;
  UBYTE id;
  UBYTE len;                            /* length of BCD number         */
  UBYTE numTp;
  UBYTE dldNum[10];                     /* dialed number                */
  UBYTE ccp;                            /* capability/configuration id  */
  UBYTE ext1;                           /* extension1 record identifier */
} EF_LRN;

#define SIZE_EF_LRN 23
#define NR_EF_LRN    1

/********************************************************************
 *
 * Field LAST MISSED NUMBERS
 *
 ********************************************************************/

typedef struct pcm_EFlmn_Type         /* Last Missed Numbers        */
{
  UBYTE year;
  UBYTE month;
  UBYTE day;
  UBYTE hour;
  UBYTE minute;
  UBYTE second;
  UBYTE id;
  UBYTE len;                            /* length of BCD number         */
  UBYTE numTp;
  UBYTE dldNum[10];                     /* dialed number                */
  UBYTE ccp;                            /* capability/configuration id  */
  UBYTE ext1;                           /* extension1 record identifier */
} EF_LMN;

#define SIZE_EF_LMN 21
#define NR_EF_LMN    1

/********************************************************************
 *
 * Field USER PERSONAL NUMBERS
 *
 ********************************************************************/

typedef struct pcm_EFupn_Type       /* User Personal Numbers        */
{
  UBYTE alphId[10];
  UBYTE len;
  UBYTE numTp;                      /* bitmap same as EF_LDN        */
  UBYTE usrNum[10];
  UBYTE ccp;
  UBYTE ext1;
} EF_UPN;

#define SIZE_EF_UPN 24
#define NR_EF_UPN    1

/********************************************************************
 *
 * Field MAILBOX NUMBERS
 *
 ********************************************************************/

typedef struct pcm_EFmbn_Type           /* Mailbox Numbers        */
{
  UBYTE alphId[10];
  UBYTE len;
  UBYTE numTp;                          /* bitmap same as EF_LDN        */
  UBYTE mbNum[10];
} EF_MBN;

#define SIZE_EF_MBN 22
#define NR_EF_MBN    4


/********************************************************************
 *
 * Field Voice Mail Number                                               
 *
 ********************************************************************/

/* 
 * note that with new 04.08 the called party bcd number of the CC
 * protocol can have up to 43 octets, 3 are used for other things 
 * than the BCD coded digits
 */
#define MAX_CALLED_PARTY_BCD_NO_OCTETS 40
typedef struct pcm_EFvmn_Type         /* Voice mail Number         */
{
  UBYTE vmNum[MAX_CALLED_PARTY_BCD_NO_OCTETS + 1];
  /* implementation uses delimiter of 0xFF */
  UBYTE numTp;

} EF_VMN;

#define SIZE_EF_VMN (MAX_CALLED_PARTY_BCD_NO_OCTETS + 1 + 1)
#define NR_EF_VMN    1


/********************************************************************
 *
 * Field CALL TIMER
 *
 ********************************************************************/

typedef struct pcm_EFctim_Type      /* Call Timer                   */
{
  UBYTE moVcDrHm[4];
  UBYTE mtVcDrHm[4];
  UBYTE moDtDrHm[4];
  UBYTE mtDtDrHm[4];
  UBYTE moFxDrHm[4];
  UBYTE mtFxDrHm[4];
  UBYTE moVcDrRm[4];
  UBYTE mtVcDrRm[4];
  UBYTE moDtDrRm[4];
  UBYTE mtDtDrRm[4];
  UBYTE moFxDrRm[4];
  UBYTE mtFxDrRm[4];
} EF_CTIM;

#define SIZE_EF_CTIM 48
#define NR_EF_CTIM    1

/********************************************************************
 *
 * Field CALL COUNTER
 *
 ********************************************************************/

typedef struct pcm_EFccnt_Type     /* Call Counter                  */
{
  UBYTE total[4];
  UBYTE moVcDrHm[4];
  UBYTE mtVcDrHm[4];
  UBYTE moDtDrHm[4];
  UBYTE mtDtDrHm[4];
  UBYTE moFxDrHm[4];
  UBYTE mtFxDrHm[4];
  UBYTE moVcDrRm[4];
  UBYTE mtVcDrRm[4];
  UBYTE moDtDrRm[4];
  UBYTE mtDtDrRm[4];
  UBYTE moFxDrRm[4];
  UBYTE mtFxDrRm[4];
} EF_CCNT;

#define SIZE_EF_CCNT 52
#define NR_EF_CCNT    1

/********************************************************************
 *
 * Field EMERGENCY CALL CODES
 *
 ********************************************************************/


typedef struct pcm_EFecc_Type           /* Emergency Call Codes         */
{
  UBYTE ecc1[3];                        /* emergency call code          */
  UBYTE ecc2[3];
  UBYTE ecc3[3];
  UBYTE ecc4[3];
  UBYTE ecc5[3];
} EF_ECC;

#define SIZE_EF_ECC 15
#define NR_EF_ECC    1

/********************************************************************
 *
 * Field ORGANIZER AND ALARM
 *
 ********************************************************************/


typedef struct pcm_EForg_Type           /* Organizer and Alarm          */
{
  UBYTE date[6];                        /* bitmap same as EF_LDN        */
  UBYTE alrm;
  UBYTE alphMem[16];                    /* alpha memo                   */
} EF_ORG;
                  
#define SIZE_EF_ORG 23
#define NR_EF_ORG    1

/*
 *  alrm
 */
#define alrm_stat        0x00000001,0
#define alrm_statm       0x00000001
#define alrm_stats       0
#define alrm_type        0x0000000E,1
#define alrm_typem       0x0000000E
#define alrm_types       1

/********************************************************************
 *
 * Field CAPABILITY AND CONFIGURATION PARAMETERS
 *
 ********************************************************************/

typedef struct pcm_EFccp_Type    /* Capability and Configuration Parameters */
{
  UBYTE usrRate;                        /* user rate                    */
  UBYTE bearServ;                       /* bearer service               */
  UBYTE conElem;                        /* connection element           */
  UBYTE stopBits;                       /* stop bits                    */
  UBYTE dataBits;                       /* data bits                    */
  UBYTE parity;                         /* parity                       */
  UBYTE flowCntrl;                      /* flow control                 */
} EF_CCP;

#define SIZE_EF_CCP 7
#define NR_EF_CCP   1

/********************************************************************
 *
 * Field EXTENSION 1
 *
 ********************************************************************/

typedef struct pcm_EFext1_Type        /* Extension 1                  */
{
  UBYTE recTp;                          /* record type                  */
  UBYTE extDat[11];                     /* extension data               */
  UBYTE id;                             /* identifier                   */
} EF_EXT1;

#define SIZE_EF_EXT1 13
#define NR_EF_EXT1    1

/********************************************************************
 *
 * Field SIM LOCK and Extended SIM LOCK (use alternativly)
 *
 ********************************************************************/

typedef struct pcm_EFsimlck_Type       /* SIM Lock                     */
{
  UBYTE locks1;                         /* lock status                  */
  UBYTE locks2;
  UBYTE cnt;                            /* lock counter                 */
  UBYTE maxcnt;                         /* lock counter                 */
  UBYTE PKey[8];                        /* SIM control key              */
  UBYTE SPKey[8];                       /* SP control key               */
  UBYTE NSKey[8];                       /* NS control key               */
  UBYTE CKey[8];                        /* corporate control key        */
  UBYTE NKey[8];                        /* network control key          */
  UBYTE len_imsi;                       /* bytes of IMSI               */
  UBYTE imsi[15];                       /* IMSI                         */
  UBYTE gidl1;                          /* Group Identifier Level 1     */
  UBYTE gidl2;                          /* Group Identifier Level 1     */
} EF_SIMLCK;

#define SIZE_EF_SIMLCK 62
#define NR_EF_SIMLCK    1

typedef struct pcm_EFsimlckext_Type     /* Extended SIM Lock            */
{
  UBYTE locks1;                         /* lock status                  */
  UBYTE locks2;
  UBYTE cnt;                            /* lock counter                 */
  UBYTE maxcnt;                         /* lock counter                 */
  UBYTE PKey[8];                        /* SIM control key              */
  UBYTE SPKey[8];                       /* SP control key               */
  UBYTE NSKey[8];                       /* NS control key               */
  UBYTE CKey[8];                        /* corporate control key        */
  UBYTE NKey[8];                        /* network control key          */
  UBYTE len_p_imsi;                     /* bytes of IMSI P-Lock         */
  UBYTE p_imsi[15];                     /* IMSI P-Lock                  */
  UBYTE len_sp_imsi;                    /* bytes of IMSI SP-Lock        */
  UBYTE sp_imsi[15];                    /* IMSI SP-Lock                 */
  UBYTE len_ns_imsi;                    /* bytes of IMSI NS-Lock        */
  UBYTE ns_imsi[15];                    /* IMSI NS-Lock                 */
  UBYTE len_c_imsi;                     /* bytes of IMSI C-Lock         */
  UBYTE c_imsi[15];                     /* IMSI C-Lock                  */
  UBYTE len_n_imsi;                     /* bytes of IMSI N-Lock         */
  UBYTE n_imsi[15];                     /* IMSI N-Lock                  */
  UBYTE len_u_imsi;                     /* bytes of IMSI U-Lock         */
  UBYTE u_imsi[15];                     /* IMSI U-Lock                  */
  UBYTE gidl1;                          /* Group Identifier Level 1     */
  UBYTE gidl2;                          /* Group Identifier Level 1     */
} EF_SIMLCKEXT;

#define SIZE_EF_SIMLCKEXT 142
#define NR_EF_SIMLCKEXT     1

/*
 *  locks1
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
 *  locks2
 */
#define clock            0x00000003,0
#define clockm           0x00000003
#define clocks           0
#define flock            0x0000000C,2
#define flockm           0x0000000C
#define flocks           2
/*
#define reserved1        0x00000030,4
#define reserved1m       0x00000030
#define reserved1s       4
#define reserved2        0x000000C0,6
#define reserved2m       0x000000C0
#define reserved2s       6
*/

/********************************************************************
 *
 * Field MAINTENANCE INFORMATION
 *
 ********************************************************************/

/*
 *  T.B.D.
 */
#define SIZE_EF_MAIN  8
#define NR_EF_MAIN    1

/********************************************************************
 *
 * Field SPECIAL FUNCTION KEY
 *
 ********************************************************************/

/*
 *  T.B.D.
 */
#define SIZE_EF_SFK   8
#define NR_EF_SFK     1

/********************************************************************
 *
 * Field FAULT CONDITIONS
 *
 ********************************************************************/

/*
 *  T.B.D.
 */
#define SIZE_EF_FAULT 8
#define NR_EF_FAULT   1

/********************************************************************
 *
 * Field DEBUG INFORMATION
 *
 ********************************************************************/

/*
 *  T.B.D.
 */
#define SIZE_EF_DEBUG 8
#define NR_EF_DEBUG   1

/********************************************************************
 *
 * Field POWER MANAGEMENT
 *
 ********************************************************************/

/*
 *  T.B.D.
 */
#define SIZE_EF_POWER 8
#define NR_EF_POWER   1

 /********************************************************************
 *
 * Field KEYBOARD MAPPING
 *
 ********************************************************************/

typedef struct pcm_EFkbd_Type       /* Keyboard Mapping             */
{
  UBYTE logical_key [32];           /* logical key                  */
  UBYTE raw_key [32];               /* raw key                      */
} EF_KBD;

#define SIZE_EF_KEYB 64
#define NR_EF_KEYB    1

/********************************************************************
 *
 * Field RADIO PARAMETERS
 *
 ********************************************************************/

/*
 *  T.B.D.
 */
#define SIZE_EF_RADIO 8
#define NR_EF_RADIO   1

/********************************************************************
 *
 * Manufacturer
 *
 ********************************************************************/

#define SIZE_EF_CGMI_DATA 20

typedef struct pcm_EFcgmi_Type   /* Manufacturer                 */
{
  UBYTE data[SIZE_EF_CGMI_DATA]; /* Name of Manufacturer         */
} EF_CGMI;

#define SIZE_EF_CGMI SIZE_EF_CGMI_DATA
#define NR_EF_CGMI   1

/********************************************************************
 *
 *  Identification Information 
 *
 ********************************************************************/

#define SIZE_EF_INF0_DATA 20

typedef struct pcm_EFinf0_Type
{
  UBYTE data[SIZE_EF_INF0_DATA]; /*Identification Information*/
} EF_INF0;

#define SIZE_EF_INF0 SIZE_EF_INF0_DATA
#define NR_EF_INF0   2

/********************************************************************
 *
 * Model
 *
 ********************************************************************/

#define SIZE_EF_CGMM_DATA 20

typedef struct pcm_EFcgmm_Type   /* Model                        */
{
  UBYTE data[SIZE_EF_CGMM_DATA]; /* Name of Product              */
} EF_CGMM;

#define SIZE_EF_CGMM SIZE_EF_CGMM_DATA
#define NR_EF_CGMM   1

/********************************************************************
 *
 * Revision
 *
 ********************************************************************/

#define SIZE_EF_CGMR_DATA 20

typedef struct pcm_EFcgmr_Type   /* Revision                     */
{
  UBYTE data[SIZE_EF_CGMR_DATA]; /* Version of Product           */
} EF_CGMR;

#define SIZE_EF_CGMR SIZE_EF_CGMR_DATA
#define NR_EF_CGMR   1

/********************************************************************
 *
 * Product Serial Number
 *
 ********************************************************************/

#define SIZE_EF_CGSN_DATA 20

typedef struct pcm_EFcgsn_Type   /* Product Serial Number        */
{
  UBYTE data[SIZE_EF_CGSN_DATA]; /* Serial Number of Product     */
} EF_CGSN;

#define SIZE_EF_CGSN SIZE_EF_CGSN_DATA
#define NR_EF_CGSN   1

/********************************************************************
 *
 * SMS Profile
 *
 ********************************************************************/

#define SIZE_EF_SMSPRFL_SCA   20
#define SIZE_EF_SMSPRFL_MIDS  40
#define SIZE_EF_SMSPRFL_DCSS  20
#define SIZE_EF_SMSPRFL_VPABS 14

#define EF_SMSPRFL_VLD        0x00
#define EF_SMSPRFL_INVLD      0xFF

typedef struct pcm_EFsmsprfl_Type         /* SMS Profile                      */
{
  UBYTE vldFlag;                          /* Valid Flag                       */
  UBYTE CSCAsca[SIZE_EF_SMSPRFL_SCA];     /* Service Center Address           */  
  UBYTE CSCAlenSca;                       /* Length of Service Center Address */
  UBYTE CSCAton;                          /* Type of Number                   */
  UBYTE CSCAnpi;                          /* Numbering Plan Identification    */
  UBYTE CSCBmode;                         /* Mode                             */
  UBYTE CSCBmids[SIZE_EF_SMSPRFL_MIDS];   /* Message Identifier               */
  UBYTE CSCBdcss[SIZE_EF_SMSPRFL_DCSS];   /* Data Coding Schemes              */
  UBYTE CSMPfo;                           /* First Octet                      */
  UBYTE CSMPvprel;                        /* Validity Period Relative         */
  UBYTE CSMPvpabs[SIZE_EF_SMSPRFL_VPABS]; /* Validity Period Absolute         */
  UBYTE CSMPpid;                          /* Protocol Identifier              */
  UBYTE CSMPdcs;                          /* Data Coding Scheme               */
} EF_SMSPRFL;

#define SIZE_EF_SMSPRFL ( SIZE_EF_SMSPRFL_SCA   +   \
                          SIZE_EF_SMSPRFL_MIDS  +   \
                          SIZE_EF_SMSPRFL_DCSS  +   \
                          SIZE_EF_SMSPRFL_VPABS + 9   )
#define NR_EF_SMSPRFL   2

/********************************************************************
 *
 * PLMN Identifier
 *
 ********************************************************************/

#define SIZE_EF_PLMN_LONG 20
#define SIZE_EF_PLMN_SHRT 10
#define SIZE_EF_PLMN_MCC   2
#define SIZE_EF_PLMN_MNC   2

typedef struct pcm_EFplmn_Type      /* PLMN Identifier              */
{
  UBYTE mcc[SIZE_EF_PLMN_MCC];      /* Mobile country code          */
  UBYTE mnc[SIZE_EF_PLMN_MNC];      /* Mobile network code          */
  UBYTE lngNam[SIZE_EF_PLMN_LONG];  /* Long operator name           */
  UBYTE shrtNam[SIZE_EF_PLMN_SHRT]; /* Short operator name          */
} EF_PLMN;

#define SIZE_EF_PLMN ( SIZE_EF_PLMN_MCC  + SIZE_EF_PLMN_MCC +   \
                       SIZE_EF_PLMN_LONG + SIZE_EF_PLMN_SHRT )
#define NR_EF_PLMN   2

/********************************************************************
 *
 * Field BCCH Information
 *
 ********************************************************************/

typedef struct pcm_EFbcchinfo_Type  /* BCCH information             */
{
  UBYTE bcch_info[54];              /* content                      */
} EF_BCCHINFO;

#define SIZE_EF_BCCHINFO 54
#define NR_EF_BCCHINFO    1

/********************************************************************
 *
 * Field alternate line service
 *
 ********************************************************************/

typedef struct pcm_EFals_Type  /* alternate line service            */
{
  UBYTE selLine;
  UBYTE statLine;
} EF_ALS;

#define SIZE_EF_ALS 2
#define NR_EF_ALS   1

/********************************************************************
 *
 * Field Location Information (GPRS)
 *
 ********************************************************************/

typedef struct pcm_EFlocgprs_Type   /* GPRS Location information    */
{
  UBYTE ptmsi[4];
  UBYTE ptmsi_signature[3];
  UBYTE rai[6];
  UBYTE ra_status;
} EF_LOCGPRS;

#define SIZE_EF_LOCGPRS sizeof (EF_LOCGPRS)
#define NR_EF_LOCGPRS   1

/********************************************************************
 *
 * Field Ciphering Key (GPRS)
 *
 ********************************************************************/

typedef struct pcm_EFkcgprs_Type         /* GPRS Ciphering key      */
{
  UBYTE kc[8];
  UBYTE cksn;
} EF_KCGPRS;

#define SIZE_EF_KCGPRS sizeof (EF_KCGPRS)
#define NR_EF_KCGPRS   1

/********************************************************************
 *
 * Field IMSI (GPRS, only for validation of GPRS related fields)
 *
 ********************************************************************/

typedef struct pcm_EFimsigprs_Type    /* International Subscriber Id */
{
  UBYTE len;
  UBYTE IMSI[8];
} EF_IMSIGPRS;

#define SIZE_EF_IMSIGPRS sizeof (EF_IMSIGPRS)
#define NR_EF_IMSIGPRS   1

/********************************************************************
 *
 * Prototypes
 *
 ********************************************************************/

#if defined (NEW_FRAME)
/*
 * to achieve backward compatibility with older definitions
 */
#define drv_SignalCB_Type           T_DRV_CB_FUNC
#define drv_SignalID_Type           T_DRV_SIGNAL
#define T_VSI_THANDLE               USHORT
#endif

EXTERN drv_Return_Type pcm_Init         (void);
EXTERN void pcm_Exit                    (void);
EXTERN drv_Return_Type pcm_ReadFile     (UBYTE  * in_FileName,
                                         USHORT   in_BufferSize,
                                         UBYTE  * out_BufferPtr,
                                         UBYTE  * out_VersionPtr);
EXTERN drv_Return_Type pcm_GetFileInfo  (UBYTE  * in_FileName,
                                         pcm_FileInfo_Type * out_FileInfoPtr);
EXTERN drv_Return_Type pcm_ReadRecord   (UBYTE  * in_FileName,
                                         USHORT   in_Record,
                                         USHORT   in_BufferSize,
                                         UBYTE  * out_BufferPtr,
                                         UBYTE  * out_VersionPtr,
                                         USHORT * out_MaxRecordsPtr);
EXTERN drv_Return_Type pcm_WriteFile    (UBYTE  * in_FileName,
                                         USHORT   in_BufferSize,
                                         UBYTE  * in_BufferPtr);
EXTERN drv_Return_Type pcm_WriteRecord  (UBYTE  * in_FileName,
                                         USHORT   in_Record,
                                         USHORT   in_BufferSize,
                                         UBYTE  * in_BufferPtr);
EXTERN drv_Return_Type pcm_Flush        (void);


#endif



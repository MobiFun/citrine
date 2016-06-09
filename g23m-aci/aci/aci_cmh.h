/*  
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_CMH
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
|  Purpose :  Command handler interface definitions
+-----------------------------------------------------------------------------
*/

#ifndef ACI_CMH_H 
#define ACI_CMH_H

#ifdef TI_PS_FF_AT_P_CMD_CTREG
#include "cl_shrd.h"
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

#include "p_sim.h"

/*#ifdef SIM_TOOLKIT */
/*#if defined (FAX_AND_DATA) || defined (GPRS) || defined (_SIMULATION_) */
/*#define FF_SAT_E */ /* enable SAT CLASS E */
/*#endif  F&D or GPRS or Simulation */
/*#endif  SIM_TOOLKIT */
/*==== DATA TYPES FROM OLD SMS SAP (TEMPORARY) ====================*/

/*
 * service center address
 */
typedef struct
{
  UBYTE                     ntype;                    /*<  0:  1> numbering type                                     */
  UBYTE                     nplan;                    /*<  1:  1> numbering plan                                     */
  UBYTE                     no_bcd;                   /*<  2:  1> number of BCD digits                               */
  UBYTE                     bcd[MAX_SMS_ADDR_DIG];    /*<  3: 20> address                                            */
} T_sc_addr;


typedef struct
{
  UBYTE                     year[2];                  /*<  0:  2> year                                               */
  UBYTE                     month[2];                 /*<  2:  2> month                                              */
  UBYTE                     day[2];                   /*<  4:  2> day                                                */
  UBYTE                     hour[2];                  /*<  6:  2> hour                                               */
  UBYTE                     minute[2];                /*<  8:  2> minute                                             */
  UBYTE                     second[2];                /*< 10:  2> second                                             */
  UBYTE                     timezone;                 /*< 12:  1> timezone                                           */
} T_vp_abs;

typedef enum
{
  SR_TYP_Name      =   0,
  SR_TYP_Number,
  SR_TYP_Index,
  SR_TYP_Physical
}
T_ACI_SR_TYP;

#ifdef FF_EM_MODE
#include "aci_em.h"
#endif /* FF_EM_MODE */

/*==== MACROS =====================================================*/
#define ACI_ERR_DESC_CLASS( errDesc ) ((errDesc&0xFFFF0000)>>16)
#define ACI_ERR_DESC_NR( errDesc )    (errDesc&0xFFFF)
#define ACI_ERR_DESC_BLD( errCls, errNr ) ((errCls<<16)+errNr)

/*==== DEFINES ====================================================*/

#define ACI_NumParmNotPresent (-1)
#define MAX_OPER              12
#define MAX_FACILITY          17

#define MAX_B_SUBSCR_NUM_LEN  (MAX_PARTY_NUM+1)        /* + '\0' */
#define MAX_CC_ORIG_NUM_LEN   (MNCC_MAX_CC_CALLED_NUMBER+1) /* + '\0' */


#ifdef TI_PS_FFS_PHB
/*
 * A phase 2 and above mobile has to support extension records always,
 * we support at least one extension record which
 * gives us a total length of 40 number digits without trailing '\0'.
 * The former preprocessor constant PHONEBOOK_EXTENSION has been 
 * eliminated.
 */
#define MAX_PHB_NUM_LEN       (40+1) /* Phonebook number length + '\0' */

#else

#ifdef PHONEBOOK_EXTENSION
  #define MAX_PHB_NUM_LEN     (44+1) /* Phonebook number length + '\0' */
#else
  #define MAX_PHB_NUM_LEN     (20+1) /* Phonebook number length + '\0' */
#endif /* else, #ifdef PHONEBOOK_EXTENSION */

#endif /* else, #ifdef TI_PS_FFS_PHB */

#define MAX_SUBADDR_LEN       (20+1)  /* + '\0' */
#define MAX_ALPHA_LEN         (20+1)  /* + '\0' */
#define MAX_ALPHA_OPER_LEN    (25+1)  /* + '\0' */
#define MAX_NUM_OPER_LEN      7
#define MAX_VP_ABS_DIGITS     2
#define MAX_SM_LEN            176
#define MAX_CBM_LEN           94
#define MAX_SM_CMD_LEN        158
#define MAX_CBM_TYPES         (MAX_IDENTS)
#define MAX_PB_ENTR           5     /* Should be the same as file: phb.h, MAX_PHB_ENTRY */
#define MAX_PB_INDEX          255
#ifdef FAX_AND_DATA
 #define MAX_CALL_NR           7
#else
 #define MAX_CALL_NR           6
#endif
#define MAX_SM_ENTR           1
#define MAX_USSD_LEN          183   /* max len in message is 160 can be 7bit compressed so 182*/
#define MAX_DIAL_LEN          183   /* should be the same as for ussd len */
#define MAX_IMSI_LEN          15    /* 3.03/2.3 */
#define MAX_SAT_PRF_LEN       (MAX_STK_PRF)  /* from SAP definition SIM.DOC */
#define MAX_SAT_CMD_LEN       255
#define MAX_CUR_LEN           4
#define MAX_MSISDN            2

#define CLAN_CODE_LEN         2
#define CBCH_HEAD_LEN         6
#define CMGW_IDX_FREE_ENTRY   0
#define MAX_SMS_NUM_LEN       (MAX_SMS_ADDR_DIG+1)
#define MAX_LAN               24   /* Maximal language supported */

#ifdef FF_CPHS_REL4
#define MAX_MB_NUM_LEN        (20+1) /* Mailbox number length + '\0' */
#endif /* FF_CPHS_REL4 */

#ifdef FF_HOMEZONE
#define CBHZ_MAX_TIMEOUT      120
#define CBHZ_MIN_TIMEOUT      30
#define CBHZ_DEF_TIMEOUT      60
#endif /* FF_HOMEZONE */

#ifndef NO_ASCIIZ
#define NO_ASCIIZ
#endif

/* A T T E N T I O N : this constants have to be verified */
#define MAX_CBM_ENTR 1
/* A T T E N T I O N : this constants have to be verified */

/* SMS: TP-Validity-Period-Format (TP-VPF) according to GSM 03.40 */
#define TP_VPF_NOT_PRESENT      0x00
#define TP_VPF_ENHANCED         0x08
#define TP_VPF_RELATIVE         0x10
#define TP_VPF_ABSOLUTE         0x18
#define TP_VPF_MASK             0x18

/* SMS: TP-Validity-Period-Enhanced extension bit */
#define TP_VPF_ENH_EXT_BIT_MASK 0x80

/* SMS: TP-Validity-Period-Enhanced single shot bit */
#define TP_VPF_ENH_SINGLE_SHOT_MASK 0x40

/* SMS: TP-Validity-Period-Enhanced-Format */
#define TP_VPF_ENH_NOT_PRESENT  0x00
#define TP_VPF_ENH_REL          0x01
#define TP_VPF_ENH_SEC          0x02
#define TP_VPF_ENH_HRS          0x03
#define TP_VPF_ENH_FORMAT_MASK  0x07

/* SMS: TP-Message-Type-Indicator (TP-MTI) according to GSM 03.40 */
#define TP_MTI_SMS_DELIVER      0x00
#define TP_MTI_SMS_DELIVER_REP  0x00
#define TP_MTI_SMS_SUBMIT       0x01
#define TP_MTI_SMS_SUBMIT_REP   0x01
#define TP_MTI_SMS_COMMAND      0x02
#define TP_MTI_SMS_STATUS_REP   0x02
#define TP_MTI_SMS_RESERVED     0x03
#define TP_MTI_MASK             0x03

/* SMS: TP-Status-Report-Request (TP-SRR) according to GSM 03.40 */
#define TP_SRR_NOT_REQUEST      0x00
#define TP_SRR_REQUEST          0x20
#define TP_SRR_MASK             0x20

/* SMS: TP-User-Data-Header-Indicator (TP-UDHI) according to GSM 03.40 */
#define TP_UDHI_WITHOUT_HEADER  0x00
#define TP_UDHI_WITH_HEADER     0x40
#define TP_UDHI_MASK            0x40

/* SMS: TP-Reply-Path (TP-RP) according to GSM 03.40 */
#define TP_RP_NOT_REQUEST       0x00
#define TP_RP_REQUEST           0x80
#define TP_RP_MASK              0x80

/* SMS/CBM: control parameter access in PCM */
#define ACI_PCM_ACCESS_SMSP     0x01
#define ACI_PCM_ACCESS_CBMP     0x02

/* EONS definitions */
#define OPL_MAX_RECORDS         50
#define OPL_MAX_RECORD_SIZE     8

#define PNN_MAX_RECORDS         10

/* CSQ, signal quality definitions*/
#define ACI_RSSI_FAULT          99
#define ACI_BER_FAULT           99
#ifdef FF_PS_RSSI
#define ACI_MIN_RXLEV_FAULT     99
#endif

/*CSP ALS service group code and bit value definitions*/
#define ACI_CPHS_INFO_SIZE      3
#define ACI_CPHS_CSP_SIZE       18
#define ALS_SERVICE_GROUP_CODE  0x06
#define ALS_BIT_ON              0x80
/*CSP VAS service group code and bit value definitions*/
#define VAS_SERVICE_GROUP_CODE  0xc0
#define PLMN_MODE_BIT_ON        0x80

#ifdef TI_PS_FF_AT_CMD_P_ECC
/*
 * Maximum length of an additional ECC number
 * and maximum number of additional ECC.
 */
#define ADDITIONAL_ECC_NUMBER_LENGTH 4 //3
#define ADDITIONAL_ECC_NUMBER_MAX    8
#endif /* TI_PS_FF_AT_CMD_P_ECC */
/*==== TYPES ======================================================*/

typedef enum             /* AT command identifier     */
{
  AT_CMD_NONE      = 0,  /* no ACI command identifier */
  AT_CMD_CACM      = 1,
  AT_CMD_CAMM      = 2,
  AT_CMD_CAOC      = 3,
  AT_CMD_CBC       = 4,
  AT_CMD_CBST      = 5,
  AT_CMD_CCFC      = 6,
  AT_CMD_CCUG      = 7,
  AT_CMD_CCWA      = 8,
  AT_CMD_CCWE      = 9,
  AT_CMD_CEER      = 10,
  AT_CMD_CFUN      = 11,
  AT_CMD_CGACT     = 12,
  AT_CMD_CGANS     = 13,
  AT_CMD_CGATT     = 14,
  AT_CMD_CGAUTO    = 15,
  AT_CMD_CGCLASS   = 16,
  AT_CMD_CGDATA    = 17,
  AT_CMD_CGDCONT   = 18,
  AT_CMD_CGEREP    = 19,
  AT_CMD_CGMI      = 20,
  AT_CMD_CGMM      = 21,
  AT_CMD_CGMR      = 22,
  AT_CMD_CGPADDR   = 23,
  AT_CMD_CGQMIN    = 24,
  AT_CMD_CGQREQ    = 25,
  AT_CMD_CGREG     = 26,
  AT_CMD_CGSMS     = 27,
  AT_CMD_CGSN      = 28,
  AT_CMD_CHLD      = 29,
  AT_CMD_CHUP      = 30,
  AT_CMD_CIMI      = 31,
  AT_CMD_CLAC      = 32,
  AT_CMD_CLAE      = 33,
  AT_CMD_CLAN      = 34,
  AT_CMD_CLCC      = 35,
  AT_CMD_CLCK      = 36,
  AT_CMD_CLIP      = 37,
  AT_CMD_CLIR      = 38,
  AT_CMD_CLVL      = 39,
  AT_CMD_CMEE      = 40,
  AT_CMD_CMGC      = 41,
  AT_CMD_CMGD      = 42,
  AT_CMD_CMGF      = 43,
  AT_CMD_CMGL      = 44,
  AT_CMD_CMGR      = 45,
  AT_CMD_CMGS      = 46,
  AT_CMD_CMGW      = 47,
  AT_CMD_CMOD      = 48,
  AT_CMD_CMSS      = 49,
  AT_CMD_CMUT      = 50,
  AT_CMD_CMUX      = 51,
  AT_CMD_CNMA      = 52,
  AT_CMD_CNMI      = 53,
  AT_CMD_CNUM      = 54,
  AT_CMD_COLP      = 55,
  AT_CMD_COPN      = 56,
  AT_CMD_COPS      = 57,
  AT_CMD_CPAS      = 58,
  AT_CMD_CPBF      = 59,
  AT_CMD_CPBR      = 60,
  AT_CMD_CPBS      = 61,
  AT_CMD_CPBW      = 62,
  AT_CMD_CPIN      = 63,
  AT_CMD_CPMS      = 64,
  AT_CMD_CPOL      = 65,
  AT_CMD_CPUC      = 66,
  AT_CMD_CPWD      = 67,
  AT_CMD_CR        = 68,
  AT_CMD_CRC       = 69,
  AT_CMD_CREG      = 70,
  AT_CMD_CRES      = 71,
  AT_CMD_CRLP      = 72,
  AT_CMD_CRSL      = 73,
  AT_CMD_CRSM      = 74,
  AT_CMD_CSAS      = 75,
  AT_CMD_CSCA      = 76,
  AT_CMD_CSCB      = 77,
  AT_CMD_CSCS      = 78,
  AT_CMD_CSDH      = 79,
  AT_CMD_CSMP      = 80,
  AT_CMD_CSMS      = 81,
  AT_CMD_CSNS      = 82,
  AT_CMD_CSQ       = 83,
  AT_CMD_CSSN      = 84,
  AT_CMD_CSTA      = 85,
  AT_CMD_CSVM      = 86,
  AT_CMD_CTFR      = 87,
  AT_CMD_CUSD      = 88,
  AT_CMD_DR        = 89,
  AT_CMD_DS        = 90,

#ifdef FF_FAX
  AT_CMD_FAP       = 92,
  AT_CMD_FBO       = 93,
  AT_CMD_FBS       = 94,
  AT_CMD_FBU       = 95,
  AT_CMD_FCC       = 96,
  AT_CMD_FCLASS    = 97,
  AT_CMD_FCQ       = 98,
  AT_CMD_FCR       = 99,
  AT_CMD_FCS       = 100,
  AT_CMD_FCT       = 101,
  AT_CMD_FDR       = 102,
  AT_CMD_FDT       = 103,
  AT_CMD_FEA       = 104,
  AT_CMD_FFC       = 105,
  AT_CMD_FHS       = 106,
  AT_CMD_FIE       = 107,
  AT_CMD_FIP       = 108,
  AT_CMD_FIS       = 109,
  AT_CMD_FIT       = 110,
  AT_CMD_FKS       = 111,
  AT_CMD_FLI       = 112,
  AT_CMD_FLO       = 113,
  AT_CMD_FLP       = 114,
  AT_CMD_FMI       = 115,
  AT_CMD_FMM       = 116,
  AT_CMD_FMR       = 117,
  AT_CMD_FMS       = 118,
  AT_CMD_FND       = 119,
  AT_CMD_FNR       = 120,
  AT_CMD_FNS       = 121,
  AT_CMD_FPA       = 122,
  AT_CMD_FPI       = 123,
  AT_CMD_FPS       = 125,
  AT_CMD_FPW       = 126,
  AT_CMD_FRQ       = 127,
  AT_CMD_FSA       = 129,
  AT_CMD_FSP       = 130,
#endif /* FF_FAX */

  AT_CMD_GCAP      = 131,
  AT_CMD_GCI       = 132,
  AT_CMD_GMI       = 133,
  AT_CMD_GMM       = 134,
  AT_CMD_GMR       = 135,
  AT_CMD_GSN       = 136,
  AT_CMD_ICF       = 137,
  AT_CMD_IFC       = 138,
  AT_CMD_ILRR      = 139,
  AT_CMD_IPR       = 140,
  AT_CMD_TM        = 141,
  AT_CMD_VST       = 142,
  AT_CMD_WS46      = 143,
  AT_CMD_ALS       = 144,
  AT_CMD_CLSA      = 145,
  AT_CMD_CLOM      = 146,
  AT_CMD_CLPS      = 147,
  AT_CMD_CLSR      = 148,
  AT_CMD_BAND      = 149,
  AT_CMD_P_CACM    = 150,
  AT_CMD_P_CAOC    = 151,
  AT_CMD_CCBS      = 152,
  AT_CMD_CGAATT    = 153,
  AT_CMD_P_CGMM    = 154,
  AT_CMD_P_CGREG   = 155,
  AT_CMD_CNAP      = 156,
  AT_CMD_CPI       = 157,
  AT_CMD_CTTY      = 158,
  AT_CMD_COLR      = 159,
  AT_CMD_CPRIM     = 160,
  AT_CMD_CTV       = 161,
  AT_CMD_CUNS      = 162,
  AT_CMD_NRG       = 163,
  AT_CMD_PPP       = 164,
  AT_CMD_SATC      = 165,
  AT_CMD_SATE      = 166,
  AT_CMD_SATR      = 167,
  AT_CMD_SATT      = 168,
  AT_CMD_MTST      = 169,
  AT_CMD_SNCNT     = 170,
  AT_CMD_VER       = 171,
  AT_CMD_P_CGCLASS = 172,
  AT_CMD_CGPCO     = 173,
  AT_CMD_CGPPP     = 174,
  AT_CMD_EM        = 175,
  AT_CMD_EMET      = 176,
  AT_CMD_EMETS     = 177,
  AT_CMD_WAP       = 178,
  AT_CMD_CBHZ      = 179,
  AT_CMD_CPHS      = 180,     /* %CPHS   command id */
  AT_CMD_CPNUMS    = 181,     /* %CPNUMS command id */
  AT_CMD_CPALS     = 182,     /* %CPALS  command id */
  AT_CMD_CPVWI     = 183,     /* %CPVWI  voice message waiting command id */
  AT_CMD_CPOPN     = 184,     /* %CPOPN  operator name string command id */
  AT_CMD_CPCFU     = 185,     /* %CPCFU  command id */
  AT_CMD_CPINF     = 186,     /* %CPHS information and customer service profile command id */
  AT_CMD_CPMB      = 187,     /* %CPHS mailbox numbers */
  AT_CMD_CPRI      = 188,
  AT_CMD_DATA      = 189,
  AT_CMD_DINF      = 190,
  AT_CMD_P_CLCC    = 191,
  AT_CMD_P_VST     = 192,
  AT_CMD_CHPL      = 193,
  AT_CMD_CTZR      = 194,
  AT_CMD_VTS       = 195,
  AT_CMD_PVRF      = 196,
  AT_CMD_CWUP      = 197,
  AT_CMD_ABRT      = 198,
  AT_CMD_EXT       = 199,
  AT_CMD_D         = 200,     /* D     command id */
  AT_CMD_O         = 201,     /* O     command id */
  AT_CMD_A         = 202,     /* A     command id */
  AT_CMD_H         = 203,     /* H     command id */
  AT_CMD_Z         = 204,     /* Z     command id */
  AT_CMD_P_CREG    = 205,
  AT_CMD_P_CSQ     = 206,     /* %CSQ  command id */
  AT_CMD_CSIM      = 207,     /* +CSIM command id */
  AT_CMD_ATR       = 208,     /* %ATR  command id */
  AT_CMD_SMBS      = 209,
  AT_CMD_DAR       = 210,     /* %DAR  command id */
  AT_CMD_RDL       = 211,     /* %RDL command id, process redial mode */
#ifdef TI_PS_FF_AT_P_CMD_RDLB
  AT_CMD_RDLB      = 212,     /* %RDLB command id, process black list */
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
  AT_CMD_CIND      = 213,     /* +CIND command id */
  AT_CMD_CMER      = 214,     /* +CMER command id */
  AT_CMD_CSCN      = 215,     /* %CSCN command id */
  AT_CMD_CSTAT     = 216,
  AT_CMD_CPRSM     = 217,     /* %CPRSM command id */
  AT_CMD_P_CHLD    = 218,     /* %CHLD  command id */
  AT_CMD_CTZU      = 219,
  AT_CMD_P_CTZV    = 220,
  AT_CMD_P_CNIV    = 221,
  AT_CMD_P_SECP    = 222,
  AT_CMD_P_SECS = 223,
  AT_CMD_P_CSSN = 224, /* %CSSN command id */
  AT_CMD_CCLK    = 225,
  AT_CMD_CSSD      = 226,     /* %CSSD  command id */
  AT_CMD_P_COPS =227,
  AT_CMD_CPMBW     = 228,     /* %CPMBW command id */
  AT_CMD_CUST       = 229,    /* %CUST command id */
  AT_CMD_SATCC       = 230,    /* %SATCC command id */
  AT_CMD_P_SIMIND  = 231,     /* %SIMIND command id */
  AT_CMD_SIMRST    = 232,     /* State during SIM reset, not related to any AT cmd */
  AT_CMD_P_COPN    = 233,
  AT_CMD_P_CGEREP    = 234,     /* %CGEREP - TI Proprietary CPHS Event Reporting */
#ifdef FF_DUAL_SIM
  AT_CMD_SIM       = 235,
#endif /*FF_DUAL_SIM*/
  AT_CMD_CUSCFG    =  236,     /* %CUSCFG comand id*/
  AT_CMD_CUSDR     =  237,     /* %CUSDR command id */
  AT_CMD_CMMS      =  238,      /* +CMMS command id */
  AT_CMD_STDR      =  239,     /* %CUSDR command id */
  AT_CMD_P_CPBS    =  240,     /* %CPBS comand id*/
  AT_CMD_P_DBGINFO =  241,     /* %DBGINFO command id */
  AT_CMD_CDIP      =  242,
  AT_CMD_P_PBCF    =  243,     /* %PBCF comand id */  
  AT_CMD_SIMEF     =  244,     /* %SIMEF command id */
  AT_CMD_EFRSLT    =  245,     /* %EFRSLT command id */
#ifdef SIM_PERS
  AT_CMD_MEPD      =  246,      /* For %MEPD -for querying ME Personalisation Data... Added on 11/03/2005 */
#endif  
  AT_CMD_P_CMGMDU  =  247,     /* %CMGMDU command id */
  AT_CMD_P_CMGL    =  248,
  AT_CMD_P_CMGR    =  249,
#ifdef FF_CPHS_REL4
  AT_CMD_P_CFIS    =  250,     /* %CFIS command Id  */
  AT_CMD_P_MWIS    =  251,     /* %MWIS command Id  */
  AT_CMD_P_MWI     =  252,     /* %MWI command Id   */
  AT_CMD_P_MBI     =  253,     /* %MBI command Id   */
  AT_CMD_P_MBDN    =  254,     /* %MBDN command Id  */
#endif /* FF_CPHS_REL4 */
#ifdef TI_PS_FF_AT_CMD_P_ECC
  AT_CMD_P_ECC     =  255,     /* %ECC command id */
#endif /* TI_PS_FF_AT_CMD_P_ECC */
#ifdef TI_PS_FF_AT_P_CMD_CTREG
  AT_CMD_CTREG     =  256,
#endif /* TI_PS_FF_AT_P_CMD_CTREG */
#ifdef REL99
  AT_CMD_P_CMGRS   =  257,    /* Message retransmission */
  AT_CMD_CGEQREQ,             /* +CGEQREQ  command id  */
  AT_CMD_CGEQMIN,             /* +CGEQMIN  command id   */
  AT_CMD_CGEQNEG,             /* +CGEQNEG  command id  */
  AT_CMD_CGCMOD,               /* +CGCMOD   command id  */
  AT_CMD_CGDSCONT,           /* +CGDSCONT command id */
  AT_CMD_CGTFT,                  /* +CGTFT    command id     */
#endif
  AT_CMD_P_PBCI,             /* %PBCI    command id     */
  AT_CMD_CVHU,                 /* +CVHU command id   */
  AT_CMD_MAX,                  /* maximum command id */
  AT_CMD_BIGGEST = 0x0000ffff  /* To avoid the lint warning 650 */
} T_ACI_AT_CMD;

typedef enum                /* KSD command identifier */
{
  KSD_CMD_NONE = AT_CMD_MAX, /* no KSD command identifier  */
  KSD_CMD_CB,                /* call barring    command id */
  KSD_CMD_CF,                /* call forwarding command id */
  KSD_CMD_CL,                /* calling line    command id */
  KSD_CMD_CW,                /* call waiting    command id */
  KSD_CMD_PWD,               /* password        command id */
  KSD_CMD_UBLK,              /* unblock PIN     command id */
  KSD_CMD_USSD,              /* unstructured SS command id */
  KSD_CMD_IMEI,              /* get IMEI        command id */
  KSD_CMD_CCBS,              /* CCBS            command id */

  KSD_CMD_MAX                /* maximum command id         */
}
T_ACI_KSD_CMD;

#define CMD_SRC_ATI     1
typedef enum             /* AT command source identifier */
{
  CMD_SRC_NONE = -1,     /* no ACI command source identifier */
  CMD_SRC_LCL,           /* local command source id */
#ifdef FF_ATI
  CMD_SRC_ATI_1,           /* remote command source id */
  CMD_SRC_ATI_2,           /* remote command source id */
#ifndef GOLITE
  CMD_SRC_ATI_3,           /* remote command source id */
  CMD_SRC_ATI_4,           /* remote command source id */
#ifdef SIM_TOOLKIT
  CMD_SRC_ATI_5,           /* added in case of SAT run at cmd
                           THIS DOES NOT MEAN THIS IS THE SAT SOURCE THOUGH !!! */
#endif /* SIM_TOOLKIT */
#if defined FF_EOTD OR defined _SIMULATION_ OR defined CONFIG_AT_RVTMUX
  CMD_SRC_ATI_6,           /* added in case of Location Service (test purposes) */
#endif /* FF_EOTD  OR  _SIMULATION_ */
#endif /* GOLITE */
#endif /* FF_ATI */
  CMD_SRC_MAX            /* maximum command source id */
} T_ACI_CMD_SRC;


typedef enum              /* Command mode */
{
  CMD_MODE_ACI=0,         /* ACI */
  CMD_MODE_ATI,           /* ATI */
#ifdef FF_BAT
  CMD_MODE_BAT,           /* BAT */
#endif
  CMD_MODE_MAX,           /* Number of possible command modes */
  CMD_MODE_NONE           /* 'null' value */
}
T_ACI_CMD_MODE;

typedef enum              /* ACI functional return codes */
{
  AT_FAIL = -1,           /* execution of command failed */
  AT_CMPL,                /* execution of command completed */
  AT_EXCT,                /* execution of command is in progress */
  AT_BUSY                 /* execution of command is rejected due
                             to a busy command handler */
} T_ACI_RETURN;

/* value used internally by ACI */
#define AT_CONT (AT_BUSY+1)

typedef enum              /* ACI error class */
{
  ACI_ERR_CLASS_NotPresent = -1,
  ACI_ERR_CLASS_Cme,      /* +CME  Error codes */
  ACI_ERR_CLASS_Cms,      /* +CMS  Error codes */
  ACI_ERR_CLASS_Ceer,     /* +CEER Error code */
  ACI_ERR_CLASS_Ext       /* extended error codes */

} T_ACI_ERR_CLASS;

typedef ULONG T_ACI_ERR_DESC; /* supplemental error description */


typedef enum                    /* used data coding schemes */
{
  CS_NotPresent = -1,
  CS_GsmDef     =  0,           /* 7 BIT */
  CS_GsmInt,                    /* 7 BIT (8. Bit gesetzt) */
  CS_Hex,                       /* 8 BIT transparent */
  CS_Ucs2,                      /* 16 BIT Unicode */
  CS_Sim,                       /* SIM card format */
  CS_Ascii
}
T_ACI_CS;

typedef struct            /* CRES type defination for restores message service settings */
{
  UBYTE min;
  UBYTE max;
} T_ACI_CRES;

typedef struct            /* data buffer for phonebook alpha */
{
  UBYTE    data[MAX_PHB_NUM_LEN]; /*GW - from MAX_ALPHA_LEN - used to search for a number */
  UBYTE    len;
  T_ACI_CS cs;
}
T_ACI_PB_TEXT;

typedef struct            /* data buffer for Short Messages */
{
  UBYTE data[MAX_SM_LEN];
  UBYTE len;
}
T_ACI_SM_DATA;

typedef struct            /* data buffer for Concatenated Short Messages */
{
  UBYTE  *data;
  USHORT len;
}
T_SM_DATA_EXT;

typedef enum              /* for conc. SMS init functions */
{
  EMPTY = -1,
  CMSS_CONC,
  CMGS_CONC,
  CMGC_CONC,
  CMGR_CONC,
  CMGW_CONC,
  CMGD_CONC
} T_EXT_CMS_CMD_ID;

typedef struct            /* error data Concat. SMS */
{
  T_EXT_CMS_CMD_ID     id;
  union
  {
    struct
    {
      UBYTE segs;
    } errConcCMSS;
    struct
    {
      USHORT sent_chars;
      UBYTE  ref_num;
      UBYTE  next_seg;
      UBYTE  max_num;
    } errConcCMGS;
    struct
    {
      USHORT sent_chars;
      UBYTE  ref_num;
      UBYTE  next_seg;
      UBYTE  max_num;
    } errConcCMGW;
  } specErr;
}
T_EXT_CMS_ERROR;

typedef struct            /* data buffer for Commands */
{
  UBYTE data[MAX_SM_CMD_LEN];
  UBYTE len;
}
T_ACI_CMD_DATA;

typedef struct            /* data buffer for Cell Broadcast Messages */
{
  UBYTE data[MAX_CBM_LEN];
  UBYTE len;
}
T_ACI_CBM_DATA;

typedef struct            /* data buffer for User Data Headers */
{
  UBYTE data[MAX_SM_LEN];
  UBYTE len;
}
T_ACI_UDH_DATA;

typedef struct            /* data buffer for unstructured SS data */
{
  UBYTE data[MAX_USSD_LEN];
  UBYTE len;
}
T_ACI_USSD_DATA;

/*---- +CFUN ------------------------------------------------------*/
typedef enum              /* +CFUN parameter <fun> */
{
  CFUN_FUN_NotPresent = -1,
  CFUN_FUN_Minimum,
  CFUN_FUN_Full,
  CFUN_FUN_Disable_TX_RX_RF = 4
}
T_ACI_CFUN_FUN;

typedef enum              /* +CFUN parameter <rst> */
{
  CFUN_RST_NotPresent = -1,
  CFUN_RST_NoReset,
  CFUN_RST_PreReset
}
T_ACI_CFUN_RST;

/*---- +CME ------------------------------------------------------*/
typedef enum              /* +CME ERROR parameter <err> */
{
  CME_ERR_NotPresent    = -1,
  CME_ERR_PhoneFail,
  CME_ERR_NoConnect,
  CME_ERR_LinkRes,
  CME_ERR_OpNotAllow,
  CME_ERR_OpNotSupp,
  CME_ERR_PhSimPinReq,                   /* PH-SIM PIN required (SIM personalisation) */
  CME_ERR_PhFSimPinReq,                  /* PH-FSIM PIN required (personalisation on first inserted SIM) */
  CME_ERR_PhFSimPukReq,                  /* PH-FSIM PUK required (personalisation on first inserted SIM) */
  CME_ERR_SimNotIns     = 10,
  CME_ERR_SimPinReq,
  CME_ERR_SimPukReq,
  CME_ERR_SimFail,
  CME_ERR_SimBusy,
  CME_ERR_SimWrong,
  CME_ERR_WrongPasswd,
  CME_ERR_SimPin2Req,
  CME_ERR_SimPuk2Req,
  CME_ERR_MemFull       = 20,
  CME_ERR_InvIdx,
  CME_ERR_NotFound,
  CME_ERR_MemFail,
  CME_ERR_TxtToLong,
  CME_ERR_InvalidTxtChar,
  CME_ERR_DialToLong,
  CME_ERR_InvDialChar,
  CME_ERR_NoServ        = 30,
  CME_ERR_Timeout,
  CME_ERR_LimServ,
  CME_ERR_NetworkPersPinReq  = 40,       /* PIN to change network personalisation required */
  CME_ERR_NetworkPersPukReq,             /* network personalisation PUK is required */
  CME_ERR_NetworkSubsetPersPinReq,       /* keycode to change nw subset personalisation required */
  CME_ERR_NetworkSubsetPersPukReq,       /* network subset  personalisation PUK is required */
  CME_ERR_ProviderPersPinReq,            /* keycode to change service provider personal. required */
  CME_ERR_ProviderPersPukReq,            /* service provider personalisation PUK is required */
  CME_ERR_CorporatePersPinReq,           /* keycode to change corporate personalisation required */
  CME_ERR_CorporatePersPukReq,           /* corporate personalisation PUK is required */
  CME_ERR_Busy,
  CME_ERR_Unknown       = 100,

#ifdef GPRS
/* GSM 7:60 10.3.1 error codes */
  CME_ERR_GPRSBadMs       = 103,
  CME_ERR_GPRSBadMe       = 106,
  CME_ERR_GPRSNoService   = 107,
  CME_ERR_GPRSBadPlmn     = 111,
  CME_ERR_GPRSBadLoc      = 112,
  CME_ERR_GPRSNoRoam      = 113,
  CME_ERR_GPRSSerOptNsup  = 132,
  CME_ERR_GPRSSerOptNsub  = 133,
  CME_ERR_GPRSSerOptOOO   = 134,
  CME_ERR_GPRSUnspec      = 148,
  CME_ERR_GPRSPdpAuth     = 149,
  CME_ERR_GPRSBadModClass = 150,
#endif /*GPRS*/

  CME_ERR_FailedToAbort = 512,
  CME_ERR_AcmResetNeeded= 513,
  CME_ERR_SimSatBusy    = 514,           /* Sim is busy with SAT (sw1=0x93), further normal commands allowed */
  CME_ERR_SimNoExtAvail = 515,
  CME_ERR_SimResetNeeded = 516,         /* Reinsertion of SIM, SIM reset required */
  CME_ERR_AbortedByNetwork = 517        /* PLMN Search aborted by Network */ 
}
T_ACI_CME_ERR;

/*---- +CEER -- Standards  Definition-----------------------------------*/
typedef enum              /* +CEER parameter <report> */
{
  CEER_NotPresent             = -1,
  CEER_Unassign               =  1,
  CEER_NoRoute                =  3,
  CEER_ChanUnaccept           =  6,
  CEER_Barred                 =  8,
  CEER_CallClear              = 16,
  CEER_UserBusy,
  CEER_NoResponse,
  CEER_AlertNoAnswer,
  CEER_CallReject             = 21,
  CEER_NumChanged,
  CEER_UserClear              = 26,
  CEER_DestOutOfOrder,
  CEER_NumFormat,
  CEER_FacilityReject,
  CEER_StatusEnquiry,
  CEER_Unspecified,
  CEER_NoChanAvail            = 34,
  CEER_NetworkOutOfOrder      = 38,
  CEER_TempFailure            = 41,
  CEER_SwitchCongest,
  CEER_InfoDiscard,
  CEER_ReqChanUnavail,
  CEER_ResourceUnavail        = 47,
  CEER_QOS_Unavail            = 49,
  CEER_FAC_Unsubscr,
  CEER_BarredInCUG            = 55,
  CEER_BearerCapNotAuth       = 57,
  CEER_BearerCapUnavail,
  CEER_ServUnavail            = 63,
  CEER_BearerNotImpl          = 65,
  CEER_ACM_Max                = 68,
  CEER_FAC_NotImpl,
  CEER_RestrictBearerCap,
  CEER_ServNotImpl            = 79,
  CEER_InvalidTI              = 81,
  CEER_UserNotInCUG           = 87,
  CEER_IncompatDest,
  CEER_InvalidTransNet        = 91,
  CEER_IncorrMsg              = 95,
  CEER_InvalidMandInfo,
  CEER_MsgTypeNotImpl,
  CEER_MsgTypeIncomp,
  CEER_InfoElemNotImpl,
  CEER_CondInfoElem,
  CEER_MsgIncomp,
  CEER_Timer,

#ifdef GPRS
  CEER_FailedGPRSAttach        = 101 ,
  CEER_FailedGPRSContextAct,
  CEER_GPRSDetach,
  CEER_GPRSDeactivation,
#endif /* GPRS */

  CEER_Protocol               = 111, /* These are supposed to be GPRS codes !!! de*/
  CEER_Interworking           = 127, /* These are supposed to be GPRS codes !!! de*/
  CEER_ReqBearerServNotAvail  = 200,
  CEER_NoTransIdAvail,
  CEER_Timer303,
  CEER_EstabFail,
  CEER_NoError                = 210,
  CEER_Failed,
  CEER_Timeout,
  CEER_BearerServNotCompat
}
T_ACI_CEER;

/*---- +CEER --ACI Proprietary Definition-----------------------------------*/
typedef enum              /* +CEER parameter <report> */
{
  P_CEER_NotPresent = CEER_NotPresent ,
  P_CEER_ACMMaxReachedOrExceeded,
  P_CEER_InvalidFDN
}
T_ACI_PCEER;

/*---- +CEER --MM Proprietary Definition-----------------------------------*/
typedef enum              /* +CEER parameter <report> */
{
  P_MM_CEER_NotPresent   = CEER_NotPresent ,
  P_MM_CEER_IMSINotInHLR = 2,    /* IMSI not present in HLR */
  P_MM_CEER_NoService    = 128  /* 0x80 */
}
T_MM_PCEER;

/*---- +CEER --SS Proprietary Definition-----------------------------------*/
typedef enum              /* +CEER parameter <report> */
{
  P_SS_CEER_NotPresent          = CEER_NotPresent ,
  P_SS_CEER_UnknownSubscriber   = 1,
  P_SS_CEER_IllegalSubscriber   = 9,
  P_SS_CEER_BearerSvcNotProv    = 10,
  P_SS_CEER_TeleSvcNotProv      = 11,
  P_SS_CEER_IllegalEquipment    = 12,
  P_SS_CEER_CallBarred          = 13,
  P_SS_CEER_IllegalSSOperation  = 16,
  P_SS_CEER_SSerrStatus         = 17,
  P_SS_CEER_SSNotAvail          = 18,
  P_SS_CEER_SSSubsViolation     = 19,
  P_SS_CEER_SSIncomp            = 20,
  P_SS_CEER_FacNotSupported     = 21,
  P_SS_CEER_AbsentSubs          = 27,
  P_SS_CEER_SystenFail          = 34,
  P_SS_CEER_DataMissing         = 35,
  P_SS_CEER_UnexpectData        = 36,
  P_SS_CEER_PwdRegFail          = 37,
  P_SS_CEER_NegPwdCheck         = 38,
  P_SS_CEER_NumPwdViolation     = 43,
  P_SS_CEER_UnknownAlpha        = 71,
  P_SS_CEER_UssdBusy            = 72,
  P_SS_CEER_MaxNumMptyExceed    = 126,
  P_SS_CEER_ResourceNotAvail    = 127
}
T_SS_PCEER;

typedef enum
{
  P_CEER_mod = CEER_NotPresent,
  P_CEER_sim,     /* +CEER for sim */
  P_CEER_mm,      /* +CEER for mm  */
  P_CEER_ss       /* +CEER for ss  */
} T_PCEER;

/*---- ACI extended error  -------------------------------------*/
typedef enum              /* extended error parameter */
{
  EXT_ERR_NotPresent    = -1,
  EXT_ERR_Parameter,
  EXT_ERR_DataCorrupt,
  EXT_ERR_Internal,
  EXT_ERR_CallTabFull,
  EXT_ERR_SrvTabFull,
  EXT_ERR_CallNotFound,
  EXT_ERR_NoDataCallSup,
  EXT_ERR_OneCallOnHold,
  EXT_ERR_CallTypeNoHold,
  EXT_ERR_FdnCheck,
  EXT_ERR_BdnCheck,
  EXT_ERR_ParallelUSSD,
  EXT_ERR_FaxMinSpeedCond,
  EXT_ERR_CmdDetailsSAT,
  EXT_ERR_AlsLock,
  EXT_ERR_IMEICheck,
#ifdef REL99
  EXT_ERR_FailedMsgNotPresent,
#endif
#if defined FF_EOTD
  EXT_ERR_LCS_CmdNotSup,
  EXT_ERR_LCS_CmdNotRec,
  EXT_ERR_LCS_CLPSClientNotRec,
  EXT_ERR_LCS_IntervalNotSup,
  EXT_ERR_LCS_RepeatNotSup,
  EXT_ERR_LCS_SendReqTyNotRec,
  EXT_ERR_LCS_UsConfReqTyNotRec,
  EXT_ERR_LCS_CLSRClientIdNotRec,
  EXT_ERR_LCS_CSCallNumNotSup,
#endif /* FF_EOTD */

#ifdef SIM_PERS
 EXT_ERR_BlockedNetworkPersPinReq,
 EXT_ERR_BlockedNetworkPersPukReq,
 EXT_ERR_Busy,
 EXT_ERR_NoMEPD,
#endif
  EXT_ERR_Unknown       = 100

}
T_ACI_EXT_ERR;

/*---- Class of Service--------------------------------------------*/
typedef enum              /* service class */
{
  CLASS_NotPresent      = -1,
  CLASS_None,
  CLASS_Vce,
  CLASS_Dat,
  CLASS_VceDat,
  CLASS_Fax,
  CLASS_VceFax,
  CLASS_DatFax,
  CLASS_VceDatFax,
  CLASS_Sms,
  CLASS_VceSms,
  CLASS_DatSms,
  CLASS_VceDatSms,
  CLASS_FaxSms,
  CLASS_VceFaxSms,
  CLASS_DatFaxSms,
  CLASS_VceDatFaxSms,
  CLASS_DatCirSync,
  CLASS_DatCirAsync     = 32,
  CLASS_DedPacAccess    = 64,
  CLASS_AllSync         = 80,
  CLASS_AllAsync        = 160,
  CLASS_DedPADAcess     = 128,
  CLASS_AuxVce          = 256
} T_ACI_CLASS;

/*---- Type of address --------------------------------------------*/
typedef enum              /* numbering plan identifier */
{
  NPI_NotPresent    =-1,
  NPI_Unknown       = 0,
  NPI_IsdnTelephony = 1,
  NPI_Data          = 3,
  NPI_Telex         = 4,
  NPI_National      = 8,
  NPI_Private       = 9,
  NPI_ERMES         = 10,  /* ffs */
  NPI_CTS           = 11   /* ffs */
}
T_ACI_TOA_NPI;

typedef enum               /* type of number */
{
  TON_NotPresent    =-1,
  TON_Unknown       = 0,
  TON_International,
  TON_National,
  TON_NetSpecific,
  TON_DedAccess,
  TON_Alphanumeric,
  TON_Abbreviated,
  TON_Extended             /* ffs */
}
T_ACI_TOA_TON;

typedef struct             /* type of address octet */
{
  T_ACI_TOA_TON ton;
  T_ACI_TOA_NPI npi;
}
T_ACI_TOA;

/*---- Type of subaddress -----------------------------------------*/
typedef enum               /* type of subaddress */
{
  TOS_NotPresent  =-1,
  TOS_Nsap        = 0,
  TOS_User        = 2
}
T_ACI_TOS_TOS;

typedef enum               /* odd/even indicator */
{
  OE_NotPresent   =-1,
  OE_Even         = 0,
  OE_Odd          = 1
}
T_ACI_TOS_OE;

typedef struct             /* type of address octet */
{
  T_ACI_TOS_TOS tos;
  T_ACI_TOS_OE  oe;
}
T_ACI_TOS;

/*---- alerting pattern ------------------------------------------*/
typedef enum               /* alerting pattern */
{
  ALPT_NotPresent = -1,
  ALPT_Level_0,
  ALPT_Level_1,
  ALPT_Level_2,
  ALPT_Ctgry_1 = 4,
  ALPT_Ctgry_2,
  ALPT_Ctgry_3,
  ALPT_Ctgry_4,
  ALPT_Ctgry_5
}
T_ACI_ALRT_PTRN;

/*---- +CPIN ------------------------------------------------------*/
typedef enum              /* +CPIN response parameter <rslt> */
{
  CPIN_RSLT_NotPresent = -1,
  CPIN_RSLT_SimReady,
  CPIN_RSLT_SimPinReq,
  CPIN_RSLT_SimPukReq,
  CPIN_RSLT_PhSimPinReq,
  CPIN_RSLT_SimPin2Req,
  CPIN_RSLT_SimPuk2Req, 
  /* OVK: Extended list of all possible result according to 07.07 */
  CPIN_RSLT_PhFSimPinReq,
  CPIN_RSLT_PhFSimPukReq,
  CPIN_RSLT_PhNetPinReq,
  CPIN_RSLT_PhNetPukReq,
  CPIN_RSLT_PhNetSubPinReq,
  CPIN_RSLT_PhNetSubPukReq,
  CPIN_RSLT_PhSPPinReq,
  CPIN_RSLT_PhSPPukReq,
  CPIN_RSLT_PhCorpPinReq,
  CPIN_RSLT_PhCorpPukReq,
  CPIN_RSLT_PhSimFail,
  CPIN_RSLT_PhBlockedNetPinReq,
  CPIN_RSLT_PhBlockedNetPukReq    
}
T_ACI_CPIN_RSLT;

/*---- +COPS ------------------------------------------------------*/

typedef enum              /* +COPS parameter <mode> */
{
  COPS_MOD_NotPresent = -1,
  COPS_MOD_Auto,
  COPS_MOD_Man,
  COPS_MOD_Dereg,
  COPS_MOD_SetOnly,
  COPS_MOD_Both
}
T_ACI_COPS_MOD;

typedef enum              /* +COPS parameter <format> */
{
  COPS_FRMT_NotPresent = -1,
  COPS_FRMT_Long,
  COPS_FRMT_Short,
  COPS_FRMT_Numeric
}
T_ACI_COPS_FRMT;

typedef enum              /* +COPS parameter <stat> */
{
  COPS_STAT_NotPresent = -1,
  COPS_STAT_Unknown,
  COPS_STAT_Available,
  COPS_STAT_Current,
  COPS_STAT_Forbidden
} T_ACI_COPS_STAT;

typedef enum               /* %COPS parameter <srvStatus> */
{
  COPS_SVST_NotPresent = -1,
  COPS_SVST_Full,
  COPS_SVST_Limited,
  COPS_SVST_NoSrv,
  COPS_SVST_SetRegModeOnly
}
T_ACI_COPS_SVST;


typedef struct            /* +COPS operator list element*/
{
  T_ACI_COPS_STAT status;
  CHAR  *         longOper;
  CHAR  *         shortOper;
  CHAR            numOper[MAX_NUM_OPER_LEN];
  UBYTE pnn;  /* PLMN Network Name Source (for EONS) */
  UBYTE long_len;
  UBYTE long_ext_dcs;
  UBYTE shrt_len;
  UBYTE shrt_ext_dcs;
}T_ACI_COPS_OPDESC;

typedef struct
{
    char longName[MAX_ALPHA_OPER_LEN];
    char shrtName[MAX_ALPHA_OPER_LEN];
    SHORT      mcc;
    SHORT      mnc;
    UBYTE      pnn;  /* PLMN Network Name Source (for EONS) */
    UBYTE      long_len;
    UBYTE      shrt_len;
    UBYTE      source;
} T_ACI_OPER_NTRY;


typedef T_ACI_COPS_OPDESC T_ACI_COPS_LST [MAX_OPER];

/*---- +CPOL ------------------------------------------------------*/

typedef enum              /* +CPOL parameter <format> */
{
  CPOL_FRMT_NotPresent = -1,
  CPOL_FRMT_Long,
  CPOL_FRMT_Short,
  CPOL_FRMT_Numeric
}
T_ACI_CPOL_FRMT;

typedef enum              /* +CPOL parameter <mode> */
{
  CPOL_MOD_NotPresent = -1,
  CPOL_MOD_CompactList,
  CPOL_MOD_Insert
}
T_ACI_CPOL_MOD;

typedef struct            /* +CPOL preferred operator list element */
{
  SHORT           index;
  T_ACI_CPOL_FRMT format;
  CHAR            oper[MAX_ALPHA_OPER_LEN];
}T_ACI_CPOL_OPDESC;

typedef T_ACI_CPOL_OPDESC T_ACI_CPOL_LST [MAX_OPER];

typedef enum              /* Language codes */
{
  CLAN_LNG_AUT = -1,           /*Automatic*/
  CLAN_LNG_ENG,                /*English*/
  CLAN_LNG_FRE,                /*French*/
  CLAN_LNG_GER,                /*German*/
  CLAN_LNG_DUT,                /*Dutch*/
  CLAN_LNG_ITA,
  CLAN_LNG_SPA,
  CLAN_LNG_SWE,
  CLAN_LNG_POR,
  CLAN_LNG_FIN,
  CLAN_LNG_NOR,
  CLAN_LNG_GRE,
  CLAN_LNG_TUR,
  CLAN_LNG_HUN,
  CLAN_LNG_SLO,
  CLAN_LNG_POL,
  CLAN_LNG_RUS,
  CLAN_LNG_IND,
  CLAN_LNG_CZE,
  CLAN_LNG_CHI,
  CLAN_LNG_CAN,
  CLAN_LNG_MAN,
  CLAN_LNG_TAI,
  CLAN_LNG_ARA
}
T_ACI_CLAN_LNG;

typedef struct
{
  CHAR           *str;
  T_ACI_CLAN_LNG lng;
}
T_ACI_LAN_SUP;

/*---- +CREG ------------------------------------------------------*/
typedef enum              /* +CREG parameter <stat> */
{
  CREG_STAT_NotPresent = -1,
  CREG_STAT_NoSearch,
  CREG_STAT_Reg,
  CREG_STAT_Search,
  CREG_STAT_Denied,
  CREG_STAT_Unknown,
  CREG_STAT_Roam
}
T_ACI_CREG_STAT;

/*---- %CREG ------------------------------------------------------*/
typedef enum              /* %CREG parameter <gprs_ind> */
{
  P_CREG_GPRS_Not_Supported = 0,
  P_CREG_GPRS_Supported_Limited_Serv,
  P_CREG_GPRS_Supported,
  P_CREG_GPRS_Support_Unknown
}
T_ACI_P_CREG_GPRS_IND;

/*---- D ---------------------------------------------------------*/
typedef enum              /* D parameter <clirOvrd> */
{
  D_CLIR_OVRD_Default = -1,
  D_CLIR_OVRD_Supp,
  D_CLIR_OVRD_Invoc
}
T_ACI_D_CLIR_OVRD;

typedef enum              /* D parameter <cugCtrl> */
{
  D_CUG_CTRL_NotPresent = -1,
  D_CUG_CTRL_Present
}
T_ACI_D_CUG_CTRL;

typedef enum              /* D parameter <callType> */
{
  D_TOC_Data = -1,
  D_TOC_Voice
}
T_ACI_D_TOC;

#ifdef SIM_TOOLKIT
typedef enum              /* D parameter <simCallControl> */
{
  D_SIMCC_NOT_ACTIVE = 0,
  D_SIMCC_ACTIVE,
  D_SIMCC_ACTIVE_CHECK
}
T_ACI_D_SIMCC;
#endif /* SIM_TOOLKIT */

/*---- +CLIR ------------------------------------------------------*/
typedef enum              /* +CLIR parameter <mode> */
{
  CLIR_MOD_NotPresent = -1,
  CLIR_MOD_Subscript,
  CLIR_MOD_Invoc,
  CLIR_MOD_Supp
}
T_ACI_CLIR_MOD;

typedef enum              /* +CLIR parameter <stat> */
{
  CLIR_STAT_NotPresent = -1,
  CLIR_STAT_NotProv,
  CLIR_STAT_Permanent,
  CLIR_STAT_Unknown,
  CLIR_STAT_RestrictTemp,
  CLIR_STAT_AllowTemp
}
T_ACI_CLIR_STAT;

/*---- +CLIP ------------------------------------------------------*/

typedef enum              /* +CLIP parameter <stat> */
{
  CLIP_STAT_NotPresent = -1,
  CLIP_STAT_NotProv,
  CLIP_STAT_Prov,
  CLIP_STAT_Unknown
}
T_ACI_CLIP_STAT;

/*---- +CDIP ------------------------------------------------------*/

typedef enum              /* +CDIP parameter <stat> */
{
  CDIP_STAT_NotPresent = -1,
  CDIP_STAT_NotProv,
  CDIP_STAT_Prov,
  CDIP_STAT_Unknown
}
T_ACI_CDIP_STAT;

/*---- +COLP ------------------------------------------------------*/

typedef enum              /* +COLP parameter <stat> */
{
  COLP_STAT_NotPresent = -1,
  COLP_STAT_NotProv,
  COLP_STAT_Prov,
  COLP_STAT_Unknown
}
T_ACI_COLP_STAT;

/*---- %CTTY ------------------------------------------------------*/

typedef enum              /* %CTTY parameter (mode) */
{
  CTTY_MOD_NotPresent = -1,
  CTTY_MOD_Disable,
  CTTY_MOD_Enable
}
T_ACI_CTTY_MOD;

typedef enum              /* %CTTY parameter (request) */
{
  CTTY_REQ_NotPresent = -1,
  CTTY_REQ_Off,
  CTTY_REQ_On,
  CTTY_REQ_HCO,
  CTTY_REQ_VCO
}
T_ACI_CTTY_REQ;

typedef enum              /* %CTTY parameter (negociation) */
{
  CTTY_NEG_None = 0,
  CTTY_NEG_Request,
  CTTY_NEG_Reject,
  CTTY_NEG_Grant
}
T_ACI_CTTY_NEG;

typedef enum              /* %CTTY parameter (activity) */
{
  CTTY_TRX_Off = 0,
  CTTY_TRX_RcvOn,
  CTTY_TRX_SendOn,
  CTTY_TRX_RcvSendOn,
  CTTY_TRX_Unknown
}
T_ACI_CTTY_TRX;

typedef enum              /* %CTTY parameter (state) */
{
  CTTY_STAT_Off = 0,
  CTTY_STAT_On,
  CTTY_STAT_Unknown
}
T_ACI_CTTY_STAT;

/*---- +CSVM ------------------------------------------------------*/

typedef enum              /* +CSVM parameter <mode> */
{
  CSVM_MOD_NotPresent = -1,
  CSVM_MOD_Disable,
  CSVM_MOD_Enable
}
T_ACI_CSVM_MOD;

/*---- +CMOD ------------------------------------------------------*/
typedef enum              /* +CMOD parameter <mode> */
{
  CMOD_MOD_NotPresent = -1,
  CMOD_MOD_Single,
  CMOD_MOD_VoiceFax,
  CMOD_MOD_VoiceDat,
  CMOD_MOD_VoiceFlwdDat
}
T_ACI_CMOD_MOD;

/*---- +CBST ------------------------------------------------------*/
typedef enum              /* +CBST parameter <speed> */
{
  BS_SPEED_NotPresent = -1,

  BS_SPEED_AUTO,
  BS_SPEED_300_V21,
  BS_SPEED_1200_V22,
  BS_SPEED_1200_75_V23,
  BS_SPEED_2400_V22bis,
  BS_SPEED_2400_V26ter,
  BS_SPEED_4800_V32,
  BS_SPEED_9600_V32,
  BS_SPEED_9600_V34    = 12,
  BS_SPEED_14400_V34   = 14,
  BS_SPEED_1200_V120   = 34,
  BS_SPEED_2400_V120   = 36,
  BS_SPEED_4800_V120   = 38,
  BS_SPEED_9600_V120,
  BS_SPEED_14400_V120  = 43,
  BS_SPEED_300_V110    = 65,
  BS_SPEED_1200_V110,
  BS_SPEED_2400_V110   = 68,
  BS_SPEED_4800_V110   = 70,
  BS_SPEED_9600_V110,
  BS_SPEED_14400_V110  = 75,
  BS_SPEED_19200_V110  = 79,
  BS_SPEED_28800_V110  = 80,
  BS_SPEED_38400_V110
}
T_ACI_BS_SPEED;

typedef enum              /* +CBST parameter <name> */
{
  CBST_NAM_NotPresent = -1,
  CBST_NAM_Asynch,
  CBST_NAM_Synch
}
T_ACI_CBST_NAM;

typedef enum              /* +CBST parameter <ce> */
{
  CBST_CE_NotPresent = -1,
  CBST_CE_Transparent,
  CBST_CE_NonTransparent,
  CBST_CE_BothTransPref,
  CBST_CE_BothNonTransPref
}
T_ACI_CBST_CE;


/*---- +DS --------------------------------------------------------*/
typedef enum              /* +DS parameter <dir> */
{
  DS_DIR_NotPresent = -1,
  DS_DIR_Negotiated,
  DS_DIR_TxOnly,
  DS_DIR_RxOnly,
  DS_DIR_Both
}
T_ACI_DS_DIR;

typedef enum              /* +DS parameter <comp> */
{
  DS_COMP_NotPresent = -1,
  DS_COMP_DoNotDisc,
  DS_COMP_Disc
}
T_ACI_DS_COMP;

/*---- +DR --------------------------------------------------------*/
typedef enum              /* +DR parameter <type> */
{
  DR_TYP_NotPresent = -1,
  DR_TYP_None,
  DR_TYP_TxOnly,
  DR_TYP_RxOnly,
  DR_TYP_Both
}
T_ACI_DR_TYP;

/*---- +CRING -----------------------------------------------------*/
typedef enum              /* +SERVICE,+CRING parameter <type> */
{
  CRING_SERV_TYP_NotPresent = -1,
  CRING_SERV_TYP_Async,
  CRING_SERV_TYP_Sync,
  CRING_SERV_TYP_RelAsync,
  CRING_SERV_TYP_RelSync,
#ifdef GPRS
 CRING_SERV_TYP_GPRS,
#endif  /* GPRS */ 
  CRING_SERV_TYP_Fax,
  CRING_SERV_TYP_Voice,
  CRING_SERV_TYP_AuxVoice
}
T_ACI_CRING_SERV_TYP;

typedef enum              /* +CRING parameter <mode> */
{
  CRING_MOD_NotPresent = -1,
  CRING_MOD_Direct,
  CRING_MOD_Alternate

#ifdef GPRS
 ,CRING_MOD_Gprs
#endif  /* GPRS */

}
T_ACI_CRING_MOD;

/*---- +CCWA -----------------------------------------------------*/
typedef enum              /* +CCWA parameter <mode> */
{
  CCWA_MOD_NotInterrogate = -1,
  CCWA_MOD_Disable,
  CCWA_MOD_Enable,
  CCWA_MOD_Query
}
T_ACI_CCWA_MOD;

/*---- +CPWD ,+CLCK-----------------------------------------------------*/
typedef enum              /* +CLCK,+CPWD parameter <fac> */
{
  FAC_NotPresent     = -1,
  FAC_Sc=0,
  FAC_Ao=1,
  FAC_Oi=2,
  FAC_Ox=3,
  FAC_Ai=4,
  FAC_Ir=5,
  FAC_Ab=6,
  FAC_Ag=7,
  FAC_Ac=8,
  FAC_Fd=9,
  FAC_Pn=10,              /* Network personalisation */
  FAC_Pu=11,              /* Network subset personalisation */
  FAC_Pp=12,              /* Service provider personalisation */
  FAC_Pc=13,              /* Corporate personalisation */
  FAC_Ps=14,              /* SIM personalisation */
  FAC_Pf=15,              /* Personalisation on first inserted SIM */
  FAC_Al =16              /* ALS settings locked by CHV2 */
  #ifdef SIM_PERS
  ,FAC_Bl =17,
  FAC_Fc=18,
  FAC_Fcm=19,
  FAC_Mu=20,              /*For Master Unlock thru bootup */
  FAC_Mum=21              /*For Master Unlock thru Menu */
  #endif
  #ifdef FF_PHONE_LOCK
  ,FAC_Pl=22,
   FAC_Apl=23
  #endif
  ,FAC_P2=24
  
}
T_ACI_FAC;

typedef struct
{
  T_ACI_FAC    fac;
  SHORT             pwdlength;
}
T_ACI_CPWD_LEN;

typedef T_ACI_CPWD_LEN T_ACI_CPWD_LST [MAX_FACILITY];

typedef enum              /* +CLCK parameter <mode> */
{
  CLCK_MOD_NotPresent     = -1,
  CLCK_MOD_Unlock,
  CLCK_MOD_Lock,
  CLCK_MODE_QUERY
}
T_ACI_CLCK_MOD;

#ifdef SIM_PERS

/*Would be used by %MEPD AT Command 
to store supplementary info type 
Added on 11/03/2005*/
typedef enum
{
  CMEPD_SUP_INFO_NotPresent = -1,
  FCMAX,   /*CFG field== Failure Counter Max Value */
  FCATTEMPTSLEFT,  /* CFG field== Failure Counter Current Value */
  FCRESETFAILMAX,
  FCRESETFAILATTEMPTSLEFT,
  FCRESETSUCCESSMAX,
  FCRESETSUCCESSATTEMPTSLEFT,
  TIMERFLAG,
  ETSIFLAG,
  AIRTELINDFLAG
} T_SUP_INFO_TYPE;

/*Would be used by %MEPD AT Command 
to display supplementary data value
Added on 11/03/2005*/
typedef struct
{
  T_SUP_INFO_TYPE  infoType; 
  UBYTE datavalue;      /*Value of CFG Data */
} T_SUP_INFO;

#endif

typedef enum
{
  STATUS_NotPresent       = -1,
  STATUS_NotActive,
  STATUS_Active
}T_ACI_STATUS;

typedef enum                /* %CCBS parameter <mode> */
{
  CCBS_MOD_NotPresent       = -1,
  CCBS_MOD_Disable,
  CCBS_MOD_Enable,
  CCBS_MOD_Query,
  CCBS_MOD_Register = 3,
  CCBS_MOD_Erasure
} T_ACI_CCBS_MOD;


/*---- +CCFC -----------------------------------------------------*/
typedef enum                /* +CCFC parameter <mode> */
{
  CCFC_MOD_NotPresent       = -1,
  CCFC_MOD_Disable,
  CCFC_MOD_Enable,
  CCFC_MOD_Query,
  CCFC_MOD_Register = 3,
  CCFC_MOD_Erasure
} T_ACI_CCFC_MOD;

typedef enum                /* +CCFC parameter <reason> */
{
  CCFC_RSN_NotPresent       = -1,
  CCFC_RSN_Uncond,
  CCFC_RSN_Busy,
  CCFC_RSN_NoReply,
  CCFC_RSN_NotReach,
  CCFC_RSN_Forward,
  CCFC_RSN_CondForward
} T_ACI_CCFC_RSN;

typedef struct
{
  T_ACI_STATUS  status;
  T_ACI_CLASS   class_type;
}
T_ACI_CLSSTAT;

typedef struct
{
  T_ACI_CLSSTAT  clsstat;
  CHAR           number[MAX_B_SUBSCR_NUM_LEN];
  T_ACI_TOA      type;
  CHAR           subaddr[MAX_SUBADDR_LEN];
  T_ACI_TOS      satype;
  SHORT          time;
}
T_ACI_CCFC_SET;

/*--- +CCUG -------------------------------------------------------*/
typedef enum
{
  CCUG_IDX_NotPresent   = -1,
  CCUG_IDX_0,
  CCUG_IDX_1,
  CCUG_IDX_2,
  CCUG_IDX_3,
  CCUG_IDX_4,
  CCUG_IDX_5,
  CCUG_IDX_6,
  CCUG_IDX_7,
  CCUG_IDX_8,
  CCUG_IDX_9,
  CCUG_IDX_No
}
T_ACI_CCUG_IDX;

typedef enum
{
  CCUG_INFO_NotPresent  = -1,
  CCUG_INFO_No,
  CCUG_INFO_SuppOa,
  CCUG_INFO_SuppPrefCug,
  CCUG_INFO_SuppBoth
}
T_ACI_CCUG_INFO;

typedef enum
{
  CCUG_MOD_NotPresent   = -1,
  CCUG_MOD_DisableTmp,
  CCUG_MOD_EnableTmp
}
T_ACI_CCUG_MOD;

/*--- +CMGF--------------------------------------------------------*/

typedef enum
{
  CMGF_MOD_NotPresent   = -1,
  CMGF_MOD_Pdu,
  CMGF_MOD_Txt
}
T_ACI_CMGF_MOD;

/*--- %SMBS--------------------------------------------------------*/

typedef enum
{
  PERC_SMBS_MOD_NotPresent    = -1,
  PERC_SMBS_MOD_DISABLE        = 0,
  PERC_SMBS_MOD_ENABLE         = 1
}
T_ACI_PERC_SMBS_MOD;

/*--- CMS Err------------------------------------------------------*/

typedef enum
{
  CMS_ERR_NotPresent       = -1,/*---From GSM 0411 E2---*/
  CMS_ERR_UnAllocNum       = 1,
  CMS_ERR_OpDetermBarr     = 8,
  CMS_ERR_CallBarr         = 10,
  CMS_ERR_TransReject      = 21,
  CMS_ERR_DestOutOfServ    = 27,
  CMS_ERR_UnidentSubsc,
  CMS_ERR_FacReject,
  CMS_ERR_UnKnownSubsc,
  CMS_ERR_NetOutOfOrder    = 38,
  CMS_ERR_TempFail         = 41,
  CMS_ERR_Congestion,
  CMS_ERR_ResUnAvail       = 47,
  CMS_ERR_FacNotSubscr     = 50,
  CMS_ERR_FacNotImpl       = 69,
  CMS_ERR_TransRefInval    = 81,
  CMS_ERR_InValSM          = 95,
  CMS_ERR_InValManInfo,
  CMS_ERR_MsgTypNotExist,
  CMS_ERR_MsgNotCompatible,
  CMS_ERR_InfoElemNotImpl,
  CMS_ERR_ProtErr          = 111,
  CMS_ERR_InterWrkUnSpec   = 127,
  CMS_ERR_TlmtkNotSup,        /*---From GSM 0340 9.2.3.22---*/
  CMS_ERR_SM0NotSup,
  CMS_ERR_CantReplceSM,
  CMS_ERR_UnSpecPIDErr     = 143,
  CMS_ERR_DcsNotSup,
  CMS_ERR_MsgClassNotSup,
  CMS_ERR_UnSpecTpDcs      = 159,
  CMS_ERR_CmdNotAct,
  CMS_ERR_CmdUnSup,
  CMS_ERR_UnSpecTpCmd      = 175,
  CMS_ERR_TpduUnSup,
  CMS_ERR_ScBsy            = 192,
  CMS_ERR_NoScSubsc,
  CMS_ERR_ScSysFail,
  CMS_ERR_InValSme,
  CMS_ERR_DestSmeBarr,
  CMS_ERR_SmRejctDuplSm,
  CMS_ERR_SmTPVPFNotSup,
  CMS_ERR_SmTPVPNotSup,
  CMS_ERR_SimSmsStorFull   = 208,
  CMS_ERR_NoStorInSim,
  CMS_ERR_ErrInMs,
  CMS_ERR_MemCabExcee,
  CMS_ERR_UnSpecErr        = 255,
  CMS_ERR_MeFail           = 300, /*---From GSM 0705 3.2.5---*/
  CMS_ERR_ServRes,
  CMS_ERR_OpNotAllowed,
  CMS_ERR_OpNotSup,
  CMS_ERR_InValPduMod,
  CMS_ERR_InValTxtMod,
  CMS_ERR_SimNotIns        = 310,
  CMS_ERR_SimPinReq,
  CMS_ERR_PhSimPinReq,
  CMS_ERR_SimFail,
  CMS_ERR_SimBsy,
  CMS_ERR_SimWrong,
  CMS_ERR_SimPukReq,
  CMS_ERR_SimPin2Req,
  CMS_ERR_SimPuk2Req,
  CMS_ERR_MemFail          = 320,
  CMS_ERR_InValMemIdx,
  CMS_ERR_MemFull,
  CMS_ERR_SmscAdrUnKnown   = 330,
  CMS_ERR_NoNetServ,
  CMS_ERR_NetTimeOut,
  CMS_ERR_NoCnmaAckExpect  = 340,
  CMS_ERR_UnknownErr       = 500,
  CMS_ERR_FailedToAbort    = 512,
  CMS_ERR_AcmResetNeeded   = 513
}
T_ACI_CMS_ERR;

/*--- +CNMI ------------------------------------------------------*/

typedef enum
{
  CNMI_MT_NotPresent    = -1,
  CNMI_MT_NoSmsDeliverInd,
  CNMI_MT_SmsDeliverInd,
  CNMI_MT_SmsDeliver,
  CNMI_MT_SmsDeliverCls3
}
T_ACI_CNMI_MT;

typedef enum
{
  CNMI_BM_NotPresent    = -1,
  CNMI_BM_NoCbmInd,
  CNMI_BM_CbmInd,
  CNMI_BM_Cbm,
  CNMI_BM_CbmCls3
}
T_ACI_CNMI_BM;

typedef enum
{
  CNMI_DS_NotPresent    = -1,
  CNMI_DS_NoSmsStatRpt,
  CNMI_DS_SmsStatRpt
}
T_ACI_CNMI_DS;

/*--- +CNUM -------------------------------------------------------*/

typedef enum
{
  CNUM_ITC_NotPresent   = -1,
  CNUM_ITC_3_1_kHz,
  CNUM_ITC_Udi
}
T_ACI_CNUM_ITC;

typedef enum
{
  CNUM_SERV_NotPresent  = -1,
  CNUM_SERV_Asynch,
  CNUM_SERV_Synch,
  CNUM_SERV_PadAsynch,
  CNUM_SERV_PacketSynch,
  CNUM_SERV_Voice,
  CNUM_SERV_Fax
}
T_ACI_CNUM_SERV;

typedef enum
{
  CNUM_MOD_NewRead      =   0,
  CNUM_MOD_NextRead
}
T_ACI_CNUM_MOD;

typedef struct
{
  BOOL              vldFlag;
  CHAR              alpha[MAX_ALPHA_LEN];
  CHAR              number[MAX_PHB_NUM_LEN];
  T_ACI_TOA         type;
  T_ACI_BS_SPEED    speed;
  T_ACI_CNUM_SERV   service;
  T_ACI_CNUM_ITC    itc;
}
T_ACI_CNUM_MSISDN;

typedef T_ACI_CNUM_MSISDN T_ACI_CNUM_LST [MAX_MSISDN];

/*--- +CPAS -------------------------------------------------------*/

typedef enum
{
  CPAS_PAS_NotPresent   = -1,
  CPAS_PAS_Ready,
  CPAS_PAS_Unavailable,
  CPAS_PAS_Unknown,
  CPAS_PAS_Ring,
  CPAS_PAS_CallProg,
  CPAS_PAS_Asleep
}
T_ACI_CPAS_PAS;

/*--- +CLAE -------------------------------------------------------*/
typedef enum
{
  CLAE_MOD_NotPresent    = -1,
  CLAE_MOD_Disable,
  CLAE_MOD_Enable
}
T_ACI_CLAE_MOD;

/*--- +CSCB -------------------------------------------------------*/

typedef enum
{
  CSCB_MOD_NotPresent   = -1,
  CSCB_MOD_Accept,
  CSCB_MOD_NotAccept
}
T_ACI_CSCB_MOD;

/*--- +CBHZ -------------------------------------------------------*/
#ifdef FF_HOMEZONE
typedef enum
{
  CBHZ_MOD_NotPresent   = -1,
  CBHZ_MOD_NotActive,
  CBHZ_MOD_Active
}
T_ACI_CBHZ_MOD;
#endif /* FF_HOMEZONE */

/*--- +CSDH -------------------------------------------------------*/

typedef enum
{
  CSDH_SHOW_NotPresent    = -1,
  CSDH_SHOW_Disable,
  CSDH_SHOW_Enable
}
T_ACI_CSDH_SHOW;

/*--- +CSMS -------------------------------------------------------*/

typedef enum
{
  CSMS_SERV_NotPresent    = -1,
  CSMS_SERV_GsmPh2,
  CSMS_SERV_GsmPh2Plus
}
T_ACI_CSMS_SERV;

typedef enum
{
  CSMS_SUPP_NotPresent    = -1,
  CSMS_SUPP_Disable,
  CSMS_SUPP_Enable
}
T_ACI_CSMS_SUPP;

/*--- +CUSD command -----------------------------------------------*/

typedef enum
{
  CUSD_MOD_NotPresent       = -1,
  CUSD_MOD_NoActReq,
  CUSD_MOD_YesActReq,
  CUSD_MOD_TerminatedByNetwork,
  CUSD_MOD_OtherLocalClientResp,
  CUSD_MOD_OperationNotSupported,
  CUSD_MOD_NetworkTimeout
}
T_ACI_CUSD_MOD;

/*--- +CSSN command -----------------------------------------------*/

typedef enum
{
  CSSI_CODE_NotPresent       = -1,
  CSSI_CODE_CFUActive,
  CSSI_CODE_SomeCCFActive,
  CSSI_CODE_ForwardedCall,
  CSSI_CODE_CallWaiting,
  CSSI_CODE_CUGCall,
  CSSI_CODE_OutCallsBarred,
  CSSI_CODE_IncCallsBarred,
  CSSI_CODE_CLIRSupRej,
  CSSI_CODE_DeflectedCall,
  CSSI_CODE_Biggest 		   = 255
}
T_ACI_CSSI_CODE;

typedef enum
{
  CSSU_CODE_NotPresent       = -1,
  CSSU_CODE_ForwardedCall,
  CSSU_CODE_CUGCall,
  CSSU_CODE_OnHold,
  CSSU_CODE_Retrieved,
  CSSU_CODE_Multiparty,
  CSSU_CODE_HeldCallRel,
  CSSU_CODE_FwrdCheckSS,
  CSSU_CODE_ECTAlert,
  CSSU_CODE_ECTConnect,
  CSSU_CODE_DeflectedCall,
  CSSU_CODE_IncCallForwarded
}
T_ACI_CSSU_CODE;


typedef enum
{
  CSSX_CODE_NotPresent       = -1,
  CSSX_CODE_ForwardedCall,
  CSSX_CODE_CUGCall,
  CSSX_CODE_OnHold,
  CSSX_CODE_Retrieved,
  CSSX_CODE_Multiparty,
  CSSX_CODE_HeldCallRel,
  CSSX_CODE_FwrdCheckSS,
  CSSX_CODE_ECTAlert,
  CSSX_CODE_ECTConnect,
  CSSX_CODE_CFUActive,
  CSSX_CODE_SomeCCFActive,
  CSSX_CODE_CallWaiting,
  CSSX_CODE_OutCallsBarred,
  CSSX_CODE_IncCallsBarred,
  CSSX_CODE_CLIRSupRej,
  CSSX_CODE_DeflectedCall,
  CSSX_CODE_IncCallForwarded

}
T_ACI_CSSX_CODE;


/*--- %CLCC command -----------------------------------------------*/

typedef enum
{
  P_CLCC_DIR_NotPresent       = -1,
  P_CLCC_DIR_MOC,
  P_CLCC_DIR_MTC,
  P_CLCC_DIR_MOC_NI, 
  P_CLCC_DIR_MOC_RDL 
}
T_ACI_P_CLCC_DIR;

/*--- %DBGINFO command --------------------------------------------*/

typedef enum
{
  P_DBGINFO_NotPresent          = 0,
  P_DBGINFO_PrimPoolPartition,
  P_DBGINFO_DmemPoolPartition,
  P_DBGINFO_DataPoolPartition
}
T_ACI_DBG_INFO;

/*--- +CLCC command -----------------------------------------------*/

typedef enum
{
  CLCC_DIR_NotPresent       = -1,
  CLCC_DIR_MOC,
  CLCC_DIR_MTC,
  CLCC_DIR_MOC_NI, 
  CLCC_DIR_MOC_RDL 
}
T_ACI_CLCC_DIR;

typedef enum
{
  CLCC_STAT_NotPresent       = -1,
  CLCC_STAT_Active,
  CLCC_STAT_Held,
  CLCC_STAT_Dialing,
  CLCC_STAT_Alerting,
  CLCC_STAT_Incoming,
  CLCC_STAT_Waiting
}
T_ACI_CLCC_STAT;

typedef enum
{
  CLCC_MODE_NotPresent       = -1,
  CLCC_MODE_Voice,
  CLCC_MODE_Data,
  CLCC_MODE_Fax,
  CLCC_MODE_VFDVoice,
  CLCC_MODE_VADVoice,
  CLCC_MODE_VAFVoice,
  CLCC_MODE_VFDData,
  CLCC_MODE_VADData,
  CLCC_MODE_VAFFax,
  CLCC_MODE_Unknown
}
T_ACI_CLCC_MODE;

typedef enum
{
  CLCC_MPTY_NotPresent       = -1,
  CLCC_MPTY_NoMember,
  CLCC_MPTY_IsMember
}
T_ACI_CLCC_MPTY;

typedef enum
{
  CLCC_CLASS_NotPresent     = -1,
  CLCC_CLASS_Line1,
  CLCC_CLASS_Line2
}
T_ACI_CLCC_CLASS;

typedef struct            /* +CLCC current call list element*/
{
  SHORT           idx;
  T_ACI_CLCC_DIR  dir;
  T_ACI_CLCC_STAT stat;
  T_ACI_CLCC_MODE mode;
  T_ACI_CLCC_MPTY mpty;
  T_ACI_CLCC_CLASS class_type;
  CHAR            number[MAX_CC_ORIG_NUM_LEN];
  T_ACI_TOA       type;
#ifdef FF_BAT
  UBYTE           prog_desc;
#endif
#ifdef NO_ASCIIZ
  T_ACI_PB_TEXT   alpha;
#else
  CHAR            alpha[MAX_ALPHA_LEN];
#endif /* else, #ifdef NO_ASCIIZ */
}
T_ACI_CLCC_CALDESC;

typedef T_ACI_CLCC_CALDESC T_ACI_CLCC_LST [MAX_CALL_NR];

/*---- +COPN ------------------------------------------------------*/
typedef enum              /* +COPN parameter list identifier */
{
  COPN_LID_NotPresent = -1,
  COPN_LID_Pcm,           /* list in permanent configuration memory */
  COPN_LID_Cnst           /* list in constant memory */
}
T_ACI_COPN_LID;

typedef struct            /* +COPN operator list element*/
{
  CHAR   alphaOper[MAX_ALPHA_OPER_LEN];
  CHAR   numOper[MAX_NUM_OPER_LEN];
}
T_ACI_COPN_OPDESC;

typedef T_ACI_COPN_OPDESC T_ACI_COPN_LST [MAX_OPER];

/*---- +CSNS ------------------------------------------------------*/
typedef enum              /* +CSNS parameter <mode> */
{
  CSNS_MOD_NotPresent = -1,
  CSNS_MOD_Voice,
  CSNS_MOD_VAFVoice,
  CSNS_MOD_Fax,
  CSNS_MOD_VADVoice,
  CSNS_MOD_Data,
  CSNS_MOD_VAFFax,
  CSNS_MOD_VADData,
  CSNS_MOD_VFD
}
T_ACI_CSNS_MOD;

/*---- +VTS ------------------------------------------------------*/
typedef enum              /* +VTS parameter <mode> */
{
  VTS_MOD_NotPresent = -1,
  VTS_MOD_ManStop,
  VTS_MOD_ManStart,
  VTS_MOD_Auto
}
T_ACI_VTS_MOD;

/*---- %SIMREM---------------------------------------------------*/
typedef enum              /* Type of SIM remove */
{
  SIMREM_NotPresent = -1,
  SIMREM_RESET,
  SIMREM_FAILURE,
  SIMREM_RETRY
}
T_ACI_SIMREM_TYPE;

/*--- SMS ---------------------------------------------------------*/

typedef enum
{
  SMS_STAT_Invalid            = -2,
  SMS_STAT_NotPresent         = -1,
  SMS_STAT_RecUnread,
  SMS_STAT_RecRead,
  SMS_STAT_StoUnsent,
  SMS_STAT_StoSent,
  SMS_STAT_All
}
T_ACI_SMS_STAT;

typedef enum
{
  SMS_READ_NotPresent         = -1,
  SMS_READ_Normal,
  SMS_READ_Preview,
  SMS_READ_StatusChange
}
T_ACI_SMS_READ;

typedef enum
{
  SMS_STOR_NotPresent         = -1,
  SMS_STOR_Me,
  SMS_STOR_Sm
}
T_ACI_SMS_STOR;

#ifdef REL99
typedef enum
{
  CMGRS_MODE_NotPresent    = -1,
  CMGRS_MODE_DISABLE_AUTO_RETRANS,
  CMGRS_MODE_ENABLE_AUTO_RETRANS,
  CMGRS_MODE_MANUAL_RETRANS
}
T_ACI_CMGRS_MODE;
#endif /* REL99 */

typedef struct
{
  UBYTE year     [MAX_VP_ABS_DIGITS];
  UBYTE month    [MAX_VP_ABS_DIGITS];
  UBYTE day      [MAX_VP_ABS_DIGITS];
  UBYTE hour     [MAX_VP_ABS_DIGITS];
  UBYTE minute   [MAX_VP_ABS_DIGITS];
  UBYTE second   [MAX_VP_ABS_DIGITS];
  SHORT timezone;
}
T_ACI_VP_ABS;

typedef struct
{
  UBYTE func_ind;
  UBYTE ext_oct;
  union
  {
    UBYTE vpenh_relative;
    UBYTE vpenh_seconds;
    struct
    {
      UBYTE hour     [MAX_VP_ABS_DIGITS];
      UBYTE minute   [MAX_VP_ABS_DIGITS];
      UBYTE second   [MAX_VP_ABS_DIGITS];
    } vpenh_hours;
  } val;
}
T_ACI_VP_ENH;

typedef struct
{
  SHORT             index;
  T_ACI_SMS_STAT    stat;
  USHORT            sn;
  USHORT            mid;
  UBYTE             page;
  UBYTE             pages;
  T_ACI_CBM_DATA    data;
}
T_ACI_CMGL_CBM;

typedef T_ACI_CMGL_CBM T_ACI_CMGL_CBM_LST [MAX_CBM_ENTR];

typedef struct
{
  SHORT             index;
  T_ACI_SMS_STAT    stat;
  CHAR              adress[MAX_SMS_ADDR_DIG];
  T_ACI_TOA         toa;
  T_ACI_PB_TEXT     alpha;
  UBYTE             vp_rel;
  T_ACI_VP_ABS      scts;
  T_ACI_VP_ENH      vp_enh;
  UBYTE             fo;
  UBYTE             msg_ref;
  UBYTE             pid;
  UBYTE             dcs;
  T_ACI_SM_DATA     data;
  T_ACI_UDH_DATA    udh;
  CHAR              sca[MAX_SMS_ADDR_DIG];
  T_ACI_TOA         tosca;
  UBYTE             tp_status;
}
T_ACI_CMGL_SM;

typedef T_ACI_CMGL_SM T_ACI_CMGL_SM_LST [MAX_SM_ENTR];

typedef struct
{
  T_ACI_SMS_STAT    stat;
  USHORT            sn;
  USHORT            mid;
  UBYTE             dcs;
  UBYTE             page;
  UBYTE             pages;
  T_ACI_CBM_DATA    data;
}
T_ACI_CMGR_CBM;

typedef struct
{
  T_ACI_SMS_STAT    stat;
  CHAR              addr[MAX_SMS_ADDR_DIG];
  T_ACI_TOA         toa;
  CHAR              sca[MAX_SMS_ADDR_DIG];
  T_ACI_TOA         tosca;
  T_ACI_PB_TEXT     alpha;
  UBYTE             vprel;
  T_ACI_VP_ABS      vpabs_scts;
  UBYTE             fo;
  UBYTE             msg_ref;
  UBYTE             pid;
  UBYTE             dcs;
  T_ACI_SM_DATA     data;
  T_ACI_UDH_DATA    udh;
}
T_ACI_CMGR_SM;

typedef struct
{
  UBYTE             fo;
  UBYTE             msg_ref;
  CHAR              addr[MAX_SMS_ADDR_DIG];
  T_ACI_TOA         toa;
  T_ACI_VP_ABS      vpabs_scts;
  T_ACI_VP_ABS      vpabs_dt;
  UBYTE             tp_status;
}
T_ACI_CDS_SM;

typedef struct
{
  T_ACI_SMS_STOR    mem;
  SHORT             used;
  SHORT             total;
}
T_ACI_SMS_STOR_OCC;


/*--------------- new function types for concat. SMS ---------*/

typedef void T_CMSS_FCT ( UBYTE           mr,
                          UBYTE           numSeg );

typedef void T_CMGS_FCT ( UBYTE           mr,
                          UBYTE           numSeg );

typedef void T_CMGC_FCT ( UBYTE           mr );

typedef void T_CMGR_FCT ( T_ACI_CMGL_SM*  sm,
                          T_ACI_CMGR_CBM* cbm );

typedef void T_CMGW_FCT ( UBYTE           index,
                          UBYTE           numSeg,
                          UBYTE           mem);

typedef void T_CMGD_FCT ( );

#ifdef REL99
typedef void T_CMGRS_FCT (  T_ACI_CMGRS_MODE  mode,
                            U8                mr,
                            U8                resend_count,
                            U8                max_retrans );
#endif


typedef void T_CMGMDU_FCT (void);

typedef void T_ERROR_FCT (T_ACI_AT_CMD cmdId,
                          T_ACI_CMS_ERR err,
                          T_EXT_CMS_ERROR *conc_error );


/*--- WS46 --------------------------------------------------------*/

typedef enum
{
  WS46_MOD_NotPresent       = -1,
  WS46_MOD_Gsm              = 12
}
T_ACI_WS46_MOD;

/*--- +/%CHLD command -----------------------------------------------*/

typedef enum
{
  CHLD_MOD_NotPresent       = -1,
  CHLD_MOD_RelHldOrUdub,            /* Entering 0  followed by SEND */
  CHLD_MOD_RelActAndAcpt,           /* Entering 1  followed by SEND */
  CHLD_MOD_RelActSpec,              /* Entering 1X followed by SEND */
  CHLD_MOD_HldActAndAcpt,           /* Entering 2  followed by SEND */
  CHLD_MOD_HldActExc,               /* Entering 2X followed by SEND */
  CHLD_MOD_AddHld,                  /* Entering 3  followed by SEND */
  CHLD_MOD_Ect,                     /* Entering 4  followed by SEND */
  CHLD_MOD_Ccbs,                    /* Entering 5  followed by SEND */
  CHLD_MOD_HldActDial,              /* Entering "Directory number"  */
  CHLD_MOD_OnlyHold         = 99,   /* special for FTA: AT+CHLD=H   */
  CHLD_MOD_RelDialCall,             /* special AT+CHLD=I            */
  CHLD_MOD_RetrieveHoldCall,        /* special for %CHLD=6 (Symbian/S60) */
  CHLD_MOD_RetrieveHoldCallSpec,    /* special for %CHLD=6x (Symbian/S60) */
  CHLD_MOD_RelAnySpec               /* special for %CHLD=7x         */
}
T_ACI_CHLD_MOD;

typedef enum
{
  CHLD_PercentCmd = 0,
  CHLD_PlusCmd
}
T_ACI_CHLD_CMD;

typedef enum
{
  CHLD_ACT_NotPresent       = -1,
  CHLD_ACT_Accept,
  CHLD_ACT_Release,
  CHLD_ACT_Hold,
  CHLD_ACT_Retrieve,
  CHLD_ACT_Swap,
  CHLD_ACT_ReleaseMpty,
  CHLD_ACT_HoldMpty,
  CHLD_ACT_RetrieveMpty,
  CHLD_ACT_SwapMpty,
  CHLD_ACT_BuildMpty,
  CHLD_ACT_SplitMpty,
  CHLD_ACT_ECT,
  CHLD_ACT_CCBS
}
T_ACI_CHLD_ACT;

/*---- +IPR ------------------------------------------------------*/
typedef enum              /* +IPR parameter <rate> */
{
  BD_RATE_NotPresent = -1,
  BD_RATE_AUTO = 0,
  BD_RATE_75 = 1,
  BD_RATE_150,
  BD_RATE_300,
  BD_RATE_600,
  BD_RATE_1200,
  BD_RATE_2400,
  BD_RATE_4800,
  BD_RATE_7200,
  BD_RATE_9600,
  BD_RATE_14400,
  BD_RATE_19200,
  BD_RATE_28800,
  BD_RATE_33900,
  BD_RATE_38400,
  BD_RATE_57600,
  BD_RATE_115200,
  BD_RATE_203125,
  BD_RATE_406250,
  BD_RATE_812500
}
T_ACI_BD_RATE;

/*---- +ICF ------------------------------------------------------*/
typedef enum              /* +ICF parameter <format> */
{
  BS_FRM_NotPresent = -1,
  /*BS_FRM_AutoDetect,*/     /* not supported */
  BS_FRM_Dat8_Par0_St2 = 1,
  BS_FRM_Dat8_Par1_St1,
  BS_FRM_Dat8_Par0_St1,
  BS_FRM_Dat7_Par0_St2,
  BS_FRM_Dat7_Par1_St1,
  BS_FRM_Dat7_Par0_St1
}
T_ACI_BS_FRM;

typedef enum              /* +ICF parameter <parity> */
{
  BS_PAR_NotPresent = -1,
  BS_PAR_Odd,
  BS_PAR_Even,
  BS_PAR_Mark,
  BS_PAR_Space
}
T_ACI_BS_PAR;

/*---- +IFC ------------------------------------------------------*/
typedef enum              /* +IFC parameter <DCE_by_DTE> */
{
  RX_FLOW_NotPresent = -1,
  RX_FLOW_NONE,
  RX_FLOW_SOFTWARE,
  RX_FLOW_HARDWARE,
  RX_FLOW_BIGGEST = 0xffff /*for lint warning 650*/ 
}
T_ACI_RX_FLOW_CTRL;

typedef enum              /* +IFC parameter <DTE_by_DCE> */
{
  TX_FLOW_NotPresent = -1,
  TX_FLOW_NONE,
  TX_FLOW_SOFTWARE,
  TX_FLOW_HARDWARE
}
T_ACI_TX_FLOW_CTRL;

/*--- +CRSM -------------------------------------------------------*/

typedef enum              /* +CRSM parameter <cmd> */
{
  CRSM_CMD_NotPresent = -1,
  CRSM_CMD_ReadBin    = 176,
  CRSM_CMD_ReadRec    = 178,
  CRSM_CMD_GetResp    = 192,
  CRSM_CMD_UpdBin     = 214,
  CRSM_CMD_UpdRec     = 220,
  CRSM_CMD_Status     = 242
}
T_ACI_CRSM_CMD;

/*--- +CSIM -------------------------------------------------------*/

/*--- +CCWV -------------------------------------------------------*/

typedef enum              /* +CCWV parameter <chrg> */
{
  CCWV_CHRG_NotPresent          = -1,
  CCWV_CHRG_Termination,
  CCWV_CHRG_Abundance,
  CCWV_CHRG_Shortage
}
T_ACI_CCWV_CHRG;

/*---- &C ---------------------------------------------------------*/
typedef enum                   /* data carrier detect modes */
{
  DCD_ALWAYS_ON = 0,           /* DCD line always on  */
  DCD_DISABLE_AFTER_CALL       /* disable DCD line at end of call */
} T_ACI_DCD_MOD;

/*--- %CAL --------------------------------------------------------*/

typedef enum              /* Status of current calls */
{
  CAL_STAT_NotPresent = -1,
  CAL_STAT_Held,
  CAL_STAT_Active,
  CAL_STAT_Wait,
  CAL_STAT_Dial,
  CAL_STAT_DeactiveReq,
  CAL_STAT_Incomming,
  CAL_STAT_Alerting
}
T_ACI_CAL_STAT;

typedef enum              /* Type of current calls */
{
  CAL_TYPE_NotPresent = -1,
  CAL_TYPE_MOC,
  CAL_TYPE_MTC
}
T_ACI_CAL_TYPE;

typedef enum              /* in-band tones usage */
{
  CAL_IBT_NotPresent = -1,
  CAL_IBT_FALSE,
  CAL_IBT_TRUE
}
T_ACI_CAL_IBT;

typedef enum              /* mode of current calls */
{
  CAL_MODE_NotPresent = -1,
  CAL_MODE_Voice,
  CAL_MODE_Data,
  CAL_MODE_Fax,
  CAL_MODE_VFD_Voice,       /* voice followed data, voice mode */
  CAL_MODE_VAD_Voice,       /* voice alternating data, voice mode */
  CAL_MODE_VAF_Voice,       /* voice alternating fax, voice mode */
  CAL_MODE_VFD_Data,        /* voice followed data, voice mode */
  CAL_MODE_VAD_Data,        /* voice alternating data, voice mode */
  CAL_MODE_VAF_Fax,         /* voice alternating fax, voice mode */
  CAL_MODE_Unknown
}
T_ACI_CAL_MODE;

typedef enum              /* call owner */
{
  CAL_OWN_NotPresent = -1,
  CAL_OWN_LCL,              /* local call */
  CAL_OWN_RMT,              /* remote call */
  CAL_OWN_NONE
}
T_ACI_CAL_OWN;

typedef enum
{
  CAL_MPTY_NotPresent       = -1,
  CAL_MPTY_NoMember,
  CAL_MPTY_IsMember
}
T_ACI_CAL_MPTY;

typedef struct            /* Call table entry */
{
  SHORT           index;
  T_ACI_CAL_STAT  status;
  CHAR            number[MAX_CC_ORIG_NUM_LEN];
  T_ACI_TOA       type;
#ifdef NO_ASCIIZ
  T_ACI_PB_TEXT   alpha;
#else
  CHAR            alpha[MAX_ALPHA_LEN];
#endif /* else, #ifdef NO_ASCIIZ */
  T_ACI_CAL_TYPE  calType;
  T_ACI_CAL_IBT   ibtUse;
  T_ACI_CAL_MODE  calMode;
  T_ACI_CAL_OWN   calOwner;
  T_ACI_CAL_MPTY  mpty;
}
T_ACI_CAL_ENTR;

 /*List of current calls*/
typedef T_ACI_CAL_ENTR T_ACI_CAL_LST [MAX_CALL_NR];

/*---- %DRV --------------------------------------------------------*/
typedef enum               /* DRV parameter <device> */
{
  DRV_DEV_Keypad,
  DRV_DEV_Audio,
  DRV_DEV_Backlight,
  DRV_DEV_Display
}
T_ACI_DRV_DEV;

typedef enum               /* DRV parameter <function> */
{
  DRV_FCT_KeypadInd,
  DRV_FCT_AudioInputReq,
  DRV_FCT_AudioOutputReq,
  DRV_FCT_BacklightReq,
  DRV_FCT_DisplayReq
}
T_ACI_DRV_FCT;

/*---- %NRG --------------------------------------------------------*/
typedef enum               /* NRG parameter <regMode> */
{
  NRG_RGMD_NotPresent = -1,
  NRG_RGMD_Auto,
  NRG_RGMD_Manual,
  NRG_RGMD_Dereg,     /* not yet implemented */
  NRG_RGMD_SetOnly,   /* not yet implemented */
  NRG_RGMD_Both
}
T_ACI_NRG_RGMD;

typedef enum               /* NRG parameter <srvMode> */
{
  NRG_SVMD_NotPresent = -1,
  NRG_SVMD_Full,
  NRG_SVMD_Limited,
  NRG_SVMD_NoSrv,
  NRG_SVMD_SetRegModeOnly
}
T_ACI_NRG_SVMD;

typedef enum              /* +COPS parameter <format> */
{
  NRG_FRMT_NotPresent = -1,
  NRG_FRMT_Long,
  NRG_FRMT_Short,
  NRG_FRMT_Numeric
}
T_ACI_NRG_FRMT;

/*---- %COLR ------------------------------------------------------*/

typedef enum              /* %COLR parameter <stat> */
{
  COLR_STAT_NotPresent = -1,
  COLR_STAT_NotProv,
  COLR_STAT_Prov,
  COLR_STAT_Unknown
}
T_ACI_COLR_STAT;

/*---- %PVRF -------------------------------------------------------*/

typedef enum              /* %PVRF parameter <type> */
{
  PVRF_TYPE_NotPresent = -1,
  PVRF_TYPE_Pin1,
  PVRF_TYPE_Pin2,
  PVRF_TYPE_Puk1,
  PVRF_TYPE_Puk2
}
T_ACI_PVRF_TYPE;

typedef enum              /* %PVRF parameter <ps1> and <ps2> */
{
  PVRF_STAT_NotPresent = -1,
  PVRF_STAT_NotRequired,
  PVRF_STAT_Required
}
T_ACI_PVRF_STAT;

/*---- %EFRSLT ----------------------------------------------------*/

typedef enum
{
  EFRSLT_RES_FAIL,
  EFRSLT_RES_OK
}
T_ACI_EFRSLT_RES;

/*---- %SIMEF -----------------------------------------------------*/

typedef enum
{
  SIMEF_MODE_OFF,
  SIMEF_MODE_ON
}
T_ACI_SIMEF_MODE;

/*---- %KSIR ------------------------------------------------------*/

typedef struct
{
  UBYTE bsTp;
  UBYTE bsCd;
} T_Cx_BSG;

typedef struct
{
  UBYTE bsTp;
  UBYTE bsCd;
  UBYTE ssSt;
} T_CB_INFO;

typedef struct
{
  UBYTE bsTp;
  UBYTE bsCd;
  UBYTE ssSt;
  UBYTE num[MAX_B_SUBSCR_NUM_LEN];
  UBYTE ton;
  UBYTE npi;
  UBYTE sub[MAX_SUBADDR_LEN];
  UBYTE tos;
  UBYTE oe;
  UBYTE time;
} T_CF_FEAT; /* Call Forwarding FEATure */

typedef struct
{
  UBYTE bsTp;
  UBYTE bsCd;
  UBYTE ssSt;
  UBYTE num[MAX_B_SUBSCR_NUM_LEN];
  UBYTE ton;
  UBYTE npi;
  UBYTE sub[MAX_SUBADDR_LEN];
  UBYTE tos;
  UBYTE oe;
  UBYTE idx;
} T_CC_FEAT; /* CCbs FEATure */

typedef struct
{
  UBYTE tac1;
  UBYTE tac2;
  UBYTE tac3;
  UBYTE fac;
  UBYTE snr1;
  UBYTE snr2;
  UBYTE snr3;
  UBYTE svn;
  UBYTE cd;
} T_ACI_IMEI;

typedef struct
{
  T_ACI_KSD_CMD ksdCmd;
  /*
  ** CQ12314 : NDH : 23/9/2003 :
  ** Required by MMI to determine what action to take in case when AT command used on Terminal.
  ** (eg Display Call Forwarding Icon)
  */
  T_ACI_CMD_SRC srcId;
  union
  {
    struct { UBYTE      opCd;
             UBYTE      ssCd;
             UBYTE      ssErr;
             UBYTE      ssSt;
             T_ACI_CLIR_MOD mode; /* "mode" is used only for AT_Interpreter */
             UBYTE      clirOpt;
             UBYTE      ovrdCtg;   } rKSCL;
    struct { UBYTE      opCd;
             UBYTE      ssCd;
             UBYTE      ssErr;
             UBYTE      ssSt;
             UBYTE      c_cwBSGLst;
             T_Cx_BSG  *cwBSGLst;   } rKSCW;
    struct { UBYTE      opCd;
             UBYTE      ssCd;
             UBYTE      ssErr;
             UBYTE      c_cfFeatLst;
             T_CF_FEAT *cfFeatLst;  } rKSCF;
    struct { UBYTE      opCd;
             UBYTE      ssCd;
             UBYTE      ssErr;
             UBYTE      c_cbInfoLst;
             T_CB_INFO *cbInfoLst;  } rKSCB;
    struct { UBYTE      opCd;
             UBYTE      ssCd;
             UBYTE      ssErr;
             UBYTE      errPrms;
             UBYTE      newPwd[MAX_PWD_NUM+1]; } rKSPW;
    struct { UBYTE     *ussd;
             UBYTE      ssErr;
             UBYTE      len;   /*store the USSD string len due to possible unicode string*/
             SHORT      dcs;  /* "dcs" and "mode" are used only for AT_Interpreter */
             T_ACI_CUSD_MOD mode; } rKSUS;
    T_ACI_IMEI rKSIMEI;
    struct { UBYTE      opCd;
             UBYTE      ssCd;
             UBYTE      ssErr;
             UBYTE      ssSt;
             UBYTE      c_ccFeatLst;
             T_CC_FEAT *ccFeatLst; } rKSCC;
  } ir;
} T_ACI_KSIR;

/* %CSQ parameter */
typedef enum
{
  CSQ_Disable = 0,
  CSQ_Enable  = 1
}
T_ACI_CSQ_MODE;



/*---- %CHPL------------------------------------------------------*/

typedef enum
{
  CHPL_FRMT_NotPresent = -1,
  CHPL_FRMT_Long,
  CHPL_FRMT_Short,
  CHPL_FRMT_Numeric
} T_ACI_CHPL_FRMT;

/*---- %CWUP------------------------------------------------------*/

typedef enum
{
  CWUP_TYPE_NotPresent = -1,
  CWUP_TYPE_RR         =  1
} T_ACI_CWUP_TYPE;

/*---- %CLOG ------------------------------------------------------*/

typedef enum
{
  CLOG_TYPE_NotPresent          = -1,
  CLOG_TYPE_Set,
  CLOG_TYPE_Query,
  CLOG_TYPE_Test
}
T_ACI_CLOG_TYPE;

typedef struct              /* %CLOG parameter */
{
  T_ACI_AT_CMD    atCmd;
  T_ACI_CLOG_TYPE cmdType;
  T_ACI_RETURN    retCode;
  SHORT           cId;
  SHORT           sId;
  union
  {
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CFUN_FUN     fun;
             T_ACI_CFUN_RST     rst;          } sCFUN;
    struct { T_ACI_CMD_SRC      srcId;
             CHAR               *pin;
             CHAR               *newpin;      } sCPIN;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_COPS_MOD     mode;
             T_ACI_COPS_FRMT    format;
             CHAR               *oper;        } sCOPS;
    struct { T_ACI_CMD_SRC      srcId;
             SHORT              startIdx;
             SHORT              *lastIdx;
             T_ACI_COPS_OPDESC  *operLst;     } tCOPS;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CLIP_STAT    *stat;        } qCLIP;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CLIR_MOD     *mode;
             T_ACI_CLIR_STAT    *stat;        } qCLIR;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_COLP_STAT    *stat;        } qCOLP;
    struct { T_ACI_CMD_SRC      srcId;
             CHAR               *number;
             T_ACI_D_CLIR_OVRD  clirOvrd;
             T_ACI_D_CUG_CTRL   cugCtrl;
             T_ACI_D_TOC        callType;
#ifdef SIM_TOOLKIT
             T_ACI_D_SIMCC      simCallCtrl;
#endif /* SIM_TOOLKIT */
                                              } sD;
    struct { T_ACI_CMD_SRC      srcId;        } sA;
    struct { T_ACI_CMD_SRC      srcId;        } sH;
    struct { T_ACI_CMD_SRC      srcId;        } sZ;
    struct { T_ACI_CMD_SRC      srcId;        } sCHUP;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_FAC     fac;
             T_ACI_CLCK_MOD     mode;
             CHAR               *passwd;
             T_ACI_CLASS        class_type;        } sCLCK;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_FAC     fac;
             T_ACI_CLASS        class_type;        } qCLCK;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_FAC     fac;
             CHAR               *oldpwd;
             CHAR               *newpwd;      } sCPWD;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CCFC_RSN     reason;
             T_ACI_CCFC_MOD     mode;
             CHAR               *number;
             T_ACI_TOA          *type;
             T_ACI_CLASS        class_type;
             CHAR               *subaddr;
             T_ACI_TOS          *satype;
             SHORT              time;         } sCCFC;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CCFC_RSN     reason;
             T_ACI_CLASS        class_type;        } qCCFC;
    struct { T_ACI_CMD_SRC      srcId;
             CHAR               *number;
             T_ACI_TOA          *type;
             CHAR               *subaddr;
             T_ACI_TOS          *satype;      } sCTFR;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CCWA_MOD     mode;
             T_ACI_CLASS        class_type;        } sCCWA;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CLASS        class_type;        } qCCWA;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CHLD_MOD     mode;
             CHAR               *call;
             T_ACI_CHLD_ACT     act;          } sCHLD;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_USSD_DATA   *str;
             SHORT              dcs;          } sCUSD;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_NRG_RGMD     regMode;
             T_ACI_NRG_SVMD     srvMode;
             T_ACI_NRG_FRMT     oprFrmt;
             CHAR               *opr;         } sNRG;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_COLR_STAT    *stat;        } qCOLR;
    struct { T_ACI_CMD_SRC      srcId;
             SHORT              idx;          } sCCBS;
    struct { T_ACI_CMD_SRC      srcId;        } qCCBS;
    struct { T_ACI_CMD_SRC      srcId;
             UBYTE              index;        } sCMGD;
    struct { T_ACI_CMD_SRC      srcId;
             SHORT              index;
             CHAR               *address;
             T_ACI_TOA          *toa;
             T_ACI_SMS_STAT     stat;
             CHAR               *data;        } sCMGW;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CSMS_SERV    service;      } sCSMS;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_SMS_STOR     mem1;
             T_ACI_SMS_STOR     mem2;
             T_ACI_SMS_STOR     mem3;         } sCPMS;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CMGF_MOD     mode;         } sCMGF;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CMGF_MOD     *mode;        } qCMGF;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CSCB_MOD     mode;
             USHORT             *mids;
             UBYTE              *dcss;        } sCSCB;
    struct { T_ACI_CMD_SRC      srcId;
             CHAR               *da;
             T_ACI_TOA          *toda;
             CHAR               *data;        } sCMGS;
    struct { T_ACI_CMD_SRC      srcId;
             UBYTE              index;
             CHAR               *da;
             T_ACI_TOA          *toda;        } sCMSS;
    struct { T_ACI_CMD_SRC      srcId;
             SHORT              fo;
             SHORT              ct;
             SHORT              pid;
             SHORT              mn;
             CHAR               *da;
             T_ACI_TOA          *toda;
             CHAR               *data;        } sCMGC;
    struct { T_ACI_CMD_SRC      srcId;
             UBYTE              opCd;
             UBYTE              ssCd;
             UBYTE              bsTp;
             UBYTE              bsCd;
             UBYTE              *num;
             UBYTE              npi;
             UBYTE              ton;
             UBYTE              *sub;
             UBYTE              tos;
             UBYTE              oe;
             UBYTE              time;        } sKSCF;
    struct { T_ACI_CMD_SRC      srcId;
             UBYTE              opCd;
             UBYTE              ssCd;
             UBYTE              bsTp;
             UBYTE              bsCd;
             UBYTE              *pwd;        } sKSCB;
    struct { T_ACI_CMD_SRC      srcId;
             UBYTE              opCd;
             UBYTE              bsTp;
             UBYTE              bsCd;        } sKSCW;
    struct { T_ACI_CMD_SRC      srcId;
             UBYTE              opCd;
             UBYTE              ssCd;        } sKSCL;
    struct { T_ACI_CMD_SRC      srcId;
             UBYTE              opCd;
             UBYTE              ssCd;
             UBYTE              idx;         } sKSCC;
    struct { T_ACI_CMD_SRC      srcId;
             UBYTE              ssCd;
             UBYTE              *oldPwd;
             UBYTE              *newPwd;     } sKSPW;
    struct { T_ACI_CMD_SRC      srcId;
             UBYTE              ssCd;
             UBYTE              *puk;
             UBYTE              *pin;        } sKSUB;
    struct { T_ACI_CMD_SRC      srcId;
             UBYTE              *ussd;       } sKSUS;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_PVRF_TYPE    type;
             CHAR               *pin;
             CHAR               *newpin;     } sPVRF;
    struct { T_ACI_CMD_SRC      srcId;
             T_ACI_CHPL_FRMT    format;
             CHAR               *oper;        } sCHPL;

   } cmdPrm;
}
T_ACI_CLOG;

/*---- %RLOG ------------------------------------------------------*/
typedef enum
{
  RLOG_RSLT_NotPresent = -1,
  RLOG_RSLT_OK,
  RLOG_RSLT_NoCarrier,
  RLOG_RSLT_Connect,
  RLOG_RSLT_Busy,
  RLOG_RSLT_NoAnswer,
  RLOG_RSLT_CME
} T_ACI_RLOG_RSLT;


typedef struct              /* %CLOG parameter */
{
  T_ACI_RLOG_RSLT atRslt;
  T_ACI_CMD_SRC   dest;
  union
  {
    struct { T_ACI_AT_CMD     cmdId;
             SHORT            cId;   } rOK;
    struct { T_ACI_AT_CMD     cmdId;
             SHORT            cId;   } rNO_CARRIER;
    struct { T_ACI_AT_CMD     cmdId;
             T_ACI_BS_SPEED   speed;
             SHORT            cId;   } rCONNECT;
    struct { T_ACI_AT_CMD     cmdId;
             SHORT            cId;   } rBUSY;
    struct { T_ACI_AT_CMD     cmdId;
             SHORT            cId;   } rNO_ANSWER;
    struct { T_ACI_AT_CMD     cmdId;
             T_ACI_CME_ERR    err;
             SHORT            cId;   } rCME;
  } rsltPrm;
}
T_ACI_RLOG;

/*---- %CPI ------------------------------------------------------*/

typedef enum              /* %CPI parameter <msg> */
{
  CPI_MSG_NotPresent = -1,
  CPI_MSG_Setup,
  CPI_MSG_Disc,
  CPI_MSG_Alert,
  CPI_MSG_Proc,
  CPI_MSG_Sync,
  CPI_MSG_Progr,
  CPI_MSG_Conn,
  CPI_MSG_Rls,
  CPI_MSG_Rjct,
  CPI_MSG_MO_Setup,
  CPI_MSG_Hld,
  CPI_MSG_Ntfy
}
T_ACI_CPI_MSG;

typedef enum              /* %CPI parameter <ibt> */
{
  CPI_IBT_NotPresent = -1,
  CPI_IBT_False,
  CPI_IBT_True
}
T_ACI_CPI_IBT;

typedef enum              /* %CPI parameter <tch> */
{
  CPI_TCH_NotPresent = -1,
  CPI_TCH_False,
  CPI_TCH_True
}
T_ACI_CPI_TCH;

/*---- %CSTAT ----------------------------------------------------*/

typedef enum             
{
  /*
   *  New enum values needs to be added  
   *  before the MAX_ENTITIES entry.
   *  
   */
  STATE_MSG_PBOOK         =  0,
  STATE_MSG_SMS,
  STATE_MSG_EONS,
  STATE_MSG_MAX_ENTITIES,         
  STATE_MSG_RDY           =  255
}
T_ACI_ENTITY_ID_MSG;

typedef enum             
{
  ENTITY_STATUS_NotReady = 0,
  ENTITY_STATUS_Ready    = 1
}
T_ACI_ENTITY_STATE_MSG;


typedef struct               /* %CSTAT parameter <msg> */
{
  T_ACI_ENTITY_ID_MSG     entityId;   
  T_ACI_ENTITY_STATE_MSG  entityState;
}
T_ACI_STATE_MSG;


/*---- %PBCF ------------------------------------------------------*/

typedef enum              /* %PBCF parameter <ldn> */
{
  PBCF_LDN_NotPresent = -1,
  PBCF_LDN_Enable,
  PBCF_LDN_Disable
}
T_ACI_PBCF_LDN;

typedef enum              /* %PBCF parameter <lrn> */
{
  PBCF_LRN_NotPresent = -1,
  PBCF_LRN_Enable,
  PBCF_LRN_Disable
}
T_ACI_PBCF_LRN;

typedef enum              /* %PBCF parameter <lmn> */
{
  PBCF_LMN_NotPresent = -1,
  PBCF_LMN_Enable,
  PBCF_LMN_Disable
}
T_ACI_PBCF_LMN;
/*---- %RPCT ------------------------------------------------------*/

typedef struct              /* %RPCT parameter <rpuct> */
{
  UBYTE currency [MAX_CUR_LEN];
  ULONG eppu;
  ULONG exp;
  ULONG sexp;
}
T_ACI_RPCT_VAL;

/*---- %SATT ------------------------------------------------------*/

typedef enum               /* %SATT parameter <cause> */
{
  SATT_CS_NotPresent = -1,
  SATT_CS_UserRedialStop,
  SATT_CS_EndRedial,
  SATT_CS_EndSession
}
T_ACI_SATT_CS;

/*---- %CCBS ------------------------------------------------------*/

typedef enum               /* %CCBS parameter <ind> */
{
  CCBS_IND_NotPresent = -1,
  CCBS_IND_PossibilityTimedOut,
  CCBS_IND_Possible,
  CCBS_IND_Registered,
  CCBS_IND_Recall,
  CCBS_IND_RecallTimedOut,
  CCBS_IND_IrgtResult
}
T_ACI_CCBS_IND;

typedef enum               /* %CCBS parameter <status> */
{
  CCBS_STAT_NotPresent = -1,
  CCBS_STAT_NotProvisioned,
  CCBS_STAT_Provisioned,
  CCBS_STAT_Active
}
T_ACI_CCBS_STAT;


typedef struct
{
  SHORT           idx;
  CHAR            number[MAX_B_SUBSCR_NUM_LEN];
  T_ACI_TOA       type;
  CHAR            subaddr[MAX_SUBADDR_LEN];
  T_ACI_TOS       satype;
  T_ACI_CLASS     class_type;
  T_ACI_ALRT_PTRN alrtPtn;
}
T_ACI_CCBS_SET;

/*---- Phonebook Management ---------------------------------------*/

typedef enum
{
  PB_STAT_Ready   = 0,    /* Phonebook ready */
#ifdef TI_PS_FFS_PHB
  PB_STAT_Busy,           /* Phonebook temporarily not available */
#endif
  PB_STAT_Blocked         /* Phonebook not available */
}
T_ACI_PB_STAT;

typedef enum              /* phonebook storage */
{
  PB_STOR_NotPresent = -1,
  PB_STOR_Fd,
  PB_STOR_Ld,
  PB_STOR_Ed,
  PB_STOR_Ad,
  PB_STOR_Bd,
  PB_STOR_Lr,
  PB_STOR_Sd,
  PB_STOR_Lm,
  PB_STOR_Af,
  PB_STOR_Ud  /* user person number */
}
T_ACI_PB_STOR;

typedef struct
{
  char *name;
  T_ACI_PB_STOR stor;
} Memory;

#ifdef CMH_PHBS_C
GLOBAL const Memory phb_mem_names[] =
{
  {"FD",  PB_STOR_Fd},
  {"DC",  PB_STOR_Ld}, /* 07.07 common name for LDN */ 
  {"LD",  PB_STOR_Ld}, /* does not reflect the SIM LDN since it is only written to SIM on CFUN=0 */
  {"RC",  PB_STOR_Lr}, /* 07.07 */
  {"LR",  PB_STOR_Lr}, /* TI equivalent */
  {"EN",  PB_STOR_Ed},
  {"BD",  PB_STOR_Bd},
  {"MT",  PB_STOR_Ad}, /* 07.07 */
  {"AD",  PB_STOR_Ad}, /* TI equivalent */
  {"SM",  PB_STOR_Ad}, /* 07.07 SIM phonebook */
  {"SD",  PB_STOR_Sd},
  {"MC",  PB_STOR_Lm}, /* 07.07 */
  {"LM",  PB_STOR_Lm}, /* TI equivalent */
  {"AF",  PB_STOR_Af},
  {"ON",  PB_STOR_Ud}, /* 07.07 */
  {"UD",  PB_STOR_Ud}, /* TI equivalent */
/* right now not available
  {"ME",  PB_STOR_??}, 07.07 ME phonebook
  {"TA",  PB_STOR_??}, 07.07 TA phonebook
*/
  {0,PB_STOR_Fd}
};
#else
EXTERN const Memory phb_mem_names[];
#endif


typedef struct
{
  T_ACI_PB_STOR book;
  SHORT         index;
  CHAR          number[MAX_PHB_NUM_LEN];
  T_ACI_TOA     type;
  T_ACI_PB_TEXT text;
  T_ACI_VP_ABS  dateTime;
  UBYTE         line;
}
T_ACI_PB_ENTR;

typedef T_ACI_PB_ENTR T_ACI_PB_LST [MAX_PB_ENTR];

typedef enum
{
  CPBF_MOD_NewSearch      =   0,
  CPBF_MOD_NextSearch
}
T_ACI_CPBF_MOD;

/*---- %ALS --------------------------------------------------*/
typedef enum               /* %ALS parameter <mod> bit_field*/
{
  ALS_MOD_NOTPRESENT = 0,
  ALS_MOD_SPEECH     = 1,
  ALS_MOD_AUX_SPEECH = 2
} T_ACI_ALS_MOD;

/*---- %BAND --------------------------------------------------*/

typedef enum
{
  BAND_MODE_Auto   = 0,
  BAND_MODE_Manual = 1
}
T_ACI_BAND_MODE;

/*---- %DINF --------------------------------------------------*/
typedef struct
{
  UBYTE                 dev_id;        /* id(name) of device            */
  UBYTE                 dev_no;        /* instance of device            */
  UBYTE                 sub_no;        /* instance with multiplexed ch. */
  UBYTE                 capability;    /* capability of device          */
  UBYTE                 src_id;        /* what ACI sees as AT cmd src   */
  UBYTE                 cur_cap;       /* capability of the DTI channel */
  UBYTE                 driver_id;     /* driver specific id like USB,  */
  UBYTE                 dio_ctrl_id;   /* device combination id controlled */
}T_DINF_PARAM;

/*---- +CTZR --------------------------------------------------*/
typedef enum
{
   CTZR_MODE_OFF = 0,
   CTZR_MODE_ON
} T_ACI_CTZR_MODE;

/*---- +CTZU --------------------------------------------------*/
typedef enum
{
   CTZU_MODE_OFF = 0,
   CTZU_MODE_ON
} T_ACI_CTZU_MODE;

/*---- +CCLK --------------------------------------------------*/
 typedef enum 
{
  TIME_FORMAT_12HOUR,
  TIME_FORMAT_24HOUR
} T_ACI_RTC_TIME_FORMAT;

typedef struct {
  UBYTE day;
  UBYTE month;
  USHORT  year;
}   T_ACI_RTC_DATE;


typedef struct
{ UBYTE minute;
  UBYTE hour;
  UBYTE   second;
  T_ACI_RTC_TIME_FORMAT format;
  BOOL  PM_flag;
} T_ACI_RTC_TIME;

/*---- %CTZV --------------------------------------------------*/
typedef enum
{
   PCTZV_MODE_OFF = 0,
   PCTZV_MODE_ON
} T_ACI_PCTZV_MODE;


/*---- %CNIV --------------------------------------------------*/
typedef enum
{
   CNIV_MODE_OFF = 0,
   CNIV_MODE_ON
} T_ACI_CNIV_MODE;

/*********************************************************************/
/******************* CPHS Module *************************************/
/*********************************************************************/
#ifdef FF_CPHS

typedef enum
{
  ACI_CPHS_CLOSE  = 0,
  ACI_CPHS_INIT,
  ACI_CPHS_REFRESH,
  ACI_CPHS_BUSY

} T_ACI_CPHS_INIT;

#endif /* FF_CPHS */


#if defined FF_WAP || defined GPRS || defined (FF_SAT_E)
/*==== WAP typedefs =================================================*/

typedef enum
{
  A_NO_AUTH = 0, /* No authentification (ignore login + pwd) */
  A_PAP,        /* PAP */
  A_CHAP,        /* CHAP */
  A_AUTO_AUTH    /* automatic authentification */
} T_ACI_PPP_PROT;

typedef enum
{
  USE_NO_PPP_FOR_AAA = 0, /* connect AAA-L2R     */
  USE_PPP_FOR_AAA         /* connect AAA-PPP-L2R */
} T_ACI_PPP_CON;

#endif /*WAP or GPRS */

/*==== EM typedefs =================================================*/
#ifdef FF_EM_MODE
typedef enum
{
  EM_NOT_SUP               = -1,
  EM_AT_SC                 =  1,
  EM_AT_SC_GPRS,
  EM_AT_NC,
  EM_AT_LOC_PAG,
  EM_AT_PLMN,
  EM_AT_CIPH_HOP_DTX,
  EM_AT_POWER,
  EM_AT_ID,
  EM_AT_VER,
  EM_AT_GMM,     /*for GMM Info Req*/
  EM_AT_GRLC,    /*for GRLC Info Req*/
  EM_AT_AMR,
  EM_AT_PDP,

  /*This defines a (value+1) if all bits in the bitmap are set to request EM data*/
  /*2^19 - This is the actual (value + 1)  the EMW is sending*/
   EM_AT_PCO_HIGHEST = 524288 

  /*2^12 - This is the value of EM - data that is actual supported by the PS*/
  /*EM_AT_PCO_HIGHEST = 2 * EM_PCO_GRLC_INFO*/
} T_EM_AT_TYPE;

typedef enum
{
  DIS_AUTO_REP             = 0,
  EN_AUTO_REP              = 1,
  SIN_REP                  = 2,
  PCO_REP                  = 3  /*enable/disable PCO-trace*/
} T_EM_AT_MODE;

typedef struct
{
  UBYTE em_utype;
  union {
    T_EM_SC_INFO_CNF            em_sc_val;
    T_EM_SC_GPRS_INFO_CNF       em_sc_gprs_val;
    T_EM_NC_INFO_CNF            em_nc_val;
    T_EM_LOC_PAG_INFO_CNF       em_loc_val;
    T_EM_PLMN_INFO_CNF          em_plmn_val;
    T_EM_CIP_HOP_DTX_INFO_CNF   em_cip_val;
    T_EM_POWER_INFO_CNF         em_power_val;
    T_EM_IDENTITY_INFO_CNF      em_id_val;
    T_EM_SW_VER                 em_ver_val;
    T_EM_GMM_INFO_CNF           em_gmm_val;
    T_EM_GRLC_INFO_CNF          em_grlc_val;
    T_EM_AMR_INFO_CNF           em_amr_val;
  } em_u;
} T_EM_VAL;

#endif /* FF_EM_MODE */

#ifdef FF_SAT_E /* SIM_TOOLKIT */
/*==== SAT typedefs =================================================*/

typedef enum
{
  SATC_DIS = 0,   /* unsolicited SAT output disabled */
  SATC_ENA,       /* standard unsolicited SAT output enabled */
  SATC_ENA_CL_E   /* class E unsolicited SAT output enabled */
} T_ACI_SATC_STAT;

typedef enum
{
  SATA_CT_NO = 0,      /* no channel type given */
  SATA_CT_VOICE,       /* channel type voice */
  SATA_CT_CSD,         /* channel type CSD */
  SATA_CT_GPRS         /* channel type GPRS */
} T_ACI_SATA_CHN;

typedef enum
{
  SATA_EST_NO = 0,      /* no establishment type given */
  SATA_EST_IM,          /* immediate channel establishment */
  SATA_EST_OD           /* on-demand channel establishment */
} T_ACI_SATA_EST;

typedef struct
{
  T_ACI_SATA_CHN chnType;
  T_ACI_SATA_EST chnEst;
}
T_ACI_SATA_ADD;

#endif /* FF_SAT_E */


#ifdef FF_EOTD

#define MAX_NUMB_LC_CLIENTS 5
#define MAX_LC_SERV_NAME 24
#define IMM_POS_DATA_REQ 0xff
#define MAX_POSITION_DATA 140

typedef enum
{
  LOCATION_SERVICE_OFF = 0,
  LOCATION_SERVICE_ON
} FEATURE_LC_STAT;

typedef enum
{
  PERIODIC_UPDATE_OFF = 0,
  PERIODIC_UPDATE_ON
} FEATURE_LC_PER_STAT;

typedef enum
{
  CLIENT_NO_ACTIVE = 0,
  CLIENT_ACTIVE
} STATUS_LC_ACT;

typedef enum
{
  PERIODIC_UP_NO_ACTIVE = 0,
  PERIODIC_UP_ACTIVE
} STATUS_LC_PER_UP;

typedef enum
{
  CLOM_LCS_UNKNOWN = -1,
  CLOM_LCS_NO_ACT,
  CLOM_LCS_ACT,
  CLOM_LCS_CLIENT_DEL,
  CLOM_LCS_CLIENT_NEW,
  CLOM_LCS_SET_MAX
} CLOM_SETTINGS;

typedef enum
{
  CLPS_LCS_UNKNOWN = -1,
  CLPS_LCS_NO_ACT,
  CLPS_LCS_ACT,
  CLPS_LCS_SET_MAX
} CLPS_SETTINGS;

typedef enum
{
  CLRS_NOTIFY_UNKNOWN = -1,
  CLRS_NO_USER_NOTIFY,
  CLRS_USER_NOTIFY,
  CLRS_NOTIFY_SET_MAX
} CLRS_NOTIFY_SETTINGS;

typedef enum
{
  CLRS_CONF_UNKNOWN = -1,
  CLRS_NO_USER_CONFIRM,
  CLRS_USER_CONFIRM,
  CLRS_CONFIRM_SET_MAX
} CLRS_CONFIRMATION_SETTINGS;

typedef struct
{
  UBYTE position_data[MAX_POSITION_DATA];
  UBYTE pos_data_length;
}T_LOC_POS_DATA;

typedef struct
{
  char          address[MAX_SMS_ADDR_DIG];
  T_ACI_TOA     toa;        /* type of address */
}T_LOC_MLC_ADDRESS;

typedef struct T_LOC_SERV_CL
{
  T_LOC_MLC_ADDRESS client_id;      /* LC client identifier */
/* common name of the location-based service / currently not supported*/
  char          lc_service_name[MAX_LC_SERV_NAME];
  UBYTE  client_status;                      /* activ/passiv */
  UBYTE  period_upd_status;            /* activ/passiv */
  USHORT period_upd_value;           /* periodic update value in minutes */
  UBYTE  period_upd_timer;
/* service_type -> bit0 = 0 then client_id is a real telephone number
                   bit0 = 1 then client_id is only a reference number
   additional features -> bit 1 - 7/ currently not supported */
  UBYTE  servid_and_features;
  BOOL   notify;                                  /* notify status */
  BOOL   confirmation;                        /* confirmation status */
  USHORT lc_clientReference;              /* MNLC reference for client */
  struct T_LOC_SERV_CL * next;       /* pointer to next client entry */
}T_LOC_SERV_CLIENT;

typedef struct
{
  UBYTE lc_src_id;               /* location service source identifier */
  T_LOC_MLC_ADDRESS mlcsrc;      /* mobile location service center source address */
  T_LOC_MLC_ADDRESS mlcdest;     /* mobile location service center destination address */
  USHORT lc_callReference;       /* MNLC reference (clsa, sms) */
  UBYTE numb_lc_clients;         /* number of client entries */
  T_LOC_SERV_CLIENT * clients;   /* pointer to list of LC clients */
}T_LOC_SERV_PARA;
#endif

typedef enum
{
  CNAP_SERVICE_STATUS_NOT_PRESENT = -1,
  CNAP_SERVICE_NOT_PROVISIONED,
  CNAP_SERVICE_PROVISIONED,
  CNAP_SERVICE_STATUS_UNKNOWN
} T_ACI_CNAP_STATUS;

#define MAX_NUM_REPEAT_ATT 10    /*  max number of call repeat attempts */
#ifdef _SIMULATION_
#define MAX_NUM_ENTR_BLACK_LIST 4 /* max number of black list phone numbers,
                                     reduced for testcases since lack of 1600 partitions
                                     for AT+CLCC in Testcase ACICC301 */
#else
#define MAX_NUM_ENTR_BLACK_LIST 10 /* max number of black list phone numbers */
#endif /*_SIMULATION_*/
#define RDL_TIM_INDEX_NOT_PRESENT 0xff

typedef enum
{
  AUTOM_REP_NOT_PRESENT = -1,   /* parameter is not given */
  AUTOM_REPEAT_OFF,             /* automatic call repeat is switched off */
  AUTOM_REPEAT_ON               /* automatic call repeat is switched on */
} T_ACI_CC_REDIAL_MODE;

typedef enum
{
  NOTIF_NO_PRESENT = -1,        /* parameter is not given */
  NO_NOTIF_USER,                /* no notification to user */
  NOTIF_USER                    /* notification to user */
} T_ACI_CC_REDIAL_NOTIF;

typedef enum
{
  BLMODE_NO_PRESENT = -1,        /* parameter is not given */
  BL_NO_DELETE,
  BL_DELETE                      /* blacklist is deleted */
}T_ACI_CC_REDIAL_BLMODE;

typedef enum
{
  REDIAL_TIM_START = 2,          /* redial timer starts */
  REDIAL_ATT_START,              /* start redialling attempt */
  CALL_ATTEMPT_FAILED,
  CALL_ATTEMPT_SUCCESSFUL,
  REDIAL_STOP                    /* redialling finished */
}T_ACI_CC_REDIAL_STATE;

typedef enum
{
  BLACKLIST_FULL = 2,             /* black list is full */
  ENTRY_BLACKLISTED,              /* phone number set in black list */
  ENTRY_IN_BLACKLIST              /* phone number is in black list */
}T_ACI_CC_RDL_BLACKL_STATE;

typedef enum
{
  READ_RDLmode = 0,              /* read redial mode from FFS */
  WRITE_RDLmode                  /* write redial mode in FFS */
}T_ACI_CC_RDL_FFS;

typedef struct
{
  UBYTE         numb_len;
  CHAR          number[MNCC_MAX_CC_CALLED_NUMBER];
  T_ACI_TOA     type;
}T_ACI_CC_BLACKL_ENTRY;

typedef struct CCblacklist
{
  UBYTE blCount;                  /* counter of black list entries */
  T_ACI_CC_BLACKL_ENTRY blNum[MAX_NUM_ENTR_BLACK_LIST]; /* table with blacklisted phone number */
}T_ACI_CC_REDIAL_BLACKL;

typedef struct
{
  T_ACI_CC_REDIAL_MODE rdlMod;          /* redial mode */
  T_ACI_CC_REDIAL_BLMODE rdlBlMod;      /* black list mode */
  SHORT rdlcId;                         /* redial call identifier, NO_ENTRY if no redial */
  BOOL rdlState;                        /* redial state */
  T_ACI_CC_REDIAL_NOTIF rdlModN;        /* notification state of redial procedure */
#ifdef TI_PS_FF_AT_P_CMD_RDLB
  T_ACI_CC_REDIAL_NOTIF rdlBlN;         /* notification state of black list */
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
}T_ACI_CC_REDIAL_PAR;

/* ------ +CIND & +CMER - typedefs ------------------------------- */
typedef enum
{
  CIND_SIGNAL_INDICATOR_INVALID = -1,
  CIND_SIGNAL_INDICATOR_LVL0,
  CIND_SIGNAL_INDICATOR_LVL1,
  CIND_SIGNAL_INDICATOR_LVL2,
  CIND_SIGNAL_INDICATOR_LVL3,
  CIND_SIGNAL_INDICATOR_LVL4,
  CIND_SIGNAL_INDICATOR_LVL5,
  CIND_SIGNAL_INDICATOR_TYPE_MAX
} T_ACI_CIND_SIGNAL_TYPE;

typedef enum
{
  CIND_SMSFULL_INDICATOR_INVALID = -1,
  CIND_SMSFULL_INDICATOR_MEMAVAIL,
  CIND_SMSFULL_INDICATOR_MEMFULL,
  CIND_SMSFULL_INDICATOR_MAX
} T_ACI_CIND_SMSFULL_TYPE;

typedef struct
{
  T_ACI_CIND_SIGNAL_TYPE  sCindSignalParam;
  T_ACI_CIND_SMSFULL_TYPE sCindSmsFullParam;
} T_ACI_MM_CIND_VAL_TYPE;

typedef enum
{
  CMER_MODE_INVALID = -1,
  CMER_MODE_0,
  CMER_MODE_1,
  CMER_MODE_2,
  CMER_MODE_TYPE_MAX
} T_ACI_CMER_MODE_TYPE;

typedef enum
{
  CMER_INDICATOR_INVALID = -1,
  CMER_INDICATOR_0,
  CMER_INDICATOR_1,
  CMER_INDICATOR_2,
  CMER_INDICATOR_TYPE_MAX
} T_ACI_CMER_IND_TYPE;

typedef enum
{
  CMER_BFR_INVALID = -1,
  CMER_BFR_0,
  CMER_BFR_1,
  CMER_BFR_TYPE_MAX
} T_ACI_CMER_BFR_TYPE;

typedef struct
{
  T_ACI_CMER_MODE_TYPE sCmerModeParam;
  T_ACI_CMER_IND_TYPE  sCmerIndParam;
  T_ACI_CMER_BFR_TYPE  sCmerBfrParam;
} T_ACI_MM_CMER_VAL_TYPE;

typedef struct
{
   T_ACI_MM_CIND_VAL_TYPE sMmCINDSettings;
   T_ACI_MM_CMER_VAL_TYPE sMmCMERSettings;
} T_ACI_IND_MODE ;

/* ------ %CSCN - typedefs -------------------------------------- */
/* PSA facility function parameter enums */
typedef enum
{
  CSCN_FACILITY_DIRECTION_INVALID = -1,
  CSCN_FACILITY_DIRECTION_IN = 0,
  CSCN_FACILITY_DIRECTION_OUT,
  CSCN_FACILITY_DIRECTION_BOTH,
  CSCN_FACILITY_DIRECTION_MAX
} T_ACI_FAC_DIR;

typedef enum
{
  CSCN_FACILITY_TRANS_TYPE_BEGIN = 0,
  CSCN_FACILITY_TRANS_TYPE,
  CSCN_FACILITY_TRANS_TYPE_END,
  CSCN_FACILITY_TRANS_TYPE_MAX
} T_ACI_FAC_TRANS_TYPE;

/* CC shared parameter types */
typedef enum
{
  CC_CSCN_MOD_STATE_INVALID = -1,
  CC_CSCN_MOD_STATE_OFF,
  CC_CSCN_MOD_STATE_ON,
  CC_CSCN_MOD_STATE_MAX
} T_ACI_CC_CSCN_MOD_STATE;

typedef enum
{
  CC_CSCN_MOD_DIR_INVALID = -1,
  CC_CSCN_MOD_DIR_IN,
  CC_CSCN_MOD_DIR_OUT,
  CC_CSCN_MOD_DIR_BOTH,
  CC_CSCN_MOD_DIR_MAX
} T_ACI_CC_CSCN_MOD_DIRECTION;

typedef struct
{
   T_ACI_CC_CSCN_MOD_STATE     CcCSCNModeState;
   T_ACI_CC_CSCN_MOD_DIRECTION CcCSCNModeDirection;
} T_ACI_CC_CSCN_MOD;

/* SS shared parameter types */
typedef enum
{
  SS_CSCN_MOD_STATE_INVALID = -1,
  SS_CSCN_MOD_STATE_OFF,
  SS_CSCN_MOD_STATE_ON,
  SS_CSCN_MOD_STATE_MAX
} T_ACI_SS_CSCN_MOD_STATE;

typedef enum
{
  SS_CSCN_MOD_DIR_INVALID = -1,
  SS_CSCN_MOD_DIR_IN,
  SS_CSCN_MOD_DIR_OUT,
  SS_CSCN_MOD_DIR_BOTH,
  SS_CSCN_MOD_DIR_MAX
} T_ACI_SS_CSCN_MOD_DIRECTION;

typedef struct
{
   T_ACI_SS_CSCN_MOD_STATE     SsCSCNModeState;
   T_ACI_SS_CSCN_MOD_DIRECTION SsCSCNModeDirection;
} T_ACI_SS_CSCN_MOD;

/* ------ %CPRI - typedefs -------------------------------------- */

typedef enum              /* ciphering indication state */
{
  CI_DONT_SHOW = 0,       /* don't show CI, CI enabled */
  CI_SHOW,                /* show CI, CI enabled */
  CI_DISABLED             /* CI disabled */
} T_CI_STAT;

/* ------ %CPRSM - typedefs -------------------------------------- */

typedef enum
{
  CPRSM_MOD_NotPresent    = -1,
  CPRSM_MOD_Resume        =  0, 
  CPRSM_MOD_Pause         =  1 
} T_ACI_CPRSM_MOD;

/* ------ %CCUST - typedefs -------------------------------------- */

typedef enum {
  CUST_NORMAL_BEHAVIOUR  = 0,
  CUST_MODE_BEHAVIOUR_1
} T_CUST_MOD;

/* ------ %SATCC - typedefs -------------------------------------- */

typedef enum {
  SATCC_CONTROL_BY_SIM_INACTIVE  = 0,
  SATCC_CONTROL_BY_SIM_ACTIVE
} T_SAT_CC_MOD;


/* ------ %SECS - typedefs -------------------------------------- */


typedef enum
{
  SECS_STA_NotPresent    = -1,
  SECS_STA_Disable        =  0, 
  SECS_STA_Enable         =  1 
} T_ACI_SECS_STA;

/*--------The following are stucture typedefs and bitfield definis specifically used for AT+NRG=?------*/

typedef enum
{
  NRG_REG_NotPresent = 0,
  NRG_REG_Auto = 1,
  NRG_REG_Manual = 2,
  NRG_REG_Dereg = 4,
  NRG_REG_SetOnly = 8,
  NRG_REG_Both = 16
}
T_ACI_NRG_REG;

typedef enum
{
  NRG_SRV_NotPresent = 0,
  NRG_SRV_Full = 1,
  NRG_SRV_Limited = 2,
  NRG_SRV_NoSrv = 4,
  NRG_SRV_SetRegModeOnly = 8
}
T_ACI_NRG_SRV;

typedef enum
{
  NRG_OPR_NotPresent = 0,
  NRG_OPR_Long = 1,
  NRG_OPR_Short = 2,
  NRG_OPR_Numeric = 4
}
T_ACI_NRG_OPR;

typedef struct
{
  T_ACI_NRG_REG reg_mode;
  T_ACI_NRG_SRV srv_mode;
  T_ACI_NRG_OPR opr_frmt;
}
T_ACI_NRG;


/*------End of typedefs for AT+NRG=?---------------------------------*/

/*   %COPN typedefs    */

typedef enum
{
  Read_ROM_TABLE = 0,
  Read_EONS = 1,
  Read_CPHS = 2,
  Read_INVALID = 255
}
T_ACI_ORIGIN_READ;


/* -------- %SATN Control By SIM Type indications ------------------*/

typedef enum
{
        SATN_CNTRL_BY_SIM_Not_Present = -1,
        SATN_CNTRL_BY_SIM_CALL = 0,
        SATN_CNTRL_BY_SIM_SS = 1,
        SATN_CNTRL_BY_SIM_USSD = 2,
        SATN_CNTRL_BY_SIM_SMS = 3
} T_ACI_SATN_CNTRL_TYPE;

/* %CUSCFG */

typedef enum
{
  CUSCFG_FAC_Not_Present = 0,
  CUSCFG_FAC_MO_SM_Control,
  CUSCFG_FAC_MO_Call_Control,
  CUSCFG_FAC_MO_SS_Control,
  CUSCFG_FAC_MO_USSD_Control,
  CUSCFG_FAC_2_Digit_Call,
  CUSCFG_FAC_Ext_USSD_Res,
  CUSCFG_FAC_T_MOBILE_Eons,
  CUSCFG_FAC_USSD_As_MO_Call
} T_ACI_CUSCFG_FAC;

typedef enum
{
  CUSCFG_MOD_Not_Present = -1,
  CUSCFG_MOD_Disable,
  CUSCFG_MOD_Enable,
  CUSCFG_MOD_Query
} T_ACI_CUSCFG_MOD;

typedef enum
{
  CUSCFG_STAT_Not_present = -1,
  CUSCFG_STAT_Disabled,
  CUSCFG_STAT_Enabled
} T_ACI_CUSCFG_STAT;

typedef struct
{
  UBYTE MO_SM_Control_SIM;
  UBYTE MO_Call_Control_SIM;
  UBYTE MO_SS_Control_SIM;
  UBYTE MO_USSD_Control_SIM;
  UBYTE Two_digit_MO_Call;
  UBYTE Ext_USSD_Response;
  UBYTE T_MOBILE_Eons;
  UBYTE USSD_As_MO_Call;
} T_ACI_CUSCFG_PARAMS;

typedef enum
{
  CUSDR_RES_Not_Present = -1,
  CUSDR_RES_Ok,
  CUSDR_RES_Unknown_Alphabet,
  CUSDR_RES_Busy
} T_ACI_CUSDR_RES;

#ifdef FF_CPHS_REL4
/*--- %CFIS :Call Forward Indication status------------------------*/

typedef enum
{
  CFIS_MOD_NotPresent  = -1,
  CFIS_MOD_Delete,
  CFIS_MOD_Write,
  CFIS_MOD_Read
}T_ACI_CFIS_MOD;

typedef struct
{
  UBYTE             mspId;
  UBYTE             cfuStat;
  CHAR              number[MAX_PHB_NUM_LEN];
  T_ACI_TOA         type;
}T_ACI_CFIS_CFU;

/*--- %MWIS :Message Waiting Indication Status------------------------*/

typedef enum
{
  MWIS_MOD_Invalid  = -1,
  MWIS_MOD_Delete,
  MWIS_MOD_Write,
  MWIS_MOD_Read
}T_ACI_MWIS_MOD;

typedef struct
{
  UBYTE      mwiStat;       /* Message Waiting Indication Status    */
  UBYTE      mwis_count_voice;    /* number of voicemail messages waiting */
  UBYTE      mwis_count_fax;      /* number of fax messages waiting       */
  UBYTE      mwis_count_email;    /* number of email messages waiting     */
  UBYTE      mwis_count_other;    /* number of other messages waiting     */
}T_ACI_MWIS_MWI;


/* MBI and MBDN */

typedef enum
{
  MBN_Mode_Invalid = -1,
  MBN_Mode_Delete,
  MBN_Mode_Write,
  MBN_Mode_Read
} T_ACI_MBN_MODE;

typedef struct
{
  CHAR    alpha[MAX_ALPHA_LEN];
  CHAR    number[MAX_MB_NUM_LEN];
  T_ACI_TOA    type;
} T_ACI_MBDN;

typedef struct
{
  UBYTE mbdn_id_voice;
  UBYTE mbdn_id_fax;
  UBYTE mbdn_id_email;
  UBYTE mbdn_id_other;
}T_ACI_MBI;

#endif /* FF_CPHS_REL4 */

/* Mode for AT%PBCI command */
typedef enum
{
  PBCI_MODE_Not_Present = -1,
  PBCI_MODE_Disable,
  PBCI_MODE_Enable
} T_ACI_PBCI_MODE;

/* Type of operation which changed phonebook entry */
typedef enum 
{
  PHB_ENTRY_DELETED = 0,
  PHB_ENTRY_EDITED,
  PHB_ENTRY_ADDED
} T_PHB_UPDATE_TYPE;

/* Information about changed phonebook entry */
typedef struct 
{
  USHORT               ef_id;
  USHORT               rec_num;
  T_PHB_UPDATE_TYPE    phbUpdateType;
} T_PHB_CHANGED_INFO;

/* CVHU mode information */
typedef enum
{
  CVHU_DropDTR_IGNORED = 0,
  CVHU_DropDTR_ATH_IGNORED,
  CVHU_DropDTR_Same_AndD
} T_ACI_CVHU_MODE;

/*==== PROTOTYPES =================================================*/

EXTERN T_ACI_RETURN sAT_PercentALS( T_ACI_CMD_SRC srcId,
                                    T_ACI_ALS_MOD mode   );
EXTERN T_ACI_RETURN qAT_PercentALS( T_ACI_CMD_SRC srcId,
                                    T_ACI_ALS_MOD *mode  );
EXTERN T_ACI_RETURN tAT_PercentALS( T_ACI_CMD_SRC srcId,
                                    T_ACI_ALS_MOD *ALSmode);
EXTERN T_ACI_RETURN sAT_PlusCFUN  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_CFUN_FUN fun,
                                    T_ACI_CFUN_RST rst );
EXTERN T_ACI_RETURN qAT_PlusCFUN  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_CFUN_FUN *fun );
EXTERN T_ACI_RETURN sAT_PlusCPIN  ( T_ACI_CMD_SRC srcId,
                                    CHAR * pin,
                                    CHAR * newpin );
#ifdef TI_PS_FF_AT_P_CMD_SECP
 EXTERN T_ACI_RETURN sAT_PercentSECP ( T_ACI_CMD_SRC srcId,
                                   CHAR * pin,
                                   CHAR * newpin );
#endif /* TI_PS_FF_AT_P_CMD_SECP */

#ifdef TI_PS_FF_AT_P_CMD_SECS
EXTERN T_ACI_RETURN sAT_PercentSECS ( T_ACI_CMD_SRC srcId,
                                   T_ACI_SECS_STA securityState,
                                   CHAR * code );

EXTERN T_ACI_RETURN qAT_PercentSECS ( T_ACI_CMD_SRC srcId,
                                                                           T_ACI_SECS_STA *status);
#endif /* TI_PS_FF_AT_P_CMD_SECS */

#ifdef FF_DUAL_SIM
EXTERN T_ACI_RETURN sAT_PercentSIM  ( T_ACI_CMD_SRC  srcId,
                                      UBYTE sim_num);
EXTERN T_ACI_RETURN qAT_PercentSIM  ( T_ACI_CMD_SRC  srcId,
                                      UBYTE *sim_num );
#endif /*FF_DUAL_SIM*/
EXTERN T_ACI_RETURN qAT_PlusCPIN   (T_ACI_CMD_SRC srcId,
                                    T_ACI_CPIN_RSLT *code);
EXTERN T_ACI_RETURN qAT_PlusCREG   ( T_ACI_CMD_SRC srcId,
                                     T_ACI_CREG_STAT * stat,
                                     USHORT          *lac,
                                     USHORT          *cid);
EXTERN T_ACI_RETURN qAT_PercentCREG ( T_ACI_CMD_SRC         srcId,
                                      T_ACI_CREG_STAT       *stat,
                                      USHORT                *lac,
                                      USHORT                *ci,
                                      T_ACI_P_CREG_GPRS_IND *gprs_ind,
                                      U8              *rt);
EXTERN T_ACI_RETURN sAT_PlusCOPS  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_COPS_MOD  mode,
                                    T_ACI_COPS_FRMT format,
                                    CHAR * oper );
EXTERN T_ACI_RETURN qAT_PlusCOPS  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_COPS_MOD * mode,
                                    T_ACI_COPS_FRMT * format,
                                    CHAR * oper);
EXTERN T_ACI_RETURN tAT_PlusCOPS  ( T_ACI_CMD_SRC srcId,
                                    SHORT startIdx,
                                    SHORT * lastIdx,
                                    T_ACI_COPS_OPDESC * operLst);

EXTERN T_ACI_RETURN tAT_PercentCOPS  ( T_ACI_CMD_SRC srcId,
                                    SHORT startIdx,
                                    SHORT * lastIdx,
                                    T_ACI_COPS_OPDESC * operLst);

EXTERN T_ACI_RETURN sAT_PercentCOPS  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_COPS_MOD  mode,
                                    T_ACI_COPS_FRMT format,
                                    CHAR * oper );
EXTERN T_ACI_RETURN qAT_PercentCOPS ( T_ACI_CMD_SRC srcId,
                                   T_ACI_COPS_MOD * mode,
                                   T_ACI_COPS_FRMT * format,
                                   T_ACI_COPS_SVST * svrStatus,
                                   CHAR * oper );
EXTERN T_ACI_RETURN sat_Plus_Percent_COPS ( T_ACI_CMD_SRC srcId,
                                   T_ACI_COPS_MOD mode,
                                   T_ACI_COPS_FRMT format,
                                   CHAR * oper,
                                   T_ACI_AT_CMD cmd);


EXTERN T_ACI_RETURN sAT_PlusCPOL  ( T_ACI_CMD_SRC srcId,
                                    SHORT index,
                                    T_ACI_CPOL_FRMT format,
                                    CHAR * oper,
                                    SHORT index2,
                                    T_ACI_CPOL_MOD mode );
EXTERN T_ACI_RETURN qAT_PlusCPOL  ( T_ACI_CMD_SRC srcId,
                                    SHORT              startIdx,
                                    SHORT             *lastIdx,
                                    T_ACI_CPOL_OPDESC *operLst,
                                    T_ACI_CPOL_MOD     mode );
EXTERN T_ACI_RETURN tAT_PlusCPOL  ( T_ACI_CMD_SRC srcId,
                                    SHORT * lastIdx,
                                    SHORT * usdNtry );
EXTERN T_ACI_RETURN qAT_PlusCLIP  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CLIP_STAT * stat);
EXTERN T_ACI_RETURN sAT_PlusCLIR  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CLIR_MOD mode );
EXTERN T_ACI_RETURN qAT_PlusCLIR  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CLIR_MOD * mode,
                                    T_ACI_CLIR_STAT * stat);
EXTERN T_ACI_RETURN qAT_PercentCLIR  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CLIR_MOD * mode);
EXTERN T_ACI_RETURN qAT_PlusCOLP  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_COLP_STAT * stat);
EXTERN T_ACI_RETURN sAT_PercentCTTY (T_ACI_CMD_SRC srcId,
                                     T_ACI_CTTY_MOD mode,
                                     T_ACI_CTTY_REQ req);
EXTERN T_ACI_RETURN qAT_PercentCTTY (T_ACI_CMD_SRC srcId,
                                     T_ACI_CTTY_MOD *mode,
                                     T_ACI_CTTY_REQ *req,
                                     T_ACI_CTTY_STAT *stat,
                                     T_ACI_CTTY_TRX *trx);
EXTERN T_ACI_RETURN sAT_PlusCMOD  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CMOD_MOD mode );
EXTERN T_ACI_RETURN qAT_PlusCMOD  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CMOD_MOD * mode );
#ifdef FAX_AND_DATA
EXTERN T_ACI_RETURN sAT_PlusCBST  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_BS_SPEED speed,
                                    T_ACI_CBST_NAM name,
                                    T_ACI_CBST_CE ce);
EXTERN T_ACI_RETURN qAT_PlusCBST  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_BS_SPEED * speed,
                                    T_ACI_CBST_NAM * name,
                                    T_ACI_CBST_CE * ce);
#endif /* FAX_AND_DATA */
EXTERN T_ACI_RETURN sAT_PlusCRLP  ( T_ACI_CMD_SRC srcId,
                                    SHORT iws, SHORT mws,
                                    SHORT t1, SHORT n2);
EXTERN T_ACI_RETURN qAT_PlusCRLP  ( T_ACI_CMD_SRC srcId,
                                    SHORT* iws, SHORT* mws,
                                    SHORT* t1,  SHORT* n2);
EXTERN T_ACI_RETURN sAT_PlusDS    ( T_ACI_CMD_SRC srcId,
                                    T_ACI_DS_DIR dir,
                                    T_ACI_DS_COMP comp,
                                    LONG maxDict,
                                    SHORT maxStr );
EXTERN T_ACI_RETURN qAT_PlusDS    ( T_ACI_CMD_SRC srcId,
                                    T_ACI_DS_DIR* dir,
                                    T_ACI_DS_COMP* comp,
                                    LONG* maxDict,
                                    SHORT* maxStr );
EXTERN T_ACI_RETURN sAT_PlusGCI  (  T_ACI_CMD_SRC    srcId,
                                    UBYTE            country);
EXTERN T_ACI_RETURN qAT_PlusGCI  (  T_ACI_CMD_SRC    srcId,
                                    UBYTE           *country);
#ifdef NO_ASCIIZ
EXTERN T_ACI_RETURN sAT_Dm        ( T_ACI_CMD_SRC       srcId,
                                    T_ACI_PB_TEXT       *str,
                           T_ACI_PB_STOR       mem,
                           SHORT               index,
                                    T_ACI_D_CLIR_OVRD   clirOvrd,
                                    T_ACI_D_CUG_CTRL    cugCtrl,
                                    T_ACI_D_TOC         callType );
#else  /* ifdef NO_ASCIIZ */
EXTERN T_ACI_RETURN sAT_Dm        ( T_ACI_CMD_SRC       srcId,
                                    CHAR                *str,
                                    T_ACI_PB_STOR       mem,
                                    SHORT               index,
                                    T_ACI_D_CLIR_OVRD   clirOvrd,
                                    T_ACI_D_CUG_CTRL    cugCtrl,
                                    T_ACI_D_TOC         callType );
#endif /* ifdef NO_ASCIIZ */
EXTERN T_ACI_RETURN sAT_Dn        ( T_ACI_CMD_SRC srcId,
                                    CHAR * number,
                                    T_ACI_D_CLIR_OVRD clirOvrd,
                                    T_ACI_D_CUG_CTRL  cugCtrl,
                                    T_ACI_D_TOC       callType );
EXTERN T_ACI_RETURN sAT_A         ( T_ACI_CMD_SRC srcId );
EXTERN T_ACI_RETURN sAT_H         ( T_ACI_CMD_SRC srcId );
EXTERN T_ACI_RETURN sAT_O         ( T_ACI_CMD_SRC  srcId );
EXTERN T_ACI_RETURN sAT_Abort     ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_AT_CMD   cmd);
EXTERN T_ACI_RETURN sAT_end_ussd  ( T_ACI_CMD_SRC  srcId );
EXTERN T_ACI_ERR_DESC qAT_ErrDesc ( void );
EXTERN T_ACI_RETURN sAT_PlusCSTA  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_TOA *    type);
EXTERN T_ACI_RETURN qAT_PlusCSTA  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_TOA *    type);
EXTERN T_ACI_RETURN sAT_PlusCHUP  ( T_ACI_CMD_SRC  srcId);

#ifdef SIM_PERS
/*For %MEPD -for querying ME Personalisation Data... 
Added on 11/03/2005 */

EXTERN T_ACI_RETURN qAT_PercentMEPD( T_ACI_CMD_SRC srcId, 
           T_SUP_INFO *sup_info );
#endif
EXTERN T_ACI_RETURN sAT_PlusCLCK  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_FAC fac,
                                    T_ACI_CLCK_MOD mode,
                                    CHAR  *        passwd,
                                    T_ACI_CLASS    class_type);
EXTERN T_ACI_RETURN qAT_PlusCLCK  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_FAC fac,
                                    T_ACI_CLASS    class_type,
                                    T_ACI_CLSSTAT *clsStat);
/*QAT_PERCENTCLCK add for Simlock in Riviear MFW

Added by Shen,Chao  April 16th, 2003
*/
EXTERN T_ACI_RETURN qAT_PercentCLCK  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_FAC fac,
                                    T_ACI_CLASS    class_type,
                                    T_ACI_CLSSTAT *clsStat,
                                    UBYTE *simClockStat);

EXTERN T_ACI_RETURN sAT_PlusCPWD  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_FAC fac,
                                    CHAR  *        oldpwd,
                                    CHAR  *        newpwd);
EXTERN T_ACI_RETURN sAT_PlusCCFC  ( T_ACI_CMD_SRC   srcId,
                                    T_ACI_CCFC_RSN reason,
                                    T_ACI_CCFC_MOD mode,
                                    CHAR*          number,
                                    T_ACI_TOA*     type,
                                    T_ACI_CLASS    class_type,
                                    CHAR*          subaddr,
                                    T_ACI_TOS*     satype,
                                    SHORT          time);
EXTERN T_ACI_RETURN qAT_PlusCCFC  ( T_ACI_CMD_SRC   srcId,
                                    T_ACI_CCFC_RSN  reason,
                                    T_ACI_CLASS     class_type);
EXTERN T_ACI_RETURN sAT_PlusCCUG  ( T_ACI_CMD_SRC   srcId,
                                    T_ACI_CCUG_MOD  mode,
                                    T_ACI_CCUG_IDX  index,
                                    T_ACI_CCUG_INFO info);
EXTERN T_ACI_RETURN qAT_PlusCCUG  ( T_ACI_CMD_SRC   srcId,
                                    T_ACI_CCUG_MOD  *mode,
                                    T_ACI_CCUG_IDX  *index,
                                    T_ACI_CCUG_INFO *info);
EXTERN T_ACI_RETURN sAT_PlusCTFR  ( T_ACI_CMD_SRC    srcId,
                                    CHAR            *number,
                                    T_ACI_TOA       *type,
                                    CHAR            *subaddr,
                                    T_ACI_TOS       *satype);
EXTERN T_ACI_RETURN sAT_PlusCCWA  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CCWA_MOD   mode,
                                    T_ACI_CLASS      class_type);
EXTERN T_ACI_RETURN qAT_PlusCCWA  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CLASS      class_type);
EXTERN T_ACI_RETURN sAT_PlusVTS   ( T_ACI_CMD_SRC    srcId,
                                    CHAR             dtmf,
                                    T_ACI_VTS_MOD    mode);
EXTERN T_ACI_RETURN sAT_PlusCHLD  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CHLD_MOD   mode,
                                    CHAR            *call);
EXTERN T_ACI_RETURN sAT_PercentCHLD  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CHLD_MOD   mode,
                                    CHAR            *call);
EXTERN T_ACI_RETURN sAT_PlusIPR   ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_BD_RATE  rate);
EXTERN T_ACI_RETURN qAT_PlusIPR   ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_BD_RATE  *rate);
EXTERN T_ACI_RETURN sAT_PlusICF   ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_BS_FRM     format,
                                    T_ACI_BS_PAR     parity);
EXTERN T_ACI_RETURN qAT_PlusICF   ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_BS_FRM    *format,
                                    T_ACI_BS_PAR    *parity);
EXTERN T_ACI_RETURN sAT_PlusIFC   ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_RX_FLOW_CTRL DCE_by_DTE,
                                    T_ACI_RX_FLOW_CTRL DTE_by_DCE );
EXTERN T_ACI_RETURN qAT_PlusIFC   ( T_ACI_CMD_SRC       srcId,
                                    T_ACI_RX_FLOW_CTRL *DCE_by_DTE,
                                    T_ACI_RX_FLOW_CTRL *DTE_by_DCE );
EXTERN T_ACI_RETURN sAT_AndD      ( T_ACI_CMD_SRC srcId,
                                    UBYTE         value);
EXTERN T_ACI_RETURN qAT_AndD      ( T_ACI_CMD_SRC srcId,
                                    UBYTE         *value);
EXTERN T_ACI_RETURN qAT_PlusCEER  ( T_ACI_CMD_SRC    srcId,
                                    USHORT           *cause);
EXTERN T_ACI_RETURN qAT_PlusCPAS  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CPAS_PAS  *pas);

EXTERN T_ACI_RETURN sAT_PlusCLAE  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CLAE_MOD mode );

EXTERN T_ACI_RETURN qAT_PlusCLAE  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CLAE_MOD * mode);

EXTERN T_ACI_RETURN sAT_PlusCUSD  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_USSD_DATA *str,
                                    SHORT            dcs);
EXTERN T_ACI_RETURN qAT_PlusCAOC  ( T_ACI_CMD_SRC    srcId,
                                    LONG            *ccm);
EXTERN T_ACI_RETURN qAT_PlusCLCC  ( T_ACI_CMD_SRC       srcId,
                                    T_ACI_CLCC_CALDESC *calLst);
EXTERN T_ACI_RETURN qAT_PercentCLCC  ( T_ACI_CMD_SRC       srcId,
                                    T_ACI_CLCC_CALDESC *calLst);
EXTERN T_ACI_RETURN qAT_PlusCOPN  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_COPN_LID   lstId,
                                    SHORT            startIdx,
                                    SHORT           *lastIdx,
                                    T_ACI_COPN_OPDESC *operLst);
EXTERN T_ACI_RETURN qAT_PercentCOPN( T_ACI_CMD_SRC  srcId,
                                     T_ACI_COPS_FRMT format,
                                     CHAR *opr,
                                     T_ACI_OPER_NTRY *oper_ntry);
EXTERN T_ACI_RETURN sAT_PlusCACM  ( T_ACI_CMD_SRC    srcId,
                                    CHAR            *passwd);
EXTERN T_ACI_RETURN qAT_PlusCACM  ( T_ACI_CMD_SRC    srcId,
                                    LONG            *acm);
EXTERN T_ACI_RETURN sAT_PlusCAMM  ( T_ACI_CMD_SRC    srcId,
                                    LONG             acmmax,
                                    CHAR            *passwd);
EXTERN T_ACI_RETURN qAT_PlusCAMM  ( T_ACI_CMD_SRC    srcId,
                                    LONG            *acmmax);
EXTERN T_ACI_RETURN sAT_PlusCPUC  ( T_ACI_CMD_SRC    srcId,
                                    CHAR            *cur,
                                    CHAR            *ppu,
                                    CHAR            *passwd);
EXTERN T_ACI_RETURN qAT_PlusCPUC  ( T_ACI_CMD_SRC    srcId,
                                    CHAR            *cur,
                                    CHAR            *ppu);
EXTERN T_ACI_RETURN qAT_PlusCIMI  ( T_ACI_CMD_SRC    srcId,
                                    CHAR            *imsi);
EXTERN T_ACI_RETURN sAT_PlusCSNS  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CSNS_MOD   mode);
EXTERN T_ACI_RETURN qAT_PlusCSNS  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CSNS_MOD  *mode);
#ifdef TI_PS_FF_AT_CMD_WS46
EXTERN T_ACI_RETURN sAT_PlusWS46   (T_ACI_CMD_SRC    srcId,
                                    T_ACI_WS46_MOD   mode );
EXTERN T_ACI_RETURN qAT_PlusWS46   (T_ACI_CMD_SRC    srcId,
                                    T_ACI_WS46_MOD  *mode );
#endif /* TI_PS_FF_AT_CMD_WS46 */
EXTERN T_ACI_RETURN sAT_AndF      ( T_ACI_CMD_SRC srcId,
                                    SHORT         value);
EXTERN T_ACI_RETURN sAT_AndC      ( T_ACI_CMD_SRC srcId,
                                    T_ACI_DCD_MOD value);
EXTERN T_ACI_RETURN qAT_AndC      ( T_ACI_CMD_SRC srcId,
                                    T_ACI_DCD_MOD *value);
EXTERN T_ACI_RETURN sAT_Z         ( T_ACI_CMD_SRC srcId,
                                    SHORT         value);
EXTERN T_ACI_RETURN qAT_PlusCNUM  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_CNUM_MOD mode );
#ifdef FF_CPHS_REL4
EXTERN T_ACI_RETURN sAT_PercentCFIS( T_ACI_CMD_SRC srcId,
                                     T_ACI_CFIS_MOD mode, 
                                     UBYTE index,
                                     UBYTE mspId,
                                     UBYTE cfuStat,
                                     CHAR *number, 
                                     T_ACI_TOA *type,
                                     UBYTE cc2_id );
EXTERN T_ACI_RETURN qAT_PercentCFIS( T_ACI_CMD_SRC srcId,
                                     UBYTE index);
GLOBAL T_ACI_RETURN sAT_PercentMWIS( T_ACI_CMD_SRC srcId,
                                    T_ACI_MWIS_MOD mode, 
                                    UBYTE mspId,
                                    T_ACI_MWIS_MWI *mwis);
EXTERN T_ACI_RETURN qAT_PercentMWIS( T_ACI_CMD_SRC srcId,
                                     UBYTE mspId);
EXTERN T_ACI_RETURN sAT_PercentMBDN( T_ACI_CMD_SRC srcId,
                                     T_ACI_MBN_MODE mode, 
                                     UBYTE index, CHAR* number, 
                                     T_ACI_TOA* type,
                                     UBYTE cc2_id,
                                     T_ACI_PB_TEXT *text);
EXTERN T_ACI_RETURN qAT_PercentMBDN ( T_ACI_CMD_SRC  srcId,
                                      UBYTE index );
EXTERN T_ACI_RETURN qAT_PercentMBI ( T_ACI_CMD_SRC  srcId,
                                     UBYTE index );
#endif /* FF_CPHS_REL4 */
EXTERN T_ACI_RETURN sAT_PlusCRSM  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_CRSM_CMD cmd,
                                    SHORT          fileId,
                                    SHORT          p1,
                                    SHORT          p2,
                                    SHORT          p3,
                                    SHORT          dataLen,
                                    UBYTE         *data   );
EXTERN T_ACI_RETURN sAT_PlusCSIM  ( T_ACI_CMD_SRC  srcId,
                                    USHORT         dataLen,
                                    UBYTE         *data    );
#ifdef TI_PS_FF_AT_P_CMD_ATR
EXTERN T_ACI_RETURN qAT_PercentATR( T_ACI_CMD_SRC  srcId,
                                    UBYTE         *phase,
                                    UBYTE         *atr_len,
                                    UBYTE         *atr_info);
#endif /* TI_PS_FF_AT_P_CMD_ATR */
EXTERN T_ACI_RETURN sAT_PlusCMUX ( T_ACI_CMD_SRC srcId,
                                   UBYTE mode,
                                   UBYTE subset,
                                   UBYTE port_speed,
                                   USHORT N1,
                                   UBYTE T1,
                                   UBYTE N2,
                                   UBYTE T2,
                                   UBYTE T3 );
EXTERN T_ACI_RETURN qAT_PlusCMUX ( T_ACI_CMD_SRC srcId,
                                   UBYTE *mode,
                                   UBYTE *subset,
                                   UBYTE *port_speed,
                                   USHORT *N1,
                                   UBYTE *T1,
                                   UBYTE *N2,
                                   UBYTE *T2,
                                   UBYTE *T3 );
EXTERN T_ACI_RETURN qAT_PercentCAL( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CAL_ENTR  *callTable );
EXTERN T_ACI_RETURN sAT_PercentNRG( T_ACI_CMD_SRC   srcId,
                                    T_ACI_NRG_RGMD  regMode,
                                    T_ACI_NRG_SVMD  srvMode,
                                    T_ACI_NRG_FRMT  oprFrmt,
                                    CHAR           *opr );
EXTERN T_ACI_RETURN qAT_PercentNRG( T_ACI_CMD_SRC   srcId,
                                    T_ACI_NRG_RGMD *regMode,
                                    T_ACI_NRG_SVMD *srvMode,
                                    T_ACI_NRG_FRMT *oprFrmt,
                                    T_ACI_NRG_SVMD *srvStat,
                                    CHAR           *oper);

EXTERN T_ACI_BD_RATE convert_mux_port_speed (UBYTE mux_port_speed);

EXTERN T_ACI_RETURN tAT_PercentNRG( T_ACI_CMD_SRC srcId,
                                    T_ACI_NRG *NRG_options );

/*Added by Shen,Chao for PercentCSQ*/
EXTERN T_ACI_RETURN sAT_PercentCSQ( T_ACI_CMD_SRC   srcId,
                                    T_ACI_CSQ_MODE CSQmode);
#ifdef FF_PS_RSSI
EXTERN T_ACI_RETURN qAT_PercentCSQ( T_ACI_CMD_SRC   srcId,
                                    UBYTE *rssi,
                                    UBYTE *ber,
                                    UBYTE *actlevel,
                                    UBYTE *min_access_level);
#else
EXTERN T_ACI_RETURN qAT_PercentCSQ( T_ACI_CMD_SRC   srcId,
                                    UBYTE *rssi,
                                    UBYTE *ber,
                                    UBYTE *actlevel);
#endif
#ifdef TI_PS_FF_AT_P_CMD_DBGINFO
EXTERN T_ACI_RETURN qAT_PercentDBGINFO(T_ACI_CMD_SRC srcId, 
                                       ULONG param,
                                       USHORT stor,
                                       USHORT *free,
                                       USHORT *alloc);
#endif /* TI_PS_FF_AT_P_CMD_DBGINFO */


EXTERN T_ACI_RETURN sAT_PercentBAND(T_ACI_CMD_SRC   srcId,
                                    T_ACI_BAND_MODE bandMode,
                                    UBYTE           bandType);
EXTERN T_ACI_RETURN tAT_PercentBAND(T_ACI_CMD_SRC   srcId,
                                    T_ACI_BAND_MODE *MaxBandMode,
                                    UBYTE           *AllowedBands);
EXTERN T_ACI_RETURN qAT_PercentBAND(T_ACI_CMD_SRC   srcId,
                                    T_ACI_BAND_MODE *bandMode,
                                    UBYTE           *bandType);
EXTERN T_ACI_RETURN qAT_PercentCOLR( T_ACI_CMD_SRC srcId );
EXTERN T_ACI_RETURN sAT_PercentPVRF( T_ACI_CMD_SRC   srcId,
                                     T_ACI_PVRF_TYPE type,
                                     CHAR * pin,
                                     CHAR * newpin );
EXTERN T_ACI_RETURN qAT_PercentPVRF( T_ACI_CMD_SRC srcId,
                                     SHORT        *pn1Cnt,
                                     SHORT        *pn2Cnt,
                                     SHORT        *pk1Cnt,
                                     SHORT        *pk2Cnt,
                                     T_ACI_PVRF_STAT *ps1,
                                     T_ACI_PVRF_STAT *ps2 );
EXTERN T_ACI_RETURN sAT_PercentSATC( T_ACI_CMD_SRC  srcId,
                                     SHORT          len,
                                     UBYTE        * satCnfg );
EXTERN T_ACI_RETURN qAT_PercentSATC( T_ACI_CMD_SRC  srcId,
                                     SHORT        * len,
                                     UBYTE        * satCnfg );
EXTERN T_ACI_RETURN sAT_PercentSATR( T_ACI_CMD_SRC  srcId,
                                     SHORT          len,
                                     UBYTE        * satCmd );
EXTERN T_ACI_RETURN sAT_PercentSATE( T_ACI_CMD_SRC  srcId,
                                     SHORT          len,
                                     UBYTE        * satCmd );
EXTERN T_ACI_RETURN sAT_PercentSATT( T_ACI_CMD_SRC  srcId,
                                     T_ACI_SATT_CS  cause);
EXTERN T_ACI_RETURN sAT_PercentSIMEF( T_ACI_CMD_SRC srcId,
                                      T_ACI_SIMEF_MODE mode);
EXTERN T_ACI_RETURN qAT_PercentSIMEF( T_ACI_CMD_SRC srcId,
                                      T_ACI_SIMEF_MODE *mode);
EXTERN T_ACI_RETURN sAT_PercentEFRSLT (T_ACI_CMD_SRC srcId,
                                       T_ACI_EFRSLT_RES result);
EXTERN T_ACI_RETURN sAT_PercentPBCF( T_ACI_CMD_SRC srcId,
                                     T_ACI_PBCF_LDN ldn,
                                     T_ACI_PBCF_LRN lrn,
                                     T_ACI_PBCF_LMN lmn );
EXTERN T_ACI_RETURN qAT_PercentPBCF( T_ACI_CMD_SRC srcId,
                                     T_ACI_PBCF_LDN *ldn,
                                     T_ACI_PBCF_LRN *lrn,
                                     T_ACI_PBCF_LMN *lmn );
EXTERN T_ACI_RETURN qAT_PercentCTV ( T_ACI_CMD_SRC    srcId,
                                     LONG            *ctv);
EXTERN T_ACI_RETURN qAT_PercentCAOC( T_ACI_CMD_SRC    srcId,
                                     CHAR            *cur,
                                     CHAR            *val);
EXTERN T_ACI_RETURN qAT_PercentCACM( T_ACI_CMD_SRC    srcId,
                                     CHAR            *cur,
                                     CHAR            *val);
EXTERN T_ACI_RETURN qAT_PercentRPCT( T_ACI_CMD_SRC    srcId,
                                     T_ACI_RPCT_VAL  *rpuct);
EXTERN T_ACI_RETURN sAT_PercentCCBS( T_ACI_CMD_SRC    srcId,
                                     SHORT            idx  );
EXTERN T_ACI_RETURN qAT_PercentCCBS( T_ACI_CMD_SRC    srcId);

EXTERN T_ACI_RETURN qAT_PercentCNAP( T_ACI_CMD_SRC    srcId);

EXTERN T_ACI_RETURN sAT_PercentRDL(T_ACI_CMD_SRC srcId,
                                      T_ACI_CC_REDIAL_MODE redial_mode,
                                      T_ACI_CC_REDIAL_NOTIF notification);
EXTERN T_ACI_RETURN qAT_PercentRDL ( T_ACI_CMD_SRC srcId,
                                        T_ACI_CC_REDIAL_MODE* redial_mode,
                                        T_ACI_CC_REDIAL_NOTIF* notification);
#ifdef TI_PS_FF_AT_P_CMD_RDLB
EXTERN T_ACI_RETURN sAT_PercentRDLB(T_ACI_CMD_SRC srcId,
                                        T_ACI_CC_REDIAL_BLMODE blacklist_mode,
                                        T_ACI_CC_REDIAL_NOTIF notification);
EXTERN T_ACI_RETURN qAT_PercentRDLB ( T_ACI_CMD_SRC srcId,
                                          T_ACI_CC_REDIAL_BLACKL *blackl,
                                          T_ACI_CC_REDIAL_NOTIF* notification);
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
EXTERN T_ACI_RETURN qAT_PercentCSSD  ( T_ACI_CMD_SRC    srcId,
                                       UBYTE           *ss_diag);

#ifdef TI_PS_FF_AT_P_CMD_CUST
EXTERN T_ACI_RETURN sAT_PercentCUST( T_ACI_CMD_SRC srcId,
                                          T_CUST_MOD customisation_mode);
EXTERN T_ACI_RETURN qAT_PercentCUST( T_ACI_CMD_SRC srcId,
                                          T_CUST_MOD *customisation_mode);
#endif /* TI_PS_FF_AT_P_CMD_CUST */
EXTERN T_ACI_RETURN sAT_PercentSATCC( T_ACI_CMD_SRC srcId,
                                          T_SAT_CC_MOD sat_cc_mode);
EXTERN T_ACI_RETURN qAT_PercentSATCC( T_ACI_CMD_SRC srcId,
                                          T_SAT_CC_MOD *sat_cc_mode);
EXTERN void rdlPrm_init(void);
EXTERN void rdlPrm_exit(void);

#ifdef DTI
#if defined(FF_WAP) || defined(FF_PPP) || defined(FF_GPF_TCPIP) || defined (FF_SAT_E) 
/*----------- WAP prototypes -----------------------------------------*/

EXTERN T_ACI_RETURN sAT_PercentWAP ( T_ACI_CMD_SRC srcId, SHORT setflag );

EXTERN T_ACI_RETURN sAT_PercentPPP ( T_ACI_CMD_SRC srcId,
                                     T_ACI_PPP_PROT protocol,
                                     CHAR *login_name,
                                     CHAR *pwd,
                                     T_ACI_PPP_CON  con_type);
GLOBAL T_ACI_RETURN qAT_PercentPPP ( UBYTE srcId,  ULONG *ipaddr,
                                     ULONG *dns1, ULONG * ns2) ;
#endif /* WAP || FF_PPP || FF_GPF_TCPIP || FF_SAT_E */
#endif /* DTI */

/*----------- SMS prototypes -----------------------------------------*/

EXTERN T_ACI_RETURN sAT_PlusCRES  ( T_ACI_CMD_SRC  srcId,
                                    SHORT          profile );
EXTERN T_ACI_RETURN tAT_PlusCRES  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_CRES      *values );
EXTERN T_ACI_RETURN sAT_PlusCSAS  ( T_ACI_CMD_SRC  srcId,
                                    SHORT          profile );
EXTERN T_ACI_RETURN sAT_PlusCMGW_Old  ( T_ACI_CMD_SRC    srcId,
                                    SHORT            index,
                                    CHAR            *address,
                                    T_ACI_TOA       *toa,
                                    T_ACI_SMS_STAT   stat,
                                    UBYTE            msg_ref,
                                    T_ACI_SM_DATA   *data,
                                    CHAR            *sca,
                                    T_ACI_TOA       *tosca,
                                    SHORT            isReply);
EXTERN T_ACI_RETURN sAT_PlusCSMP  ( T_ACI_CMD_SRC    srcId,
                                    SHORT            fo,
                                    SHORT            vprel,
                                    T_ACI_VP_ABS    *vpabs,
                                    T_ACI_VP_ENH    *vpenh,
                                    SHORT            pid,
                                    SHORT            dcs );
EXTERN T_ACI_RETURN qAT_PlusCSMP  ( T_ACI_CMD_SRC    srcId,
                                    SHORT           *fo,
                                    SHORT           *vprel,
                                    T_ACI_VP_ABS    *vpabs,
                                    T_ACI_VP_ENH    *vpenh,
                                    SHORT           *pid,
                                    SHORT           *dcs );
EXTERN T_ACI_RETURN sAT_PlusCSCA  ( T_ACI_CMD_SRC    srcId,
                                    CHAR            *sca,
                                    T_ACI_TOA       *tosca );
EXTERN T_ACI_RETURN qAT_PlusCSCA  ( T_ACI_CMD_SRC    srcId,
                                    CHAR            *sca,
                                    T_ACI_TOA       *tosca );
EXTERN T_ACI_RETURN sAT_PlusCSMS  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CSMS_SERV  service);
EXTERN T_ACI_RETURN qAT_PlusCSMS  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CSMS_SERV *service,
                                    T_ACI_CSMS_SUPP *mt,
                                    T_ACI_CSMS_SUPP *mo,
                                    T_ACI_CSMS_SUPP *bm);
EXTERN T_ACI_RETURN sAT_PlusCPMS  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_SMS_STOR   mem1,
                                    T_ACI_SMS_STOR   mem2,
                                    T_ACI_SMS_STOR   mem3);
EXTERN T_ACI_RETURN qAT_PlusCPMS  ( T_ACI_CMD_SRC    srcId);
EXTERN T_ACI_RETURN qAT_PlusCPMS_ext  ( T_ACI_CMD_SRC srcId, 
                                        UBYTE *sim_total, 
                                        UBYTE *sim_used, 
                                        UBYTE *me_total, 
                                        UBYTE *me_used );
EXTERN T_ACI_RETURN sAT_PlusCMGF  ( T_ACI_CMD_SRC     srcId,
                                    T_ACI_CMGF_MOD    mode);
EXTERN T_ACI_RETURN qAT_PlusCMGF  ( T_ACI_CMD_SRC     srcId,
                                    T_ACI_CMGF_MOD  * mode);
#if defined FF_MMI_RIV
EXTERN T_ACI_RETURN sAT_PercentSMBS( T_ACI_CMD_SRC       srcId,
                                     T_ACI_PERC_SMBS_MOD mode);
EXTERN T_ACI_RETURN qAT_PercentSMBS( T_ACI_CMD_SRC       srcId,
                                     T_ACI_PERC_SMBS_MOD *mode);
#endif /* #if defined FF_MMI_RIV */
EXTERN T_ACI_RETURN sAT_PlusCSCB  ( T_ACI_CMD_SRC     srcId,
                                    T_ACI_CSCB_MOD    mode,
                                    USHORT          * mids,
                                    UBYTE           * dcss);
EXTERN T_ACI_RETURN qAT_PlusCSCB  ( T_ACI_CMD_SRC     srcId,
                                    T_ACI_CSCB_MOD  * mode,
                                    USHORT          * mids,
                                    UBYTE           * dcss);
#ifdef FF_HOMEZONE
EXTERN T_ACI_RETURN sAT_PercentCBHZ ( T_ACI_CMD_SRC  srcId,
                                      T_ACI_CBHZ_MOD mode,
                                      T_ACI_CS       dcs,
                                      UBYTE          timeout);
EXTERN T_ACI_RETURN qAT_PercentCBHZ ( T_ACI_CMD_SRC   srcId,
                                      T_ACI_CBHZ_MOD* mode,
                                      T_ACI_CS*       dcs,
                                      UBYTE*          timeout);
#endif /* FF_HOMEZONE */

EXTERN T_ACI_RETURN sAT_PlusCMGS_Old  ( T_ACI_CMD_SRC     srcId,
                                    CHAR            * da,
                                    T_ACI_TOA       * toda,
                                    T_ACI_SM_DATA   * data,
                                    CHAR            * sca,
                                    T_ACI_TOA       * tosca,
                                    SHORT             isReply);
EXTERN T_ACI_RETURN sAT_PlusCNMI  ( T_ACI_CMD_SRC     srcId,
                                    T_ACI_CNMI_MT     mt,
                                    T_ACI_CNMI_BM     bm,
                                    T_ACI_CNMI_DS     ds);
EXTERN T_ACI_RETURN qAT_PlusCNMI  ( T_ACI_CMD_SRC     srcId,
                                    T_ACI_CNMI_MT   * mt,
                                    T_ACI_CNMI_BM   * bm,
                                    T_ACI_CNMI_DS   * ds);
EXTERN T_ACI_RETURN sAT_PlusCMGL  ( T_ACI_CMD_SRC     srcId,
                                    T_ACI_SMS_STAT    state,
                                    SHORT             startIdx,
                                    T_ACI_SMS_READ    rdMode );
EXTERN T_ACI_RETURN sAT_PlusCNMA  ( T_ACI_CMD_SRC     srcId);

#ifdef REL99
EXTERN T_ACI_RETURN sAT_PercentCMGRS( T_ACI_CMD_SRC    srcId,
                                      T_ACI_CMGRS_MODE mode );

EXTERN T_ACI_RETURN qAT_PercentCMGRS( T_ACI_CMD_SRC    srcId,
                                      UBYTE*           auto_rep_flag );
#endif /* REL99 */

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
EXTERN T_ACI_RETURN sAT_PercentCPRSM ( T_ACI_CMD_SRC  srcId, 
                                       T_ACI_CPRSM_MOD mode);

EXTERN T_ACI_RETURN qAT_PercentCPRSM ( T_ACI_CMD_SRC  srcId );
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
/*----------- New SMS prototypes for concatenated SMS --------------------*/

EXTERN T_ACI_RETURN sAT_PlusCMSS      ( T_ACI_CMD_SRC   srcId,
                                        UBYTE           index,
                                        CHAR*           da,
                                        T_ACI_TOA*      toda );
EXTERN T_ACI_RETURN sAT_PlusCMSS_Gl   ( T_ACI_CMD_SRC   srcId,
                                        UBYTE           index,
                                        CHAR*           da,
                                        T_ACI_TOA*      toda,
                                        T_CMSS_FCT      rplyCB,
                                        T_ERROR_FCT     errorCB );

#if ((defined (MFW) AND !defined (FF_MMI_RIV)) OR defined (_CONC_TESTING_)) AND defined TI_PS_FF_CONC_SMS
EXTERN T_ACI_RETURN sAT_PlusCMGS      ( T_ACI_CMD_SRC   srcId,
                                        CHAR*           da,
                                        T_ACI_TOA*      toda,
                                        T_SM_DATA_EXT*  src_data,
                                        CHAR*           sca,
                                        T_ACI_TOA*      tosca,
                                        SHORT           isReply );
#else
EXTERN T_ACI_RETURN sAT_PlusCMGS      ( T_ACI_CMD_SRC   srcId,
                                        CHAR*           da,
                                        T_ACI_TOA*      toda,
                                        T_ACI_SM_DATA*  src_data,
                                        CHAR*           sca,
                                        T_ACI_TOA*      tosca,
                                        SHORT           isReply );
#endif

EXTERN T_ACI_RETURN sAT_PercentCMGS ( T_ACI_CMD_SRC  srcId,
                                   CHAR*          da,
                                   T_ACI_TOA*     toda,
                                   T_ACI_SM_DATA* src_data,
                                   T_ACI_UDH_DATA* udh_data,
                                   CHAR*          sca,
                                   T_ACI_TOA*     tosca,
                                   SHORT          isReply );

EXTERN T_ACI_RETURN sAT_PlusCMGS_byPort( T_ACI_CMD_SRC srcId,
                  CHAR*     da,
                  T_ACI_TOA*   toda,
                  T_SM_DATA_EXT* src_data,
                  CHAR*     sca,
                  T_ACI_TOA*   tosca,
                  SHORT     isReply,
                  SHORT     isSpPORT,
                  SHORT     destPORT,
                  SHORT     origPORT);
EXTERN T_ACI_RETURN sAT_PlusCMGS_Gl   ( T_ACI_CMD_SRC   srcId,
                                        CHAR*           da,
                                        T_ACI_TOA*      toda,
                                        T_ACI_SM_DATA*  data,
                                        T_ACI_UDH_DATA* udh,
                                        CHAR*           sca,
                                        T_ACI_TOA*      tosca,
                                        SHORT           isReply,
                                        T_CMGS_FCT      rplyCB,
                                        T_ERROR_FCT     errorCB );
EXTERN T_ACI_RETURN sAT_PlusCMGR      ( T_ACI_CMD_SRC   srcId,
                                        UBYTE           index,
                                        T_ACI_SMS_READ  rdMode );
EXTERN T_ACI_RETURN sAT_PlusCMGR_Gl   ( T_ACI_CMD_SRC   srcId,
                                        UBYTE           index,
                                        T_ACI_SMS_READ  rdMode,
                                        T_CMGR_FCT      rplyCB );

#if ((defined (MFW) AND !defined (FF_MMI_RIV)) OR defined (_CONC_TESTING_)) AND defined TI_PS_FF_CONC_SMS
EXTERN T_ACI_RETURN sAT_PlusCMGW      ( T_ACI_CMD_SRC   srcId,
                                        SHORT           index,
                                        CHAR*           address,
                                        T_ACI_TOA*      toa,
                                        T_ACI_SMS_STAT  stat,
                                        UBYTE           msg_ref,
                                        T_SM_DATA_EXT*  src_data,
                                        CHAR*           sca,
                                        T_ACI_TOA*      tosca,
                                        SHORT           isReply );
#else
EXTERN T_ACI_RETURN sAT_PlusCMGW      ( T_ACI_CMD_SRC   srcId,
                                        SHORT           index,
                                        CHAR*           address,
                                        T_ACI_TOA*      toa,
                                        T_ACI_SMS_STAT  stat,
                                        UBYTE           msg_ref,
                                        T_ACI_SM_DATA*  src_data,
                                        CHAR*           sca,
                                        T_ACI_TOA*      tosca,
                                        SHORT           isReply );
#endif
EXTERN T_ACI_RETURN sAT_PercentCMGW ( T_ACI_CMD_SRC  srcId,
                                   SHORT          index,
                                   CHAR*          address,
                                   T_ACI_TOA*     toa,
                                   T_ACI_SMS_STAT stat,
                                   UBYTE          msg_ref,
                                   T_ACI_SM_DATA* src_data,
                                   T_ACI_UDH_DATA* udh_data,
                                   CHAR*          sca,
                                   T_ACI_TOA*     tosca,
                                   SHORT          isReply );

EXTERN T_ACI_RETURN sAT_PlusCMGW_Gl   ( T_ACI_CMD_SRC   srcId,
                                        SHORT           index,
                                        CHAR*           address,
                                        T_ACI_TOA*      toa,
                                        T_ACI_SMS_STAT  stat,
                                        UBYTE           msg_ref,
                                        T_ACI_SM_DATA*  data,
                                        T_ACI_UDH_DATA* udh,
                                        CHAR*           sca,
                                        T_ACI_TOA*      tosca,
                                        SHORT           isReply,
                                        T_CMGW_FCT      rplyCB,
                                        T_ERROR_FCT     errorCB );
EXTERN T_ACI_RETURN sAT_PlusCMGD      ( T_ACI_CMD_SRC   srcId,
                                        UBYTE           index,
                                        UBYTE           status );
EXTERN T_ACI_RETURN sAT_PlusCMGD_Gl   ( T_ACI_CMD_SRC   srcId,
                                        UBYTE           index,
                                        UBYTE           status,
                                        T_CMGD_FCT      rplyCB,
                                        T_ERROR_FCT     errorCB );
EXTERN T_ACI_RETURN sAT_PlusCMGC      ( T_ACI_CMD_SRC   srcId,
                                        SHORT           fo,
                                        SHORT           ct,
                                        SHORT           pid,
                                        SHORT           mn,
                                        CHAR*           da,
                                        T_ACI_TOA*      toda,
                                        T_ACI_CMD_DATA* data );
EXTERN T_ACI_RETURN sAT_PlusCMGC_Gl   ( T_ACI_CMD_SRC   srcId,
                                        SHORT           fo,
                                        SHORT           ct,
                                        SHORT           pid,
                                        SHORT           mn,
                                        CHAR*           da,
                                        T_ACI_TOA*      toda,
                                        T_ACI_CMD_DATA* data,
                                        T_CMGC_FCT      rplyCB );
EXTERN T_ACI_RETURN sAT_PercentCMGMDU ( T_ACI_CMD_SRC   srcId,
                                        UBYTE           index );
EXTERN T_ACI_RETURN sAT_PercentCMGMDU_Gl      ( T_ACI_CMD_SRC   srcId,
                                                UBYTE           index,
                                                T_CMGMDU_FCT      rplyCB );
#if defined (SMS_PDU_SUPPORT)

EXTERN void rCI_Plus_Percent_CMGLPdu       ( T_MNSMS_READ_CNF *mnsms_read_cnf,
                                             T_ACI_AT_CMD cmd);
EXTERN void rCI_Plus_Percent_CMGRPdu       ( T_MNSMS_READ_CNF* mnsms_read_cnf,
                                             T_ACI_AT_CMD cmd);
EXTERN void rCI_PlusCMTPdu        ( T_MNSMS_MESSAGE_IND * mnsms_message_ind );





EXTERN void rCI_PlusCBMPdu        ( T_MMI_CBCH_IND  * mmi_cbch_ind );
EXTERN void rCI_PlusCDSPdu        ( T_MNSMS_STATUS_IND * mnsms_status_ind );
EXTERN void rCI_PlusCMGSPdu       ( T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf);
EXTERN void rCI_PlusCMSSPdu       ( T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf);
EXTERN void rCI_PlusCMGCPdu       ( T_MNSMS_COMMAND_CNF * mnsms_command_cnf);
#ifdef REL99
EXTERN void rCI_PercentCMGRSPdu   ( UBYTE mode,
                                    T_MNSMS_RETRANS_CNF * mnsms_retrans_cnf,
                                    T_MNSMS_SEND_PROG_IND * mnsms_send_prog_ind );
#endif /* REl99 */

GLOBAL T_ACI_RETURN sAT_PlusCMGWPdu ( T_ACI_CMD_SRC  srcId,
                                      UBYTE          stat,
                                      T_ACI_SM_DATA  *pdu);
#endif

#if defined (SMS_PDU_SUPPORT) || defined (SIM_TOOLKIT)
EXTERN T_ACI_RETURN sAT_PlusCMGSPdu ( T_ACI_CMD_SRC  srcId,
                                      T_ACI_SM_DATA  *pdu );
EXTERN T_ACI_RETURN sAT_PlusCMGCPdu ( T_ACI_CMD_SRC   srcId,
                                      T_ACI_SM_DATA   *pdu );
EXTERN T_ACI_RETURN sAT_PlusCNMAPdu ( T_ACI_CMD_SRC srcId,
                                      SHORT         n,
                                      T_ACI_SM_DATA *pdu);
#endif

/*----------- Phonebook managment prototypes -----------------------------------------*/

EXTERN T_ACI_RETURN sAT_PlusCPBS ( T_ACI_CMD_SRC srcId,
                                   T_ACI_PB_STOR mem,
                                   char*        pin2 );
EXTERN T_ACI_RETURN qAT_PlusCPBS ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_PB_STOR* storage,
                                   SHORT*         used,
                                   SHORT*         total );
EXTERN T_ACI_RETURN qAT_PercentCPBS ( T_ACI_CMD_SRC  srcId,
                                      T_ACI_PB_STOR* storage,
                                      SHORT*         used,
                                      SHORT*         total,
                                      SHORT*         first, 
                                      SHORT*         used_ext,
                                      SHORT*         total_ext);
EXTERN T_ACI_RETURN sAT_PlusCPBW ( T_ACI_CMD_SRC  srcId,
                                   SHORT          index,
                                   CHAR          * number,
                                   T_ACI_TOA     * type,
                                   T_ACI_PB_TEXT * text,
                                   T_ACI_VP_ABS  * dateTime );
EXTERN T_ACI_RETURN tAT_PlusCPBW ( T_ACI_CMD_SRC srcId,
                                   SHORT*        firstIdx,
                                   SHORT*        lastIdx,
                                   UBYTE*        nlength,
                                   UBYTE*        tlength );
EXTERN T_ACI_RETURN sAT_PlusCPBR ( T_ACI_CMD_SRC  srcId,
                                   SHORT          startIdx,
                                   SHORT          stopIdx,
                                   SHORT*         lastIdx,
                                   T_ACI_PB_ENTR* pbLst);
/**
 * sAT_PercentCPBR.
 *
 * @param  srcId          Parameter 1.
 * @param  startIdx       Parameter 2.
 * @param  stopIdx        Parameter 3.
 * @param  searchMode        Parameter 4.
 * @param  lastIdx        Parameter 5.
 * @param  pbLst          Parameter 6.
 * @return Type T_ACI_RETURN.

**************************************************
Added by Shen,Chao March.18th.2003
**************************************************
 */

EXTERN T_ACI_RETURN sAT_PercentCPBR ( T_ACI_CMD_SRC srcId,
                                SHORT startIdx,
                                SHORT stopIdx,
                                T_ACI_SR_TYP searchMode,
                                SHORT* lastIdx,
                                T_ACI_PB_ENTR* pbLst );

/* PersentCPBF */
EXTERN T_ACI_RETURN sAT_PercentCPBF ( T_ACI_CMD_SRC srcId,
                                T_ACI_PB_TEXT* findtext,
                                T_ACI_CPBF_MOD mode,
                                T_ACI_SR_TYP searchMode,
                                U8 direction,
                                SHORT* found,
                                T_ACI_PB_ENTR* pbLst );


EXTERN T_ACI_RETURN tAT_PlusCPBR ( T_ACI_CMD_SRC srcId,
                                   SHORT*        firstIdx,
                                   SHORT*        lastIdx,
                                   UBYTE*        nlength,
                                   UBYTE*        tlength );
#ifdef NO_ASCIIZ
EXTERN T_ACI_RETURN sAT_PlusCPBF ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_PB_TEXT  *findtext,
                                   T_ACI_CPBF_MOD mode,
                                   SHORT          *found,
                                   T_ACI_PB_ENTR  *pbLst );
#else  /* ifdef NO_ASCIIZ */
EXTERN T_ACI_RETURN sAT_PlusCPBF ( T_ACI_CMD_SRC  srcId,
                                   CHAR           *findtext,
                                   T_ACI_CPBF_MOD mode,
                                   SHORT          *found,
                                   T_ACI_PB_ENTR  *pbLst );
#endif /* ifdef NO_ASCIIZ */
EXTERN T_ACI_RETURN tAT_PlusCPBF ( T_ACI_CMD_SRC srcId,
                                   UBYTE*        nlength,
                                   UBYTE*        tlength );

/*----------- Engineering mode prototypes -----------------------------------------*/
EXTERN T_ACI_RETURN sAT_PlusCLAE  ( T_ACI_CMD_SRC srcId,
                  T_ACI_CLAE_MOD mode );
EXTERN T_ACI_RETURN qAT_PlusCLAE  ( T_ACI_CMD_SRC srcId,
                  T_ACI_CLAE_MOD * mode);
#ifdef FF_EM_MODE

EXTERN T_ACI_RETURN sAT_PercentEM  ( T_ACI_CMD_SRC srcId,
                                     T_EM_AT_MODE  mode,
                                     T_EM_AT_TYPE  type);
EXTERN T_ACI_RETURN sAT_PercentEMET( T_ACI_CMD_SRC srcId,
                                     UBYTE         type);
EXTERN T_ACI_RETURN sAT_PercentEMETS( T_ACI_CMD_SRC srcId, UBYTE subclass,
                                      ULONG bitm_h, ULONG bitm_l );
#endif /* FF_EM_MODE */

#if defined MFW AND defined TI_PS_FF_AT_P_CMD_MMITEST
/* MMI TEST */
EXTERN T_ACI_RETURN sAT_PercentMMITEST ( T_ACI_CMD_SRC srcId, char *param);
#endif

EXTERN T_ACI_RETURN sAT_PlusCSVM  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_CSVM_MOD mode,
                                    CHAR          *pnumber,
                                    UBYTE          num_len,
                                    T_ACI_TOA     *toa);

EXTERN T_ACI_RETURN qAT_PlusCSVM ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CSVM_MOD* mode,
                                   CHAR          *number,
                                   UBYTE          num_len,
                                   SHORT         *toa_val);

EXTERN T_ACI_RETURN sAT_PlusCLAN ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_LAN_SUP  *lngCode);

EXTERN T_ACI_RETURN qAT_PlusCLAN ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_LAN_SUP* lngCode );

EXTERN T_ACI_RETURN tAT_PlusCLAN  (T_ACI_CMD_SRC    srcId,
                                   SHORT            *lastIdx,
                                   T_ACI_LAN_SUP    *lanlst);

EXTERN T_ACI_RETURN sAT_PercentCPRI( T_ACI_CMD_SRC srcId,
                                     UBYTE mode );

EXTERN T_ACI_RETURN qAT_PercentCPRI( T_ACI_CMD_SRC srcId,
                                 UBYTE *mode );
/* DTI managment */
EXTERN T_ACI_RETURN sAT_PercentDATA (T_ACI_CMD_SRC  srcId,
                                     UBYTE          redir_mode,
                                     CHAR          *des_devname,
                                     UBYTE          des_devno,
                                     UBYTE          des_subno,
                                     CHAR          *dev_cap,
                                     CHAR          *src_devname,
                                     UBYTE          src_devno,
                                     UBYTE          src_subno,
                                     UBYTE          pdp_cid);

EXTERN T_ACI_RETURN qAT_PercentDATA (T_ACI_CMD_SRC  srcId,
                                     UBYTE         *mode,
                                     UBYTE         *cid,
                                     T_DINF_PARAM  *des_param,
                                     T_DINF_PARAM  *src_param);

EXTERN T_ACI_RETURN sAT_PercentDINF (T_ACI_CMD_SRC  srcId,
                                     UBYTE          mode,
                                     T_DINF_PARAM  *device_param);

EXTERN T_ACI_RETURN sAT_PercentCHPL (T_ACI_CMD_SRC   srcId,
                                     T_ACI_OPER_NTRY *oper);

EXTERN T_ACI_RETURN sAT_PlusCTZR ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CTZR_MODE mode);

EXTERN T_ACI_RETURN qAT_PlusCTZR ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CTZR_MODE *mode);

EXTERN T_ACI_RETURN sAT_PlusCTZU ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CTZU_MODE mode);

EXTERN T_ACI_RETURN sAT_PlusCCLK (  T_ACI_CMD_SRC srcId
                                   ,T_ACI_RTC_DATE *date_s
                                   ,T_ACI_RTC_TIME *time_s
                                   ,int timeZone
                                 );

EXTERN T_ACI_RETURN qAT_PlusCCLK (  T_ACI_CMD_SRC srcId
                                   ,T_ACI_RTC_DATE *date_s
                                   ,T_ACI_RTC_TIME *time_s
                                   ,int * timeZone
                                 );

EXTERN T_ACI_RETURN sAT_PercentCTZV ( T_ACI_CMD_SRC srcId,
                                   T_ACI_PCTZV_MODE mode);

EXTERN T_ACI_RETURN sAT_PercentCNIV ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CNIV_MODE mode);

EXTERN T_ACI_RETURN qAT_PlusCTZU ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CTZU_MODE *mode);

EXTERN T_ACI_RETURN qAT_PercentCTZV ( T_ACI_CMD_SRC srcId,
                                   T_ACI_PCTZV_MODE *mode);

EXTERN T_ACI_RETURN qAT_PercentCNIV ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CNIV_MODE *mode);

EXTERN T_ACI_RETURN sAT_PercentCWUP ( T_ACI_CMD_SRC   srcId,
                                      T_ACI_CWUP_TYPE type);

#ifdef TI_PS_FF_AT_P_CMD_CSCN
EXTERN T_ACI_RETURN sAT_PercentCSCN ( T_ACI_CMD_SRC srcId,
                                   T_ACI_SS_CSCN_MOD_STATE     ss_switch,
                                   T_ACI_SS_CSCN_MOD_DIRECTION ss_direction,
                                   T_ACI_CC_CSCN_MOD_STATE     cc_switch,
                                   T_ACI_CC_CSCN_MOD_DIRECTION cc_direction );

EXTERN T_ACI_RETURN qAT_PercentCSCN ( T_ACI_CMD_SRC srcId,
                                   T_ACI_SS_CSCN_MOD_STATE     *ss_switch,
                                   T_ACI_SS_CSCN_MOD_DIRECTION *ss_direction,
                                   T_ACI_CC_CSCN_MOD_STATE     *cc_switch,
                                   T_ACI_CC_CSCN_MOD_DIRECTION *cc_direction );
#endif /* TI_PS_FF_AT_P_CMD_CSCN */
#if defined FF_EOTD
EXTERN T_ACI_RETURN sAT_PlusCLSA ( T_ACI_CMD_SRC srcId,
                                   CHAR*    mlcsc,
                                   CHAR*    mlcda );

EXTERN T_ACI_RETURN qAT_PlusCLSA ( T_ACI_CMD_SRC srcId,
                                   CHAR*    mlcsc,
                                   CHAR*    mlcda);

#endif /* FF_EOTD */
#ifdef FF_EOTD
EXTERN T_ACI_RETURN sAT_PlusCLPS ( T_ACI_CMD_SRC srcId,
                                   CHAR     clpsset,
                                   CHAR*    lcclientId,
                                   USHORT   cltimer);
EXTERN T_ACI_RETURN sAT_PlusCLSR ( T_ACI_CMD_SRC srcId,
                                   CHAR      lcnotify,
                                   CHAR      lcconfirm,
                                   CHAR*    lcclientId);
EXTERN T_ACI_RETURN sAT_PlusCLOM ( T_ACI_CMD_SRC srcId,
                                   CHAR     clomset,
                                   CHAR*    lc_clientId );
EXTERN T_ACI_RETURN qAT_PlusCLOM ( T_ACI_CMD_SRC srcId,
                                   UBYTE        * number_lc_clients,
                                   T_LOC_SERV_PARA  ** client_list);
EXTERN T_ACI_RETURN qAT_PlusCLPS ( T_ACI_CMD_SRC srcId,
                                   UBYTE        * number_lc_clients,
                                   T_LOC_SERV_PARA  ** client_list);

#endif /* FF_EOTD */

EXTERN T_ACI_RETURN sAT_PlusCIND ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_CIND_SIGNAL_TYPE  sCindSgnalSettings,
                                   T_ACI_CIND_SMSFULL_TYPE sCindSmsFullSettings );

EXTERN T_ACI_RETURN qAT_PlusCIND ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_CIND_SIGNAL_TYPE  *sCindSgnalSettings,
                                   T_ACI_CIND_SMSFULL_TYPE *sCindSmsFullSettings );

EXTERN T_ACI_RETURN sAT_PlusCMER ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_CMER_MODE_TYPE sCmerModeSettings,
                                   T_ACI_CMER_IND_TYPE sCmerIndicationSettings,
                                   T_ACI_CMER_BFR_TYPE sCmerBfrSettings );

EXTERN T_ACI_RETURN qAT_PlusCMER ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CMER_MODE_TYPE *sCmerModeSettings,
                                   T_ACI_CMER_IND_TYPE  *sCmerIndicationSettings,
                                   T_ACI_CMER_BFR_TYPE  *sCmerBfrSettings );

#ifdef TI_PS_FF_AT_P_CMD_CUSCFG
EXTERN T_ACI_RETURN sAT_PercentCUSCFG  ( T_ACI_CMD_SRC srcId, 
                                         T_ACI_CUSCFG_FAC facility,
                                         T_ACI_CUSCFG_MOD mode,
                                         CHAR  *        value);

EXTERN T_ACI_RETURN qAT_PercentCUSCFG  ( T_ACI_CMD_SRC    srcId,
                                         T_ACI_CUSCFG_FAC facility,
                                         T_ACI_CUSCFG_STAT *status);
#endif /* TI_PS_FF_AT_P_CMD_CUSCFG */
#ifdef TI_PS_FF_AT_P_CMD_CUSDR
EXTERN T_ACI_RETURN sAT_PercentCUSDR   (T_ACI_CMD_SRC srcId, T_ACI_CUSDR_RES response);
#endif /* TI_PS_FF_AT_P_CMD_CUSDR */

EXTERN void         cmhMM_GetCmerSettings ( T_ACI_CMD_SRC srcId,
                                   T_ACI_MM_CMER_VAL_TYPE *sCmerSettings );

EXTERN T_ACI_RETURN sAT_PlusCMMS ( T_ACI_CMD_SRC srcId,
                                   UBYTE         mode );

EXTERN T_ACI_RETURN qAT_PlusCMMS  ( T_ACI_CMD_SRC srcId,
                                    UBYTE*        mode);

#ifdef TI_PS_FF_AT_P_CMD_STDR
EXTERN T_ACI_RETURN qAT_PercentSTDR  ( T_ACI_CMD_SRC    srcId,
                                       UBYTE           *rvstd);
#endif /* TI_PS_FF_AT_P_CMD_STDR */

EXTERN T_ACI_RETURN sAT_PercentCMGL  ( T_ACI_CMD_SRC     srcId,
                                       T_ACI_SMS_STAT    state,
                                       T_ACI_SMS_READ    rdMode );

EXTERN T_ACI_RETURN sAT_PercentCMGR_Gl   ( T_ACI_CMD_SRC   srcId,
                                        UBYTE           index,
                                        T_ACI_SMS_READ  rdMode,
                                        T_CMGR_FCT      rplyCB );

EXTERN T_ACI_RETURN sAT_PercentCMGR      ( T_ACI_CMD_SRC   srcId,
                                           UBYTE           index,
                                           T_ACI_SMS_READ  rdMode );
 
#ifdef TI_PS_FF_AT_CMD_P_ECC
EXTERN T_ACI_RETURN sAT_PercentECC ( T_ACI_CMD_SRC srcId,
                                     U8 index,
                                     char *ecc_number);

EXTERN void  cmhCC_additional_ecc_numbers_initialize(void);
EXTERN BOOL  cmhCC_isNrInAdditionalECC(char *number);
#endif /* TI_PS_FF_AT_CMD_P_ECC */

#ifdef TI_PS_FFS_PHB
EXTERN SHORT  cmh_Query_free_ext_record(void);

EXTERN void cmh_PHB_update_ext_record(UBYTE rec_num,BOOL flag);
#endif

#ifdef TI_PS_FF_AT_P_CMD_CTREG
/*------- %CTREG --------------------------------------*/

EXTERN T_ACI_RETURN sAT_PercentCTREG (T_ACI_CMD_SRC srcId,T_TREG *treg );
EXTERN T_ACI_RETURN qAT_PercentCTREG (T_ACI_CMD_SRC srcId,T_TREG *treg );
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

/*--------------------------- %PBCI------------------------------------------*/
EXTERN T_ACI_RETURN sAT_PercentPBCI (T_ACI_CMD_SRC srcId,
                                     T_ACI_PBCI_MODE mode );
EXTERN T_ACI_RETURN qAT_PercentPBCI (T_ACI_CMD_SRC srcId,
                                     T_ACI_PBCI_MODE *mode );
/*--------------------------- %PBCI------------------------------------------*/

EXTERN T_ACI_RETURN sAT_PlusCVHU ( T_ACI_CMD_SRC srcId, T_ACI_CVHU_MODE  mode);
EXTERN T_ACI_RETURN qAT_PlusCVHU ( T_ACI_CMD_SRC srcId, T_ACI_CVHU_MODE *mode);

/*------- call-backs for MMI --------------------------------------*/

#if defined ACI OR defined SMI OR defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
#if defined CMH_F_C /*lint -save -e18 */ /* Turn off Lint errors for this "construct" */

EXTERN void rAT_OK            ( void );
EXTERN void rAT_NO_CARRIER    ( void );
EXTERN void rAT_CONNECT       ( void );
EXTERN void rAT_BUSY          ( void );
EXTERN void rAT_NO_ANSWER     ( void );
EXTERN void rAT_PlusCME       ( void );
EXTERN void rAT_PlusCMS       ( void );

EXTERN void rAT_PercentCOPS   ( void );
EXTERN void rAT_PlusCPIN      ( void );
EXTERN void rAT_PlusCREG      ( void );
EXTERN void rAT_PercentCREG   ( void );
#ifdef REL99
EXTERN void rAT_PercentCMGRS  ( void );
#endif /* REl99 */
EXTERN void rAT_PlusCRING     ( void );
EXTERN void rAT_PlusCRING_OFF ( void );
EXTERN void rAT_PlusCLIP      ( void );
EXTERN void rAT_PlusCDIP      ( void );
EXTERN void rAT_PlusCLIR      ( void );
EXTERN void rAT_PlusCOLP      ( void );
EXTERN void rAT_PlusDR        ( void );
EXTERN void rAT_PlusCR        ( void );
EXTERN void rAT_PlusCLCK      ( void );
EXTERN void rAT_PlusCCFC      ( void );
EXTERN void rAT_PlusCCWA      ( void );
EXTERN void rAT_PlusCSMS      ( void );
EXTERN void rAT_PlusCPMS      ( void );
EXTERN void rAT_PlusCMS       ( void );
EXTERN void rAT_PlusCBMI      ( void );
EXTERN void rAT_PlusCMT       ( void );
EXTERN void rAT_PlusCMTI      ( void );
EXTERN void rAT_PlusCMGS      ( void );
EXTERN void rAT_PlusCMSS      ( void );
EXTERN void rAT_PlusCMGW      ( void );
EXTERN void rAT_PlusCMGC      ( void );
EXTERN void rAT_PlusCDS       ( void );
EXTERN void rAT_PlusCBM       ( void );
EXTERN void rAT_PlusILRR      ( void );
EXTERN void rAT_PlusCMGR      ( void );
EXTERN void rAT_PlusCMGD      ( void );
EXTERN void rAT_PlusCMGL      ( void );
EXTERN void rAT_PlusCUSD      ( void );
EXTERN void rAT_PlusCCCM      ( void );
EXTERN void rAT_PlusCSSI      ( void );
EXTERN void rAT_PlusCSSU      ( void );
EXTERN void rAT_PercentCSSN   ( void );
EXTERN void rAT_PercentCCCN   ( void );

EXTERN void rAT_PlusCPOL      ( void );
EXTERN void rAT_PlusCLAN      ( void );
EXTERN void rAT_PlusCLAE      ( void );
EXTERN void rAT_PercentCSQ    ( void );

EXTERN void rAT_PlusCIMI      ( void );
EXTERN void rAT_PlusCNUM      ( void );
EXTERN void rAT_PlusCRSM      ( void );
EXTERN void rAT_PlusCSIM      ( void );
EXTERN void rAT_PercentRDL    ( void );
#ifdef TI_PS_FF_AT_P_CMD_RDLB
EXTERN void rAT_PercentRDLB   ( void );
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
EXTERN void rAT_PlusCCWV      ( void );
EXTERN void rAT_PercentBC     ( void );
EXTERN void rAT_PercentDRV    ( void );
EXTERN void rAT_PercentSIMREM ( void );
EXTERN void rAT_PercentSIMINS ( void );
EXTERN void rAT_PercentCOLR   ( void );
EXTERN void rAT_PercentKSIR   ( void );
EXTERN void rAT_PercentCPI    ( void );
EXTERN void rAT_PercentCSTAT  ( void );
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
EXTERN void rAT_PercentCPRSM  ( void );
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
EXTERN void rAT_PercentCTYI   ( void );
EXTERN void rAT_PercentCTV    ( void );
EXTERN void rAT_PercentALS    ( void );
#ifdef SIM_TOOLKIT
EXTERN void rAT_PercentSATI   ( void );
EXTERN void rAT_PercentSATE   ( void );
EXTERN void rAT_PercentSATN   ( void );
EXTERN void rAT_PercentSATA   ( void );
#endif  /* SIM_TOOLKIT */
EXTERN void rAT_PercentCCBS   ( void );
EXTERN void rAT_PercentCNAP   ( void );
EXTERN void rAT_PlusCTZV      ( void );
EXTERN void rAT_SignalSMS     ( void );
/*#ifdef FF_MMI_RIV*/
EXTERN void rAT_PercentCTZV   ( void );
EXTERN void rAT_PercentCNIV (  void);
/*#endif*/
EXTERN void rAT_phb_status    ( void );
EXTERN void rAT_sms_ready     ( void ); /* indicates phonebook read ready */

#ifdef FF_EM_MODE
EXTERN void rAT_PercentEM     ( void );
EXTERN void rAT_PercentEMET   ( void );
EXTERN void rAT_PercentEMETS  ( void );
#endif /* FF_EM_MODE */

EXTERN void rAT_PlusCIEV      ( void );
EXTERN void rAT_Z             ( void );
EXTERN void rAT_PlusCOPS      ( void );

#ifdef FF_WAP
EXTERN void rAT_WAP_PPP_connected(SHORT cId,ULONG IPAddress);

EXTERN void rAT_WAP_start_login(void);
EXTERN void rAT_WAP_start_gprs_login(void);

EXTERN void rAT_WAP_call_disconnected(SHORT cId);
#endif /* WAP */

EXTERN void rAT_PercentCPRI(void);
EXTERN void rAT_PercentSIMEF(void);
EXTERN void rAT_PercentCMGR ( void );
#ifdef FF_CPHS_REL4
EXTERN void rAT_PercentCFIS ( void );

EXTERN void rAT_PercentMWIS ( void );
EXTERN void rAT_PercentMWI  ( void );

EXTERN void rAT_PercentMBI  ( void );
EXTERN void rAT_PercentMBDN ( void );
#endif /* FF_CPHS_REL4 */
EXTERN void rAT_PercentPBCI ( void ); /* indicates change in phonebook */

#ifdef FF_BAT
EXTERN void rBAT_OK              (void);
EXTERN void rBAT_CONNECT         (void);
EXTERN void rBAT_PlusCME         (void);
EXTERN void rBAT_NO_CARRIER      (void);
EXTERN void rBAT_PlusCRING       (void);
EXTERN void rBAT_PlusDR          (void);
EXTERN void rBAT_PlusCR          (void);
EXTERN void rBAT_PlusCPIN        (void);
EXTERN void rBAT_PlusCOPS        (void);
EXTERN void rBAT_PercentCOPS     (void);
EXTERN void rBAT_PlusCREG        (void);
EXTERN void rBAT_PlusCCWA        (void);
EXTERN void rBAT_PlusCLIP        (void);
EXTERN void rBAT_PlusCDIP        (void);
EXTERN void rBAT_PlusCOLP        (void);
EXTERN void rBAT_PlusCRING_OFF   (void);
EXTERN void rBAT_PlusFCO         (void);
EXTERN void rBAT_PlusFIS         (void);
EXTERN void rBAT_PlusFTI         (void);
EXTERN void rBAT_PlusFCS         (void);
EXTERN void rBAT_PlusFCI         (void);
EXTERN void rBAT_PlusCMS         (void);
EXTERN void rBAT_PlusCSMS        (void);
EXTERN void rBAT_PlusCMGS        (void);
EXTERN void rBAT_PlusCMSS        (void);
EXTERN void rBAT_PlusCMGW        (void);
EXTERN void rBAT_PlusCDS         (void);
EXTERN void rBAT_PlusCMGC        (void);
EXTERN void rBAT_PlusCMGD        (void);
EXTERN void rBAT_PlusCMGR        (void);
EXTERN void rBAT_PlusCMGL        (void);
EXTERN void rBAT_PlusCMTI        (void);
EXTERN void rBAT_PlusCMT         (void);
EXTERN void rBAT_PlusCMTI        (void);
EXTERN void rBAT_PlusCMT         (void);
EXTERN void rBAT_PlusCBM         (void);
EXTERN void rBAT_PlusCPMS        (void);
EXTERN void rBAT_PlusFHT         (void);
EXTERN void rBAT_PlusFHR         (void);
EXTERN void rBAT_PlusFSA         (void);
EXTERN void rBAT_PlusFPA         (void);
EXTERN void rBAT_PlusFPW         (void);
EXTERN void rBAT_PlusFET         (void);
EXTERN void rBAT_PlusFVO         (void);
EXTERN void rBAT_PlusFPO         (void);
EXTERN void rBAT_PlusFPI         (void);
EXTERN void rBAT_PlusFNF         (void);
EXTERN void rBAT_PlusFNS         (void);
EXTERN void rBAT_PlusFNC         (void);
EXTERN void rBAT_PlusFHS         (void);
EXTERN void rBAT_PlusFPS         (void);
EXTERN void rBAT_PlusFTC         (void);
EXTERN void rBAT_PlusILRR        (void);
EXTERN void rBAT_BUSY            (void);
EXTERN void rBAT_NO_ANSWER       (void);
EXTERN void rBAT_PercentSIMREM   (void);
EXTERN void rBAT_PlusCLIR        (void);
EXTERN void rBAT_PercentCOLR     (void);
EXTERN void rBAT_PlusCSSI        (void);
EXTERN void rBAT_PlusCSSU        (void);
EXTERN void rBAT_PlusCUSD        (void);
EXTERN void rBAT_PlusCCFC        (void);
EXTERN void rBAT_PlusCLCK        (void);
EXTERN void rBAT_PlusCIMI        (void);
#ifdef SIM_TOOLKIT
EXTERN void rBAT_PercentSATI     (void);
EXTERN void rBAT_PercentSATE     (void);
#endif
EXTERN void rBAT_PercentKSIR     (void);
EXTERN void rBAT_PercentCPI      (void);
EXTERN void rBAT_PercentCTYI     (void);
EXTERN void rBAT_PlusCNUM        (void);
EXTERN void rBAT_PlusCPOL        (void);
EXTERN void rBAT_PlusCCCM        (void);
EXTERN void rBAT_PercentCTV      (void);
#ifdef SIM_TOOLKIT
EXTERN void rBAT_PercentSATN     (void);
EXTERN void rBAT_PercentSATA     (void);
#endif /* SIM_TOOLKIT */
EXTERN void rBAT_sms_ready       (void);
EXTERN void rBAT_phb_status      (void);
EXTERN void rBAT_PercentSIMINS   (void);
EXTERN void rBAT_PlusCRSM        (void);
EXTERN void rBAT_PlusCSIM        (void);
EXTERN void rBAT_PercentCCBS     (void);
EXTERN void rBAT_PlusCCWV        (void);
EXTERN void rBAT_PercentCNAP     (void);
EXTERN void rBAT_SignalSMS       (void);
EXTERN void rBAT_PlusCLAN        (void);
EXTERN void rBAT_PlusCLAE        (void);
EXTERN void rBAT_PercentCSQ      (void);
EXTERN void rBAT_PercentALS      (void);
EXTERN void rBAT_PlusCTZV        (void);
EXTERN void rBAT_PercentCREG     (void);
#ifdef GPRS
EXTERN void rBAT_PlusCGACT       (void);
EXTERN void rBAT_PlusCGDATA      (void);
EXTERN void rBAT_PlusCGANS       (void);
EXTERN void rBAT_PlusCGEREP      (void);
EXTERN void rBAT_PlusCGREG       (void);
EXTERN void rBAT_changedQOS      (void);
EXTERN void rBAT_PercentSNCNT    (void);
EXTERN void rBAT_PercentCGREG    (void);
#endif /* GPRS */
EXTERN void rBAT_PercentEM       (void);
EXTERN void rBAT_PercentEMET     (void);
EXTERN void rBAT_PercentEMETS    (void);
EXTERN void rBAT_PercentCPNUMS   (void);
EXTERN void rBAT_PercentCPVWI    (void);
EXTERN void rBAT_PercentCPROAM   (void);
EXTERN void rBAT_PlusCIEV        (void);
EXTERN void rBAT_PercentRDL      (void);
#ifdef TI_PS_FF_AT_P_CMD_RDLB
EXTERN void rBAT_PercentRDLB     (void);
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
EXTERN void rBAT_PercentCCCN     (void);
EXTERN void rBAT_PercentCSSN     (void);
EXTERN void rBAT_PercentCSTAT    (void);
EXTERN void rBAT_Z               (void);
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
EXTERN void rBAT_PercentCPRSM    (void);
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
EXTERN void rBAT_PercentCTZV     (void);
EXTERN void rBAT_PercentCNIV     (void);
#ifdef GPRS
EXTERN void rBAT_PercentCGEV     (void);
#endif /* GPRS */
EXTERN void rBAT_PercentCPRI     (void);
EXTERN void rBAT_PercentSIMEF    (void);
#endif /* FF_BAT */

EXTERN void rAT_PercentCMGL      ( void );
EXTERN void rBAT_PercentCMGR     (void);
EXTERN void rBAT_PercentCMGL     (void);
#ifdef REL99
EXTERN void rBAT_PercentCMGRS     (void);
EXTERN void rBAT_PlusCGCMOD       (void);
#endif
EXTERN void rBAT_PercentPBCI         (void);
/*lint -restore */
#else /* CMH_F_C */

EXTERN void rAT_OK        ( T_ACI_AT_CMD );
EXTERN void rAT_NO_CARRIER( T_ACI_AT_CMD    cmdId,
                            SHORT cId );
EXTERN void rAT_CONNECT   ( T_ACI_AT_CMD    cmdId,
                            T_ACI_BS_SPEED  speed,
                            SHORT cId );
EXTERN void rAT_BUSY      ( T_ACI_AT_CMD    cmdId,
                            SHORT cId );
EXTERN void rAT_NO_ANSWER ( T_ACI_AT_CMD    cmdId,
                            SHORT cId );
EXTERN void rAT_PlusCME   ( T_ACI_AT_CMD    cmdId,
                            T_ACI_CME_ERR   err );
EXTERN void rAT_PlusCMS   ( T_ACI_AT_CMD    cmdId,
                            T_ACI_CMS_ERR   err,
                            T_EXT_CMS_ERROR * conc_error);
EXTERN void rAT_PlusCOPS  ( SHORT           lastIdx,
                            T_ACI_COPS_OPDESC * operLst);
EXTERN void rAT_PercentCOPS  ( SHORT           lastIdx,
                            T_ACI_COPS_OPDESC * operLst);
EXTERN void rAT_PlusCPIN  ( T_ACI_CPIN_RSLT rslt );
EXTERN void rAT_PlusCREG  ( T_ACI_CREG_STAT status ,
                            USHORT          lac,
                            USHORT          cid );
EXTERN void rAT_PercentCREG  ( T_ACI_CREG_STAT       status ,
                               USHORT                lac,
                               USHORT                cid,
                               T_ACI_P_CREG_GPRS_IND gprs_ind);
EXTERN void rAT_PlusCRING ( T_ACI_CRING_MOD mode,
                            T_ACI_CRING_SERV_TYP type1,
                            T_ACI_CRING_SERV_TYP type2 );
EXTERN void rAT_PlusCRING_OFF ( SHORT cId  );
#ifdef NO_ASCIIZ
EXTERN void rAT_PlusCLIP  ( T_ACI_CLIP_STAT stat,
                            CHAR            * number,
                            T_ACI_TOA       * type,
                            U8                validity,
                            CHAR            * subaddr,
                            T_ACI_TOS       * satype,
                            T_ACI_PB_TEXT   * alpha);
#else  /* ifdef NO_ASCIIZ */
EXTERN void rAT_PlusCLIP  ( T_ACI_CLIP_STAT stat,
                            CHAR            * number,
                            T_ACI_TOA       * type,
                            U8                validity,
                            CHAR            * subaddr,
                            T_ACI_TOS       * satype,
                            CHAR            * alpha);
#endif /* ifdef NO_ASCIIZ */

EXTERN  void rAT_PlusCDIP  ( CHAR   * number,
                                                    T_ACI_TOA       * type,
                                                     CHAR            * subaddr,
                                                     T_ACI_TOS       * satype);


EXTERN void rAT_PlusCLIR  ( T_ACI_CLIR_MOD  mode,
                            T_ACI_CLIR_STAT stat);
#ifdef NO_ASCIIZ
EXTERN void rAT_PlusCOLP  ( T_ACI_COLP_STAT stat,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            CHAR          * subaddr,
                            T_ACI_TOS     * satype,
                            T_ACI_PB_TEXT * alpha);
#else  /* ifdef NO_ASCIIZ */
EXTERN void rAT_PlusCOLP  ( T_ACI_COLP_STAT stat,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            CHAR          * subaddr,
                            T_ACI_TOS     * satype,
                            CHAR          * alpha);
#endif /* ifdef NO_ASCIIZ */
EXTERN void rAT_PlusDR    ( T_ACI_DR_TYP    type );
EXTERN void rAT_PlusCR    ( T_ACI_CRING_SERV_TYP   service);
EXTERN void rAT_PlusCLCK  ( T_ACI_CLSSTAT * clsStat);
EXTERN void rAT_PlusCCFC  ( T_ACI_CCFC_SET* setting);
#ifdef NO_ASCIIZ
EXTERN void rAT_PlusCCWA  ( T_ACI_CLSSTAT * clsStat,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            U8              validity,
                            T_ACI_CLASS     class_type,
                            T_ACI_PB_TEXT * alpha);
#else  /* ifdef NO_ASCIIZ */
EXTERN void rAT_PlusCCWA  ( T_ACI_CLSSTAT * clsStat,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            U8              validity,
                            T_ACI_CLASS     class_type,
                            CHAR          * alpha);
#endif /* ifdef NO_ASCIIZ */
EXTERN void rAT_PlusCSMS  ( T_ACI_CSMS_SERV service,
                            T_ACI_CSMS_SUPP mt,
                            T_ACI_CSMS_SUPP mo,
                            T_ACI_CSMS_SUPP bm);
EXTERN void rAT_PlusCPMS  ( T_ACI_SMS_STOR_OCC * mem1,
                            T_ACI_SMS_STOR_OCC * mem2,
                            T_ACI_SMS_STOR_OCC * mem3);
EXTERN void rAT_PlusCMS   ( T_ACI_AT_CMD    cmdId,
                            T_ACI_CMS_ERR   err,
                            T_EXT_CMS_ERROR * conc_error);
EXTERN void rAT_PlusCBMI  ( T_ACI_SMS_STOR  mem,
                            UBYTE           index);
EXTERN void rAT_PlusCMT   ( T_ACI_CMGL_SM*  sm);

EXTERN void rAT_PlusCMTI  ( T_ACI_SMS_STOR  mem,
                            UBYTE           index,
                            T_ACI_CMGL_SM*  sm);
EXTERN void rAT_PlusCMGS  ( UBYTE           mr,
                            UBYTE           numSeg);
#ifdef REL99
EXTERN void rAT_PercentCMGRS  ( T_ACI_CMGRS_MODE  mode,
                                UBYTE             tp_mr,
                                UBYTE             resend_count,
                                UBYTE             max_retrans );
#endif /* REL99 */
EXTERN void rAT_PlusCMSS  ( UBYTE           mr,
                            UBYTE           numSeg);
EXTERN void rAT_PlusCMGW  ( UBYTE           index,
                            UBYTE           numSeg,
                            UBYTE           mem);
EXTERN void rAT_PlusCMGC  ( UBYTE           mr);
EXTERN void rAT_PlusCDS   ( T_ACI_CDS_SM*   st);

EXTERN void rAT_PlusCDSPdu(T_MNSMS_STATUS_IND * mnsms_status_ind);

EXTERN void rAT_PlusCBM   ( SHORT           sn,
                            SHORT           mid,
                            SHORT           dcs,
                            UBYTE           page,
                            UBYTE           pages,
                            T_ACI_CBM_DATA* data);
EXTERN void rAT_PlusILRR  ( T_ACI_BS_SPEED  speed,
                            T_ACI_BS_FRM    format,
                            T_ACI_BS_PAR    parity);
EXTERN void rAT_PlusCMGR  ( T_ACI_CMGL_SM*  sm,
                            T_ACI_CMGR_CBM* cbm );
EXTERN void rAT_PlusCMGD  ( );
EXTERN void rAT_PlusCMGL  ( T_ACI_CMGL_SM  *smLst);
EXTERN void rAT_PlusCUSD  ( T_ACI_CUSD_MOD   m,
                            T_ACI_USSD_DATA *ussd,
                            SHORT            dcs);
EXTERN void rAT_PlusCCCM  ( LONG           *ccm);
EXTERN void rAT_PlusCSSI  ( T_ACI_CSSI_CODE code,
                            SHORT           index);
EXTERN void rAT_PlusCSSU  ( T_ACI_CSSU_CODE code,
                            SHORT           index,
                            CHAR           *number,
                            T_ACI_TOA      *type,
                            CHAR           *subaddr,
                            T_ACI_TOS      *satype);
EXTERN void rAT_PercentCCCN ( T_ACI_FAC_DIR tDirection,
                            SHORT cId,
                            T_MNCC_fac_inf *acFie  );
EXTERN void rAT_PercentCSSN ( T_ACI_FAC_DIR tDirection,
                            T_ACI_FAC_TRANS_TYPE tType,
                            T_MNCC_fac_inf           *acFie );

EXTERN void rAT_PlusCPOL  ( SHORT              startIdx,
                            SHORT              lastIdx,
                            T_ACI_CPOL_OPDESC *operLst,
                            SHORT              usdNtry );

EXTERN void rAT_PlusCLAN  ( T_ACI_LAN_SUP  *CLang);
EXTERN void rAT_PlusCLAE  ( T_ACI_LAN_SUP  *CLang);

#ifdef FF_PS_RSSI
EXTERN void rAT_PercentCSQ  (UBYTE rssi, UBYTE ber, UBYTE actlevel, UBYTE min_access_level);
#else
EXTERN void rAT_PercentCSQ  (UBYTE rssi, UBYTE ber, UBYTE actlevel);
#endif

EXTERN void rAT_PlusCIMI  ( CHAR           *imsi);
EXTERN void rAT_PlusCNUM  ( T_ACI_CNUM_MSISDN *msisdn,
                            UBYTE              num );
EXTERN void rAT_PlusCRSM  ( SHORT           sw1,
                            SHORT           sw2,
                            SHORT           rspLen,
                            UBYTE          *rsp    );
EXTERN void rAT_PlusCSIM  ( SHORT           rspLen,
                            UBYTE          *rsp    );
EXTERN void rAT_PlusCCWV  ( T_ACI_CCWV_CHRG charging );
EXTERN void rAT_PercentSQ ( BYTE            segm);
EXTERN void rAT_PercentBC ( BYTE            segm);
EXTERN void rAT_PercentDRV( T_ACI_DRV_DEV   device,
                            T_ACI_DRV_FCT   function,
                            UBYTE           val1,
                            UBYTE           val2);
EXTERN void rAT_PercentSIMREM( T_ACI_SIMREM_TYPE srType );
EXTERN void rAT_PercentSIMINS( T_ACI_CME_ERR     err    );
EXTERN void rAT_PercentCOLR  ( T_ACI_COLR_STAT stat);
EXTERN void rAT_PercentCLOG  ( T_ACI_CLOG     *cmd );
EXTERN void rAT_PercentKSIR  ( T_ACI_KSIR     *ksStat);
EXTERN void rAT_PercentCPI   ( SHORT           cId,
                               T_ACI_CPI_MSG   msgType,
                               T_ACI_CPI_IBT   ibt,
                               T_ACI_CPI_TCH   tch,
                               USHORT          cause );
EXTERN void rAT_PercentCSTAT (T_ACI_STATE_MSG msgType);

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
EXTERN void rAT_PercentCPRSM (T_ACI_CPRSM_MOD mode);
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */

EXTERN void rAT_PercentALS  ( T_ACI_ALS_MOD ALSmode );

EXTERN void rAT_PercentCTYI  ( T_ACI_CTTY_NEG  neg,
                               T_ACI_CTTY_TRX  trx );
EXTERN void rAT_PercentCTV   ( void );

#ifdef SIM_TOOLKIT
EXTERN void rAT_PercentSATI  ( SHORT           len,
                               UBYTE          *satCmd);
EXTERN void rAT_PercentSATE  ( SHORT           len,
                               UBYTE          *satCmd);
EXTERN void rAT_PercentSATN  ( SHORT           len,
                               UBYTE          *satCmd,
                               T_ACI_SATN_CNTRL_TYPE  cntrl_type);
#ifdef FF_SAT_E
EXTERN void rAT_PercentSATA  ( SHORT           cId,
                               LONG            rdlTimeout_ms,
                               T_ACI_SATA_ADD *addParm);
#else 
EXTERN void rAT_PercentSATA  ( SHORT           cId,
                               LONG            rdlTimeout_ms);
#endif /* FF_SAT_E */

EXTERN void rAT_SatFUN ( int ref, T_SIM_FILE_UPDATE_IND* mmi_insert_ind );
EXTERN void rAT_SatFRN ( int ref, T_SIM_FILE_UPDATE_IND* mmi_insert_ind );

#endif  /* SIM_TOOLKIT */
EXTERN void rAT_PercentCCBS  ( T_ACI_CCBS_IND  indct,
                               T_ACI_CCBS_STAT status,
                               T_ACI_CCBS_SET *setting );
EXTERN void rAT_PercentCNAP  ( T_callingName *NameId, T_ACI_CNAP_STATUS status );
#ifdef FF_TIMEZONE
EXTERN void rAT_PlusCTZV     ( S32 time_zone );
#else
EXTERN void rAT_PlusCTZV     ( UBYTE* time_zone );
#endif /*FF_TIMEZONE*/
/*#ifdef FF_MMI_RIV*/
EXTERN void rAT_PercentCTZV  ( T_MMR_INFO_IND *mmr_info_ind, S32 timezone);
EXTERN void rAT_PercentCNIV  ( T_MMR_INFO_IND *mmr_info_ind);
/*#endif*/
EXTERN void rAT_SignalSMS    ( UBYTE state );

EXTERN void rAT_phb_status   ( T_ACI_PB_STAT status );
EXTERN void rAT_sms_ready    ( void );

EXTERN void rAT_PlusCIEV( T_ACI_MM_CIND_VAL_TYPE sCindValues, T_ACI_MM_CMER_VAL_TYPE sCmerSettings );

#ifdef FF_EM_MODE
EXTERN void rAT_PercentEM    ( T_EM_VAL *val_tmp );
EXTERN void rAT_PercentEMET  ( T_EM_VAL val_tmp );
EXTERN void rAT_PercentEMETS ( UBYTE entity );
#endif /* FF_EM_MODE */

EXTERN void rAT_PercentRDL(T_ACI_CC_REDIAL_STATE state);

#ifdef TI_PS_FF_AT_P_CMD_RDLB
EXTERN void rAT_PercentRDLB(T_ACI_CC_RDL_BLACKL_STATE state);
#endif /* TI_PS_FF_AT_P_CMD_RDLB */

EXTERN void rAT_Z      ( void );

EXTERN void rAT_PercentCPRI  ( UBYTE gsm_ciph,
                               UBYTE gprs_ciph );

EXTERN void rAT_PercentSIMEF( T_SIM_FILE_UPDATE_IND *sim_file_update_ind);

EXTERN void rAT_PercentCMGR  ( T_ACI_CMGL_SM*  sm,
                               T_ACI_CMGR_CBM* cbm );

EXTERN void rAT_PercentCMGL  ( T_ACI_CMGL_SM  *smLst);

#ifdef FF_CPHS_REL4
EXTERN void rAT_PercentCFIS  ( T_ACI_CFIS_CFU  *cfis);

EXTERN void rAT_PercentMWIS  ( T_ACI_MWIS_MWI  *mwis);
EXTERN void rAT_PercentMWI   ( UBYTE mspId,T_ACI_MWIS_MWI  *mwis);

EXTERN void rAT_PercentMBI  ( T_ACI_MBI *mbi );
EXTERN void rAT_PercentMBDN ( T_ACI_MBDN *mbdn );
#endif /* FF_CPHS_REL4 */

EXTERN void rAT_PercentPBCI  ( T_PHB_CHANGED_INFO *chgInfo );

#endif /* CMH_F_C */

EXTERN void rAT_PercentRLOG  ( T_ACI_RLOG *rslt );
#ifdef FF_WAP
EXTERN void rAT_WAP_PPP_connected(SHORT cId,ULONG IPAddress);

EXTERN void rAT_WAP_start_login(void);
EXTERN void rAT_WAP_start_gprs_login(void);

EXTERN void rAT_WAP_call_disconnected(SHORT cId);
#endif /* WAP */

#endif /*#if defined SMI OR defined MFW OR FF_MMI_RIV*/

#if defined MFW AND defined TI_PS_FF_AT_P_CMD_MMITEST
/* MMI TEST */
EXTERN void rAT_PercentMMITEST(char *param);
#endif

/*--------------- call-backs for AT CI ----------------------------*/

#ifdef CMH_F_C /*lint -save -e18 */ /* Turn off Lint errors for this "construct" */

EXTERN void rCI_OK            ( void );
EXTERN void rCI_NO_CARRIER    ( void );
EXTERN void rCI_CONNECT       ( void );
EXTERN void rCI_BUSY          ( void );
EXTERN void rCI_NO_ANSWER     ( void );
EXTERN void rCI_PlusCME       ( void );
EXTERN void rCI_PlusCOPS      ( void );
EXTERN void rCI_PercentCOPS   ( void );
EXTERN void rCI_PlusCPIN      ( void );
EXTERN void rCI_PlusCREG      ( void );
EXTERN void rCI_PercentCREG   ( void );
#ifdef REL99
EXTERN void rCI_PercentCMGRS  ( void );
#endif /* REL99 */
EXTERN void rCI_PlusCRING     ( void );
EXTERN void rCI_PlusCRING_OFF ( void );
EXTERN void rCI_PlusCLIP      ( void );
EXTERN void rCI_PlusCDIP      ( void );
EXTERN void rCI_PlusCLIR      ( void );
EXTERN void rCI_PercentCSQ    ( void );
EXTERN void rCI_PlusCOLP      ( void );
EXTERN void rCI_PlusDR        ( void );
EXTERN void rCI_PlusCR        ( void );
EXTERN void rCI_PlusCLCK      ( void );
EXTERN void rCI_PlusCCFC      ( void );
EXTERN void rCI_PlusCCWA      ( void );
EXTERN void rCI_PlusCSMS      ( void );
EXTERN void rCI_PlusCPMS      ( void );
EXTERN void rCI_PlusCMS       ( void );
EXTERN void rCI_PlusCBMI      ( void );
EXTERN void rCI_PlusCMT       ( void );
EXTERN void rCI_PlusCMTI      ( void );
EXTERN void rCI_PlusCMGS      ( void );
EXTERN void rCI_PlusCMSS      ( void );
EXTERN void rCI_PlusCMGW      ( void );
EXTERN void rCI_PlusCMGC      ( void );
EXTERN void rCI_PlusCMGD      ( void );
EXTERN void rCI_PlusCDS       ( void );
EXTERN void rCI_PlusCBM       ( void );
EXTERN void rCI_PlusILRR      ( void );
EXTERN void rCI_PlusCMGR      ( void );
EXTERN void rCI_PlusCMGL      ( void );
EXTERN void rCI_PlusCUSD      ( void );
EXTERN void rCI_PlusCCCM      ( void );
EXTERN void rCI_PlusCSSI      ( void );
EXTERN void rCI_PlusCSSU      ( void );
EXTERN void rCI_PlusCPOL      ( void );

EXTERN void rCI_PlusCLAN      ( void );
EXTERN void rCI_PlusCLAE      ( void );

EXTERN void rCI_PlusCIMI      ( void );
EXTERN void rCI_PlusCNUM      ( void );
EXTERN void rCI_PlusCRSM      ( void );
EXTERN void rCI_PlusCSIM      ( void );
EXTERN void rCI_PlusCCWV      ( void );
EXTERN void rCI_PercentSQ     ( void );
EXTERN void rCI_PercentBC     ( void );
EXTERN void rCI_PercentDRV    ( void );
EXTERN void rCI_PercentSIMREM ( void );
EXTERN void rCI_PercentSIMINS ( void );
EXTERN void rCI_PercentCOLR   ( void );
EXTERN void rCI_PercentKSIR   ( void );
EXTERN void rCI_PercentCPI    ( void );
EXTERN void rCI_PercentCSTAT  ( void );

EXTERN void rCI_PercentCTYI   ( void );
EXTERN void rCI_PercentCTV    ( void );
EXTERN void rCI_PercentCCCN   ( void );
EXTERN void rCI_PercentCSSN   ( void );
#ifdef SIM_TOOLKIT
EXTERN void rCI_PercentSATI   ( void );
EXTERN void rCI_PercentSATE   ( void );
EXTERN void rCI_PercentSATN   ( void );
EXTERN void rCI_PercentSATA   ( void );
#endif  /* SIM_TOOLKIT */
EXTERN void rCI_PercentCCBS   ( void );
EXTERN void rCI_PercentCNAP   ( void );
EXTERN void rCI_PlusCTZV      ( void );
EXTERN void rCI_PercentCTZV   ( void );
EXTERN void rCI_PercentCNIV   ( void );

EXTERN void rCI_SignalSMS     ( void );
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
EXTERN void rCI_PercentCPRSM  ( void );
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */

EXTERN void rCI_phb_status    ( void );
EXTERN void rCI_sms_ready     ( void );
#ifdef FF_EM_MODE
EXTERN void rCI_PercentEM     ( void );
EXTERN void rCI_PercentEMET   ( void );
EXTERN void rCI_PercentEMETS  ( void );
#endif /* FF_EM_MODE */
#if defined FF_EOTD
EXTERN void rCI_PlusCLPS      ( void );
#endif
EXTERN void rCI_PercentALS    ( void );
EXTERN void rCI_PercentRDL ( void );
#ifdef TI_PS_FF_AT_P_CMD_RDLB
EXTERN void rCI_PercentRDLB ( void );
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
EXTERN void rCI_PlusCIEV      ( void );
EXTERN void rCI_PercentCSTAT  ( void );
EXTERN void rCI_Z             ( void );
EXTERN void rCI_PercentCPRI   ( void );
EXTERN void rCI_PercentSIMEF  ( void );

EXTERN void rCI_PercentCMGR   ( void );
EXTERN void rCI_PercentCMGL   ( void );
#ifdef FF_CPHS_REL4
EXTERN void rCI_PercentCFIS   ( void );

EXTERN void rCI_PercentMWIS   ( void );
EXTERN void rCI_PercentMWI    ( void );

EXTERN void rCI_PercentMBI  ( void );
EXTERN void rCI_PercentMBDN ( void );
#endif /* FF_CPHS_REL4 */

EXTERN void rCI_PercentPBCI   ( void );

/*lint -restore */
#else

EXTERN void rCI_OK        ( T_ACI_AT_CMD    cmdId );
EXTERN void rCI_NO_CARRIER( T_ACI_AT_CMD    cmdId,
                            SHORT cId );
EXTERN void rCI_CONNECT   ( T_ACI_AT_CMD    cmdId,
                            T_ACI_BS_SPEED  speed,
                            SHORT cId,
                            BOOL flow_cntr);
EXTERN void rCI_BUSY      ( T_ACI_AT_CMD    cmdId,
                            SHORT cId );
EXTERN void rCI_NO_ANSWER ( T_ACI_AT_CMD    cmdId,
                            SHORT cId );
EXTERN void rCI_PlusCME   ( T_ACI_AT_CMD    cmdId,
                            T_ACI_CME_ERR   err );
EXTERN void rCI_PlusCOPS  ( SHORT           lastIdx,
                            T_ACI_COPS_OPDESC * operLst);
EXTERN void rCI_PercentCOPS  ( SHORT           lastIdx,
                            T_ACI_COPS_OPDESC * operLst);
EXTERN void rCI_PlusCPIN  ( T_ACI_CPIN_RSLT rslt );
EXTERN void rCI_PlusCREG  ( T_ACI_CREG_STAT status,
                            USHORT          lac,
                            USHORT          cid );
EXTERN void rCI_PercentCREG  ( T_ACI_CREG_STAT       status,
                               USHORT                lac,
                               USHORT                cid,
                               T_ACI_P_CREG_GPRS_IND gprs_ind,
                               U8              rt);
#ifdef REL99
EXTERN void rCI_PercentCMGRS ( UBYTE mode,
                               T_MNSMS_RETRANS_CNF * mnsms_retrans_cnf,
                               T_MNSMS_SEND_PROG_IND * mnsms_send_prog_ind );
#endif
EXTERN void rCI_PlusCRING ( T_ACI_CRING_MOD mode,
                            T_ACI_CRING_SERV_TYP type1,
                            T_ACI_CRING_SERV_TYP type2 );
EXTERN void rCI_PlusCRING_OFF ( SHORT cId );

#ifdef FF_PS_RSSI
EXTERN void rCI_PercentCSQ(UBYTE rssi, UBYTE ber, UBYTE actlevel, UBYTE min_access_level);
#else
EXTERN void rCI_PercentCSQ(UBYTE rssi, UBYTE ber, UBYTE actlevel);
#endif

#ifdef NO_ASCIIZ
EXTERN void rCI_PlusCLIP  ( T_ACI_CLIP_STAT stat,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            U8              validity,
                            CHAR          * subaddr,
                            T_ACI_TOS     * satype,
                            T_ACI_PB_TEXT * alpha);
#else  /* ifdef NO_ASCIIZ */
EXTERN void rCI_PlusCLIP  ( T_ACI_CLIP_STAT stat,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            U8              validity,
                            CHAR          * subaddr,
                            T_ACI_TOS     * satype,
                            CHAR          * alpha);
#endif /* ifdef NO_ASCIIZ */

EXTERN void rCI_PlusCDIP  ( CHAR          * number,
                                                  T_ACI_TOA     * type,
                                                  CHAR          * subaddr,
                                                  T_ACI_TOS     * satype);


EXTERN void rCI_PlusCLIR  ( T_ACI_CLIR_MOD  mode,
                            T_ACI_CLIR_STAT stat);
#ifdef NO_ASCIIZ
EXTERN void rCI_PlusCOLP  ( T_ACI_COLP_STAT stat,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            CHAR          * subaddr,
                            T_ACI_TOS     * satype,
                            T_ACI_PB_TEXT * alpha);
#else  /* ifdef NO_ASCIIZ */
EXTERN void rCI_PlusCOLP  ( T_ACI_COLP_STAT stat,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            CHAR          * subaddr,
                            T_ACI_TOS     * satype,
                            CHAR          * alpha);
#endif /* ifdef NO_ASCIIZ */
EXTERN void rCI_PlusDR    ( T_ACI_DR_TYP    type );
EXTERN void rCI_PlusCR    ( T_ACI_CRING_SERV_TYP   service);
EXTERN void rCI_PlusCLCK  ( T_ACI_CLSSTAT * clsStat);
EXTERN void rCI_PlusCCFC  ( T_ACI_CCFC_SET* setting);
#ifdef NO_ASCIIZ
EXTERN void rCI_PlusCCWA  ( T_ACI_CLSSTAT * clsStatLst,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            U8              validity,
                            T_ACI_CLASS     class_type,
                            T_ACI_PB_TEXT * alpha);
#else  /* ifdef NO_ASCIIZ */
EXTERN void rCI_PlusCCWA  ( T_ACI_CLSSTAT * clsStatLst,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            U8              validity,
                            T_ACI_CLASS     class_type,
                            CHAR          * alpha);
#endif /* ifdef NO_ASCIIZ */
EXTERN void rCI_PlusCSMS  ( T_ACI_CSMS_SERV service,
                            T_ACI_CSMS_SUPP mt,
                            T_ACI_CSMS_SUPP mo,
                            T_ACI_CSMS_SUPP bm);
EXTERN void rCI_PlusCPMS  ( T_ACI_SMS_STOR_OCC * mem1,
                            T_ACI_SMS_STOR_OCC * mem2,
                            T_ACI_SMS_STOR_OCC * mem3);
EXTERN void rCI_PlusCMS   ( T_ACI_AT_CMD    cmdId,
                            T_ACI_CMS_ERR   err,
                            T_EXT_CMS_ERROR * conc_error);
EXTERN void rCI_PlusCBMI  ( T_ACI_SMS_STOR  mem,
                            UBYTE           index);
EXTERN void rCI_PlusCMT   ( T_MNSMS_MESSAGE_IND * mnsms_message_ind);
EXTERN void rCI_PlusCMTI  ( T_ACI_SMS_STOR  mem,
                            UBYTE           index);
EXTERN void rCI_PlusCMGS  ( T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf);
EXTERN void rCI_PlusCMSS  ( T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf);
EXTERN void rCI_PlusCMGW  ( UBYTE           index);
EXTERN void rCI_PlusCMGC  ( T_MNSMS_COMMAND_CNF * mnsms_command_cnf);
EXTERN void rCI_PlusCMGD  ( );
#ifndef CST_EXTS_C
EXTERN void rCI_PlusCBM   ( T_MMI_CBCH_IND * mmi_cbch_ind);
#endif
EXTERN void rCI_PlusCDS   ( T_MNSMS_STATUS_IND * mnsms_status_ind);
EXTERN void rCI_PlusILRR  ( T_ACI_BS_SPEED  speed,
                            T_ACI_BS_FRM    format,
                            T_ACI_BS_PAR    parity);
EXTERN void rCI_PlusCMGR  ( T_MNSMS_READ_CNF* mnsms_read_cnf,
                            T_ACI_CMGR_CBM * cbm);
EXTERN void rCI_PlusCMGL  ( T_MNSMS_READ_CNF *mnsms_read_cnf);
EXTERN void rCI_PlusCUSD  ( T_ACI_CUSD_MOD   m,
                            T_ACI_USSD_DATA  *ussd,
                            SHORT            dcs);
EXTERN void rCI_PlusCCCM  ( LONG           *ccm);
EXTERN void rCI_PlusCSSI  ( T_ACI_CSSI_CODE code,
                            SHORT           index);
EXTERN void rCI_PlusCSSU  ( T_ACI_CSSU_CODE code,
                            SHORT           index,
                            CHAR           *number,
                            T_ACI_TOA      *type,
                            CHAR           *subaddr,
                            T_ACI_TOS      *satype);
EXTERN void rCI_PlusCPOL  ( SHORT              startIdx,
                            SHORT              lastIdx,
                            T_ACI_CPOL_OPDESC *operLst,
                            SHORT              usdNtry );

EXTERN void rCI_PlusCLAN  ( T_ACI_LAN_SUP  *CLang);
EXTERN void rCI_PlusCLAE  ( T_ACI_LAN_SUP  *CLang);

EXTERN void rCI_PlusCIMI  ( CHAR           *imsi);
EXTERN void rCI_PlusCNUM  ( T_ACI_CNUM_MSISDN *msisdn,
                            UBYTE              num );
EXTERN void rCI_PlusCRSM  ( SHORT           sw1,
                            SHORT           sw2,
                            SHORT           rspLen,
                            UBYTE          *rsp    );
EXTERN void rCI_PlusCSIM  ( SHORT           rspLen,
                            UBYTE          *rsp    );
EXTERN void rCI_PlusCCWV  ( T_ACI_CCWV_CHRG charging );
EXTERN void rCI_PercentSQ ( BYTE            segm);
EXTERN void rCI_PercentBC ( BYTE            segm);
EXTERN void rCI_PercentDRV( T_ACI_DRV_DEV   device,
                            T_ACI_DRV_FCT   function,
                            UBYTE           val1,
                            UBYTE           val2);
EXTERN void rCI_PercentSIMREM( T_ACI_SIMREM_TYPE srType );
EXTERN void rCI_PercentSIMINS( T_ACI_CME_ERR     err    );
EXTERN void rCI_PercentCOLR  ( T_ACI_COLR_STAT stat);
EXTERN void rCI_PercentKSIR  ( T_ACI_KSIR     *ksStat);
EXTERN void rCI_PercentCPI   ( SHORT           cId,
                               T_ACI_CPI_MSG   msgType,
                               T_ACI_CPI_IBT   ibt,
                               T_ACI_CPI_TCH   tch,
                               USHORT          cause);
EXTERN void rCI_PercentCSTAT (T_ACI_STATE_MSG msgType);

EXTERN void rCI_PercentCTYI  ( T_ACI_CTTY_NEG  neg,
                               T_ACI_CTTY_TRX  trx );

EXTERN void rCI_PercentCSSN  (T_ACI_FAC_DIR        tDirection,
                              T_ACI_FAC_TRANS_TYPE tType,
                              T_MNCC_fac_inf       *fie);
#ifdef SIM_TOOLKIT
EXTERN void rCI_PercentSATN  ( SHORT           len,
                               UBYTE          *satCmd,
                               T_ACI_SATN_CNTRL_TYPE  cntrl_type);
#ifdef FF_SAT_E
EXTERN void rCI_PercentSATA  ( SHORT           cId,
                               LONG            rdlTimeout_ms,
                               T_ACI_SATA_ADD *addParm);
#else 
EXTERN void rCI_PercentSATA  ( SHORT           cId,
                               LONG            rdlTimeout_ms);
#endif /* FF_SAT_E */ 
EXTERN void rCI_PercentSATI  ( SHORT           len,
                               UBYTE          *satCmd);
EXTERN void rCI_PercentSATE  ( SHORT           len,
                               UBYTE          *satCmd);
#endif  /* SIM_TOOLKIT */
EXTERN void rCI_PercentCCBS  ( T_ACI_CCBS_IND  indct,
                               T_ACI_CCBS_STAT status,
                               T_ACI_CCBS_SET  *setting,
                               BOOL            internediate_result);
EXTERN void rCI_PercentCNAP  ( T_callingName *NameId, T_ACI_CNAP_STATUS status );
#ifdef FF_TIMEZONE
EXTERN void rCI_PlusCTZV     ( S32 timezone );
#else
EXTERN void rCI_PlusCTZV     ( UBYTE* timezone );
#endif
EXTERN void rCI_PercentCTZV  ( T_MMR_INFO_IND *mmr_info_ind, S32 timezone );
EXTERN void rCI_PercentCNIV  ( T_MMR_INFO_IND *mmr_info_ind);

EXTERN void rCI_PlusCMS_Conc ( T_ACI_AT_CMD     cmdId,
                               T_ACI_CMS_ERR    err,
                               T_EXT_CMS_ERROR *conc_error );
EXTERN void rCI_SignalSMS    ( UBYTE state );

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
EXTERN void rCI_PercentCPRSM ( T_ACI_CPRSM_MOD mode );
#endif /* #ifdef TI_PS_FF_AT_P_CMD_CPRSM */

EXTERN void rCI_PercentCTV   ( void );
/*EXTERN void rAT_phb_status   ( T_ACI_PB_STAT status );*/
EXTERN void rCI_phb_status   ( T_ACI_PB_STAT status );
EXTERN void rCI_sms_ready    ( void );
#ifdef FF_EM_MODE
EXTERN void rCI_PercentEM    ( T_EM_VAL * val_tmp );
EXTERN void rCI_PercentEMET  ( T_DRV_SIGNAL_EM_EVENT * Signal );
EXTERN void rCI_PercentEMETS ( UBYTE entity );
#endif /* FF_EM_MODE */


EXTERN void rCI_PercentCPRI  ( UBYTE gsm_ciph,
                               UBYTE gprs_ciph );

EXTERN void rCI_PercentSIMEF ( T_SIM_FILE_UPDATE_IND *sim_file_update_ind);

#if defined FF_EOTD
EXTERN void rCI_PlusCLPS   ( UBYTE srcId, T_LOC_POS_DATA * lc_data );
#endif
EXTERN void rCI_PercentALS ( T_ACI_ALS_MOD ALSmode );
EXTERN void rCI_PlusCIEV ( T_ACI_MM_CIND_VAL_TYPE sCindValues,
                          T_ACI_MM_CMER_VAL_TYPE sCmerSettings );
EXTERN void rCI_PercentRDL ( T_ACI_CC_REDIAL_STATE state );

#ifdef TI_PS_FF_AT_P_CMD_RDLB
EXTERN void rCI_PercentRDLB( T_ACI_CC_RDL_BLACKL_STATE state );
#endif /* TI_PS_FF_AT_P_CMD_RDLB */

EXTERN void rCI_PercenCCCN ( char *sFieString, T_ACI_FAC_DIR uiDir, short cId );
EXTERN void rCI_PercenCSSN ( char *sFieString, T_ACI_FAC_DIR uiDir, T_ACI_FAC_TRANS_TYPE uiFacTransType );
EXTERN void rCI_Z          ( void );

EXTERN void rCI_PercentCMGR  ( T_MNSMS_READ_CNF* mnsms_read_cnf,
                               T_ACI_CMGR_CBM * cbm);

EXTERN void rCI_PercentCMGL  ( T_MNSMS_READ_CNF *mnsms_read_cnf);
#ifdef FF_CPHS_REL4
EXTERN void rCI_PercentCFIS  ( T_ACI_CFIS_CFU *cfis);

EXTERN void rCI_PercentMWIS  ( T_ACI_MWIS_MWI *mwis);
EXTERN void rCI_PercentMWI   ( UBYTE mspId,T_ACI_MWIS_MWI *mwis);

EXTERN void rCI_PercentMBI  ( T_ACI_MBI *mbi );
EXTERN void rCI_PercentMBDN ( T_ACI_MBDN *mbdn );
#endif /* FF_CPHS_REL4 */

EXTERN void rCI_PercentPBCI   ( T_PHB_CHANGED_INFO *chgInfo );
#endif

/*--------------- extension mechansim ----------------------------*/

/*--------------- constants --------------------------------------*/

#define ACI_EXT_STRG_END 0xFF

/*--------------- configuration ----------------------------------*/

EXTERN T_ACI_RETURN sAT_CfgEXT ( CHAR**  fwrdLst );

/*--------------- Extension Handler -> AT Interpreter ------------*/

EXTERN T_ACI_RETURN sAT_FRI    ( USHORT  cmdLen  );
EXTERN T_ACI_RETURN sAT_URC    ( CHAR*   out     );

/*--------------- AT Interpreter -> Extension Handler ------------*/

EXTERN T_ACI_RETURN rAT_EXT    ( CHAR*   cmd,
                                 USHORT* cmdLen,
                                 CHAR*   out,
                                 USHORT  outLen  );
EXTERN T_ACI_RETURN rAT_ACP    ( CHAR*   out,
                                 USHORT  outLen  );

#endif /* ACI_CMH_H */

/*==== EOF ========================================================*/

/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI
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
|  Purpose :  Definitions for the AT Command Interpreter
+----------------------------------------------------------------------------- 
*/ 

#ifndef ACI_CMD_H
#define ACI_CMD_H

#ifdef FF_ATI
#include "ati_cmd.h"
#endif /* FF_ATI */

#define KEY       15   /* Length of "+CBLA: " for instance */
#define LONG_LTH  10   /* Max length of a number of type long in a string */
#define SHORT_LTH  5
#define BYTE_LTH   3
#define HEX_LTH    8

typedef struct
{
  char *name;
  T_ACI_SMS_STOR stor;
} SMS_Memory;

typedef struct
{
  char *name;
  T_ACI_DBG_INFO stor;
} DBG_Memory;

#if (defined (ACI_CMD_C)) OR (defined (CMH_SIMF_C) AND !defined (FF_ATI))
GLOBAL const T_ACI_LAN_SUP lngs [] =
{
  {"au",  CLAN_LNG_AUT},
  {"en",  CLAN_LNG_ENG},
  {"fr",  CLAN_LNG_FRE},
  {"de",  CLAN_LNG_GER},
  {"da",  CLAN_LNG_DUT},
  {"it",  CLAN_LNG_ITA},
  {"es",  CLAN_LNG_SPA},
  {"sv",  CLAN_LNG_SWE},
  {"pt",  CLAN_LNG_POR},
  {"fi",  CLAN_LNG_FIN},
  {"no",  CLAN_LNG_NOR},
  {"el",  CLAN_LNG_GRE},
  {"tr",  CLAN_LNG_TUR},
  {"hu",  CLAN_LNG_HUN},
  {"sl",  CLAN_LNG_SLO},
  {"pl",  CLAN_LNG_POL},
  {"ru",  CLAN_LNG_RUS},
  {"in",  CLAN_LNG_IND},
  {"cs",  CLAN_LNG_CZE},
  {"zh",  CLAN_LNG_CHI},
  {"ca",  CLAN_LNG_CAN},
  {"mn",  CLAN_LNG_MAN},
  {"tw",  CLAN_LNG_TAI},
  {"ar",  CLAN_LNG_ARA},
  {NULL,  CLAN_LNG_ENG}
};
#else 
EXTERN const T_ACI_LAN_SUP lngs [];
#endif 



/*--- +CMEE -------------------------------------------------------*/

typedef enum
{
  CMEE_MOD_NotPresent   = -1,
  CMEE_MOD_Disable,
  CMEE_MOD_Numeric,
  CMEE_MOD_Verbose
}
T_ACI_CMEE_MOD;

/*----------- +CSCS -----------------------------------------------*/

typedef enum
{
  CSCS_CHSET_NotPresent = -1,
  CSCS_CHSET_Ira        =  0,
  CSCS_CHSET_Pcdn       =  1,
  CSCS_CHSET_8859_1     =  2,
  CSCS_CHSET_Pccp_437   =  3,
  CSCS_CHSET_Gsm        =  4,
  CSCS_CHSET_Hex        =  5,
  CSCS_CHSET_Ucs2       =  6,
  CSCS_CHSET_Tables     =  5
}
T_ACI_CSCS_CHSET;

/*----------- alphabets -------------------------------------------*/

typedef enum          /* see DCS in Rec. GSM 03.38 */
{
  CSCS_ALPHA_7_Bit    = 0,
  CSCS_ALPHA_8_Bit    = 1,
  CSCS_ALPHA_16_Bit   = 2,
  CSCS_ALPHA_Reserved = 3
}
T_ACI_CSCS_ALPHA;

typedef enum
{
  CSCS_DIR_GsmToIra   = 0,
  CSCS_DIR_IraToGsm   = 1
}
T_ACI_CSCS_DIR;

typedef enum
{
  GSM_ALPHA_Def,
  GSM_ALPHA_Int
}
T_ACI_GSM_ALPHA;

/*----------- +CNMI -----------------------------------------------*/

typedef enum
{
  CNMI_MOD_NotPresent       = -1,
  CNMI_MOD_Buffer,
  CNMI_MOD_DiscardOrForward,
  CNMI_MOD_BufferAndFlush
}
T_ACI_CNMI_MOD;

typedef enum
{
  CNMI_BFR_NotPresent       = -1,
  CNMI_BFR_Flush,
  CNMI_BFR_Clear
}
T_ACI_CNMI_BFR;

/*----------- other -----------------------------------------------*/

typedef enum
{
    atOk                    = 0,
    atConnect,
    atRing,
    atNoCarrier,
    atError,
    atNoDialtone,
    atBusy,
    atNoAnswer,
    atConnect1
} AtErrCode;

typedef enum
{
    copsUnknown = 0,
    copsAvail = 1,
    copsCurrent = 2,
    copsForbidden = 3
} CopsState;

typedef enum
{
    ppSimVerify = 1
} PinPending;

typedef struct
{
  struct
  {
    UBYTE  atV;
    UBYTE  atL;
    UBYTE  atM;
  }s1415;
  struct                  /* presentation flags */
  {
    UBYTE COLP_stat;
    UBYTE CCWA_stat;
    UBYTE CCBS_stat;
  }flags;
  UBYTE S[10+1];          /* Caution: for S[0..n] we need S[n+1] to be declared here!!! */

  /* the following registers above 10 are not index based and only defined if needed */
  UBYTE S30;
#ifdef GPRS
  UBYTE S99;              /* Rings until Automatic Context Rejection */
#endif /* GPRS */

  UBYTE CNMI_mode;
  UBYTE CNMI_bfr;
  struct                  /* ringing parameters */
  {
    UBYTE srcID_S0;
#ifdef GPRS
    char srcID_CGAUTO;
#endif
    UBYTE rngCnt;
    UBYTE isRng;
    T_ACI_CRING_MOD mode;
    T_ACI_CRING_SERV_TYP type1;
    T_ACI_CRING_SERV_TYP type2;
  } rngPrms;
  struct                  /* CLIP parameters */
  {
    T_ACI_CLIP_STAT stat;
    CHAR            number[MAXIMUM(MNCC_MAX_CC_CALLING_NUMBER, MNCC_MAX_CC_REDIR_NUMBER)+1];
    T_ACI_TOA       type;
    U8              validity;
    CHAR            subaddr[MNCC_SUB_LENGTH+1];
    T_ACI_TOS       satype;
#ifdef NO_ASCIIZ
    T_ACI_PB_TEXT   alpha;
#else
    CHAR            alpha[MAX_PHB_NUM_LEN+1];
#endif
  }clipPrms;
} AciCmdVars;


/* sub structure concerning AT+CREG command in ATI */
typedef enum
{
  CREG_MOD_NotPresent        = -1,
  CREG_MOD_OFF               =  0,  /* disable network registration unsolicited result code */
  CREG_MOD_ON                =  1,  /* enable network registration unsolicited result code */
  CREG_MOD_LOC_INF_ON        =  2,  /* enable network registration and location information unsolicited result code */
  CREG_MOD_LOC_INF_ON_CTXACT =  3   /* enable network registration and location information unsolicited result code 
                                       and information about activated/deactivated PDP context(s) */
}
T_ATI_CREG_MOD   /* for +CREG / +CGREG / %CGREG */;

typedef struct
{
  T_ATI_CREG_MOD  pres_mode;
  USHORT          last_presented_lac;
  USHORT          last_presented_cid;
} T_ATI_REG_MOD_LAC_CID;

typedef struct
{
  T_ATI_REG_MOD_LAC_CID  mod_lac_cid;
  T_ACI_CREG_STAT        last_presented_state;
} T_ATI_CREG;

#define LOC_INFO_STRLTH (15)
/* ACI-SPR-17218: enable the indication of GPRS info */
#define GPRS_INFO_STRLTH (5)
#define COVERAGE_INFO_STRLTH (5)

typedef enum
{
  CMD_NotPresent = -1,
  CREG_CMD,
  PercentCREG_CMD
#ifdef GPRS
  ,
  PlusCGREG_CMD,
  PercentCGREG_CMD
#endif /* GPRS */
} T_ACI_CREG_CMD;

typedef enum
{
  CNAP_DISABLED = 0,
  CNAP_ENABLED = 1
} T_ATI_CNAP_MODE;

typedef struct
{
  UBYTE  atE;
  UBYTE  atQ;
  UBYTE  atX;

  UBYTE CR_stat;
  UBYTE CRC_stat;
  UBYTE CLIP_stat;
  UBYTE CDIP_stat;  
    
  UBYTE DR_stat;
  UBYTE ILRR_stat;
  UBYTE CSDH_stat;
  UBYTE CSSI_stat;
  BYTE  CSSU_stat;
  UBYTE CUSD_stat;
  UBYTE CPI_stat;
  UBYTE CCWE_stat;
  UBYTE CAOC_stat;
  UBYTE CMEE_stat;  
  UBYTE CTTY_stat;
  UBYTE CSTAT_stat;
  UBYTE SIMIND_stat;
  T_ACI_CSCS_CHSET cscsChset;
#ifdef GPRS
  UBYTE Percent_CGEREP_stat;
  UBYTE CGEREP_mode;
  UBYTE CGEREP_bfr;
#endif

  T_ACI_CCWV_CHRG CCWV_charging;

  T_ATI_CREG creg;
  T_ATI_CREG percent_creg;
  T_ATI_CNAP_MODE cnap_mode;
  UBYTE CPRI_stat;           /* Holds the status of %CPRI */
#ifdef FF_CPHS_REL4 
  UBYTE MWI_stat;            /* Holds the status of %MWI  */
#endif /* FF_CPHS_REL4 */

}
T_ATI_USER_OUTPUT_CFG;

/* ACI-SPR-17218: Extended by gprs_ind paramter */
EXTERN void r_plus_percent_CREG  ( UBYTE           srcId,
                                   int             status,
                                   USHORT          lac,
                                   USHORT          cid,
                                   T_ACI_CREG_CMD        cmd, 
                                   T_ACI_P_CREG_GPRS_IND gprs_ind,
                                   U8               rt,
                                   BOOL                  bActiveContext );

#define CPIN_MAX_LTH (12)
/* all following lengths definitions include the null termination */
#define MAX_PWD_LENGTH (32)    /* max length for passwords          */
#define MAX_PPU_LENGTH (20)    /* max length for ppu                */
#define MAX_CM_LENGTH  (7)     /* max length for call meter values  */

/* To allow for: 'yyyy/MM/dd,hh:mm:ss+zz' or 'yyyy/MM/dd,hh:mm:ss-zz' date time format */
/* To allow for:    'yy/MM/dd,hh:mm:ss+zz' or '    yy/MM/dd,hh:mm:ss-zz' date time format */
#define DATE_TIME_LENGTH  (22)
#ifdef FF_ATI
EXTERN UBYTE aci_init (T_ATI_SRC_TYPE src_type);
#endif /* FF_ATI */

EXTERN void aci_finit (UBYTE src_id);

EXTERN  void ati_cmd_init();
EXTERN  void ati_creg_init( BYTE srcId, T_ACI_CREG_CMD cmd );

BOOL aciCmdInit (AciCmdVars *v);

#ifdef FF_ATI
int cmdCommand (T_ATI_SRC_PARAMS *src_params);
#endif /* FF_ATI */

EXTERN void   utl_chsetToGsm      ( UBYTE*           in,
                                    USHORT           inLen,
                                    UBYTE*           out,
                                    USHORT*          outLen,
#ifdef REL99
                                    USHORT           outBufLen,
#endif /* REL99 */
                                    T_ACI_GSM_ALPHA  gsm        );
EXTERN void   utl_chsetToSim      ( UBYTE*           in,
                                    USHORT           inLen,
                                    UBYTE*           out,
                                    USHORT*          outLen,
                                    T_ACI_GSM_ALPHA  gsm        );
EXTERN void utl_ucs2ToSim         ( UBYTE*          in,
                                    USHORT          inLen,
                                    UBYTE*          out,
                                    USHORT*         outLen,
                                    T_ACI_GSM_ALPHA gsm,
                                    T_ACI_CSCS_ALPHA alphabet   );
EXTERN void utl_ConvUcs2ToGSM     ( UBYTE*           in,
                                    USHORT           inLen,
                                    UBYTE*           out,
                                    USHORT*          outLen,
                                    T_ACI_GSM_ALPHA  gsm,
                                    T_ACI_CSCS_ALPHA alphabet   );
EXTERN void utl_Ucs2InFormat1     ( UBYTE*           in,
                                    USHORT           inLen,
                                    UBYTE*           out,
                                    USHORT*          outLen     );
EXTERN void utl_Ucs2InFormat2     ( UBYTE*           in,
                                    USHORT           inLen,
                                    UBYTE*           out,
                                    USHORT*          outLen     );
EXTERN void utl_hexFromUCS2       ( UBYTE  *in, 
                                    UBYTE  *out, 
                                    USHORT  maxOutlen, 
                                    USHORT *outlen,
                                    T_ACI_GSM_ALPHA  gsm );
EXTERN USHORT   utl_chsetFromGsm    ( UBYTE*           in,
                                    USHORT           inLen,
                                    UBYTE*           out,
                                    USHORT           maxOutLen,
                                    USHORT*          outLen,
                                    T_ACI_GSM_ALPHA  gsm        );
EXTERN USHORT utl_ucs2FromGsm_ussd  ( UBYTE*           in,
                                     USHORT           inLen,
                                     UBYTE*           out,
                                     USHORT           maxOutLen,
                                     USHORT*          outLen,
                                     T_ACI_GSM_ALPHA  gsm,
                                     T_ACI_CSCS_ALPHA alphabet );
EXTERN void   utl_chsetFromSim    ( UBYTE*           in,
                                    USHORT           inLen,
                                    UBYTE*           out,
                                    USHORT           maxOutLen,
                                    USHORT*          outLen,
                                    T_ACI_GSM_ALPHA  gsm        );
EXTERN void   utl_hexToGsm        ( UBYTE*           in,
                                    USHORT           inLen,
                                    UBYTE*           out,
                                    USHORT*          outLen,
                                    T_ACI_GSM_ALPHA  gsm,
                                    T_ACI_CSCS_ALPHA alphabet   );
EXTERN void   utl_ucs2ToGsm        ( UBYTE*           in,
                                    USHORT           inLen,
                                    UBYTE*           out,
                                    USHORT*          outLen,
                                    T_ACI_GSM_ALPHA  gsm,
                                    T_ACI_CSCS_ALPHA alphabet   );
EXTERN USHORT   utl_hexFromGsm      ( UBYTE*           in,
                                    USHORT           inLen,
                                    UBYTE*           out,
                                    USHORT           maxOutLen,
                                    USHORT*          outLen,
                                    T_ACI_GSM_ALPHA  gsm,
                                    T_ACI_CSCS_ALPHA alphabet   );
EXTERN BOOL   utl_cvtGsmIra       ( UBYTE*           in,
                                    USHORT           inLen,
                                    UBYTE*           out,
                                    USHORT           outLen,
                                    T_ACI_CSCS_DIR   dir        );
EXTERN USHORT sprints             ( CHAR*   buf,
                                    CHAR*   arg,
                                    USHORT  len       );
EXTERN USHORT sprintq             ( CHAR*   buf,
                                    CHAR*   arg,
                                    USHORT  len       );
EXTERN void   utl_smDtaFromTe     ( UBYTE*  in,
                                    USHORT  inLen,
                                    UBYTE*  out,
                                    USHORT* outLen,
#ifdef REL99
                                    USHORT  outBufLen, 
#endif /* REL99 */
                                    UBYTE   fo,
                                    UBYTE   dcs       );
EXTERN void   utl_smDtaToTe       ( UBYTE*  in,
                                    USHORT  inLen,
                                    UBYTE*  out,
                                    USHORT  maxOutLen,
                                    USHORT* outLen,
                                    UBYTE   fo,
                                    UBYTE   dcs       );
EXTERN void   utl_cbmDtaToTe      ( UBYTE*  in,
                                    USHORT  inLen,
                                    UBYTE*  out,
                                    USHORT  maxOutLen,
                                    USHORT* outLen,
                                    UBYTE   fo,
                                    UBYTE   dcs       );
EXTERN void   utl_ussdDtaFromTe   ( UBYTE*  in,
                                    USHORT  inLen,
                                    UBYTE*  out,
                                    USHORT* outLen,
#ifdef REL99
                                    USHORT  outBufLen,
#endif /* REL99 */
                                    UBYTE   dcs       );
EXTERN USHORT   utl_ussdDtaToTe     ( UBYTE*  in,
                                    USHORT  inLen,
                                    UBYTE*  out,
                                    USHORT  maxOutLen,
                                    USHORT* outLen,
                                    UBYTE   dcs       );
EXTERN void utl_binToHex          ( UBYTE*  in,
                                    SHORT   inLen,
                                    CHAR*   out );
EXTERN USHORT utl_HexStrToBin (UBYTE*         in,
                               USHORT         inLen,
                               UBYTE*         out,
                               USHORT         outLen);
EXTERN void RetrieveLeftCmd       ( void );

EXTERN void trace_cmd_line        ( char *prefix,
                                    char *output,
                                    UBYTE srcId,
                                    USHORT output_len );

#ifdef FF_ATI
EXTERN void trace_cmd_state       ( UBYTE            srcId,
                                    T_ATI_CMD_STATE  old_state,
                                    T_ATI_CMD_STATE  new_state);
#endif /* FF_ATI */

EXTERN int utl_create_pco   (UBYTE*  buffer,
                             USHORT* length,
                             ULONG   content,
                             UBYTE   config_prot,
                             USHORT  auth_prot,
                             UBYTE*  user_name,
                             UBYTE*  password,
                             ULONG   dns1,
                             ULONG   dns2);
EXTERN int utl_analyze_pco  (UBYTE* buffer,
                             USHORT length,
                             ULONG* dns1,
                             ULONG* dns2,
                             ULONG* gateway);
GLOBAL int utl_strcasecmp (const char *s1, 
                           const char *s2);

#if defined (FF_ATI) || defined (FF_BAT)
#define CONVERT_STRING TRUE
GLOBAL void rci_display_USSD (UBYTE            srcId,
                              T_ACI_CUSD_MOD   mode,
                              UBYTE            *ussd_str,
                              UBYTE            ussd_len,
                              BOOL             cvtStr,
                              SHORT            dcs );

GLOBAL void utl_cb_percentKSIR (U8 srcId, T_ACI_KSIR *ksStat);
#endif /* defined (FF_ATI) || defined (FF_BAT) */

/*
 *-------------------------------------------------------------------
 * The following types, definitions and variables are needed for
 * storing of new message indications while the serial interface is
 * reserved for data transmission.
 *-------------------------------------------------------------------
 * START
 *-------------------------------------------------------------------
 */
#define NONE_CALL      0
#define SAT_CALL       1
#define QAT_CALL       2
#define TAT_CALL       3

#define CNMI_NONE      0
#define CNMI_CMT       1
#define CNMI_CMTI      2
#define CNMI_CBM       3
#define CNMI_CDS       4

#define CNMI_BUF_SIZE  4
#define CIEV_BUF_SIZE  2

#define T_CMT    T_MNSMS_MESSAGE_IND
typedef struct /* new message indication AT+CMTI: */
{
  T_ACI_SMS_STOR mem;
  UBYTE          index;
}
T_CMTI;
#define T_CBM    T_MMI_CBCH_IND
#define T_CDS    T_MNSMS_STATUS_IND

typedef union /* used for the different messages indications */
{
  T_CMT  cmt;
  T_CMTI cmti;
  T_CBM  cbm;
  T_CDS  cds;
}
T_CNMI_IND;

/* Removed source ID from this structure to make
   the bufferinf source independent */ /* Issue 25033 */
typedef struct
{
  UBYTE         type;
  T_CNMI_IND    indct;
}
T_CNMI_BUFFER_ELEMENT;

typedef struct
{
  UBYTE                 next;
  T_CNMI_BUFFER_ELEMENT sCnmiElement[CNMI_BUF_SIZE];
}
T_CNMI_BUFFER;

typedef struct
{
  char*          name;
  T_ACI_SMS_STAT stat;
} SMS_Stat;

#ifdef ACI_CMD_C
GLOBAL const SMS_Stat sms_stat [] =
{
  {"REC UNREAD", SMS_STAT_RecUnread },
  {"REC READ",   SMS_STAT_RecRead   },
  {"STO UNSENT", SMS_STAT_StoUnsent },
  {"STO SENT",   SMS_STAT_StoSent   },
  {"ALL",        SMS_STAT_All       },
  {NULL,         SMS_STAT_NotPresent}
};
#else
EXTERN const SMS_Stat sms_stat [];
#endif

#ifdef DTI
EXTERN void cmd_addCnmiNtry ( UBYTE type, T_CNMI_IND* newInd );
#endif
EXTERN void aci_encodeVpenh ( CHAR* vpenh_str, T_ACI_VP_ENH* vpenh );
EXTERN void rCI_IoMode      ( void );

#ifdef FF_ATI
EXTERN UBYTE            cpmsCallType;
#endif /* FF_ATI */
EXTERN BOOL             cnmiFlushInProgress;

EXTERN T_ACI_SMS_STAT cmglStat;

typedef struct
{
  UBYTE                  uiLastIndex;
  T_ACI_CIND_SIGNAL_TYPE asBufferValues[CIEV_BUF_SIZE];
} T_CIEV_SIGNAL_BUFFER;

typedef struct
{
  UBYTE                   uiLastIndex;
  T_ACI_CIND_SMSFULL_TYPE asBufferValues[CIEV_BUF_SIZE];
} T_CIEV_SMSFULL_BUFFER;

typedef struct
{
  char*          name;
  T_ACI_SMS_READ rdmode;
} SMS_RdMode;

#ifdef ACI_CMD_C
GLOBAL const SMS_RdMode sms_rdmode [] =
{
  {"READ NORMAL",     SMS_READ_Normal },
  {"READ PREVIEW",    SMS_READ_Preview   },
  {"STATUS CHANGE",   SMS_READ_StatusChange },
  {NULL,              SMS_READ_NotPresent}
};
#else
EXTERN const SMS_RdMode sms_rdmode [];
#endif

/*
 *-------------------------------------------------------------------
 * END
 *-------------------------------------------------------------------
 */
EXTERN void       aci_sendPString (UBYTE srcId, CHAR* buffer);
EXTERN void       cmd_flushCnmiBuf(void);
EXTERN void       cmd_flushCnmiBufOneByOne(void);
EXTERN UINT16     cmd_getNumberOfCnmiEntrys(void);
EXTERN T_CNMI_BUFFER_ELEMENT* cmd_getFirstCnmiMessage(void);
EXTERN T_CNMI_BUFFER_ELEMENT* cmd_getCnmiMessage(UINT16 uiIndex);
EXTERN BOOL       cmd_clearCnmiMessage(UINT16 uiIndex);
EXTERN BOOL       cmd_clearFirstCnmiMessage(void);
EXTERN BOOL       cmd_clearFirstMsgWithSrcIdInCnmiBuf(T_ACI_CMD_SRC srcId);
EXTERN void       cmd_flushCievBuf(UBYTE srcId);
EXTERN void       cmd_clearCievBuf(UBYTE srcId);
EXTERN BOOL       cmd_storeNextCnmiBufMsgToSim();
EXTERN void       cmd_clearCnmiBuf (void);
EXTERN UBYTE      aci_timeout     (USHORT handle);
EXTERN void       aciRingTimeout  (void);
EXTERN void       aciCnmaTimeout  (void);
EXTERN void       cmd_handleCnmaTimeout (void);
EXTERN void       ci_remTrailCom  (CHAR* outLine, USHORT len);
EXTERN UBYTE      toa_merge       (T_ACI_TOA type);
EXTERN T_ACI_TOA  toa_demerge     (SHORT type);
EXTERN T_ACI_TOA  toa_sms_demerge     (SHORT type);
EXTERN SHORT      tos_merge       (T_ACI_TOS type);
EXTERN T_ACI_TOS  tos_demerge     (SHORT type);
EXTERN CHAR *     sms_status      (T_ACI_SMS_STAT stat);
EXTERN void       cmd_PlusCOPN_CB ( UBYTE src_id );
EXTERN BOOL       cmd_readCOPNBlock  ( SHORT startIdx, UBYTE src_id);
EXTERN T_ACI_RETURN  cmd_readCPBRBlock (UBYTE srcId);
EXTERN void       cmd_PlusCPBR_CB(UBYTE srcId);

#define SMS_TEXT_MODE_NONE  0
#define SMS_TEXT_MODE_CMGS  1
#define SMS_TEXT_MODE_CMGW  2
#define SMS_TEXT_MODE_CMGC  3
#define SMS_PDU_MODE_CMGS   4
#define SMS_PDU_MODE_CMGC   5
#define SMS_PDU_MODE_CMGW   6
#define SMS_PDU_MODE_CNMA   7
#define SMS_TEXT_MODE_READ  8

#ifdef ACI_CMD_C
GLOBAL AciCmdVars at;
/* next variable is aimed at containing the user defined configuration
for the output format through AT interpreter
(it will eventually replace "GLOBAL AciCmdVars at;")*/
#ifdef FF_ATI
GLOBAL T_ATI_USER_OUTPUT_CFG ati_user_output_cfg[CMD_SRC_MAX];
#endif

GLOBAL UBYTE SMS_TEXT_MODE;  /* +CMGS sets 1 - +CMGW sets 2 in PDU mode 4,5 or 6*/

GLOBAL T_ACI_SMS_READ   smsReadMode = SMS_READ_Normal;
#ifdef WIN32
GLOBAL T_ACI_CPOL_MOD   cpolMode  = CPOL_MOD_NotPresent;
GLOBAL SHORT            cpolIdx2  = ACI_NumParmNotPresent;
#endif /* WIN32 */

#else /* ACI_CMD_C */
EXTERN AciCmdVars at;
/* next variable is aimed at containing the user defined configuration
for the output format through AT interpreter
(it will eventually replace "GLOBAL AciCmdVars at;")*/

#ifdef FF_ATI
EXTERN T_ATI_USER_OUTPUT_CFG ati_user_output_cfg[CMD_SRC_MAX];
#endif /* FF_ATI */

EXTERN UBYTE SMS_TEXT_MODE;


EXTERN T_ACI_SMS_READ   smsReadMode;
#ifdef WIN32
EXTERN T_ACI_CPOL_MOD   cpolMode;
EXTERN SHORT            cpolIdx2;
#endif /* WIN32 */
#endif /* ACI_CMD_C */

#endif



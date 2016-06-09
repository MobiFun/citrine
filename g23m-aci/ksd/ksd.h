/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  KSD
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
|  Purpose :  This module is used for decoding of EMMI key sequences
|             and supplementary service control strings.
+----------------------------------------------------------------------------- 
*/ 

#ifndef KSD_H
#define KSD_H

/*==== CONSTANTS ==================================================*/

#define KSD_TON_International 0x01
#define KSD_TON_Unknown       0x00

#define KSD_NPI_IsdnTelephony 0x01
#define KSD_NPI_Unknown       0x00

#define KSD_TOS_Nsap          0x00

#define KSD_OE_Even           0x00
#define KSD_OE_Odd            0x01

#define KSD_TIME_NONE         0xFF  /* no time parameter */
#define KSD_IDX_NONE          0xFF  /* no index parameter */

/*==== TYPES ======================================================*/

typedef enum              /* +COLP parameter <mode> */
{
  COLP_MOD_NotPresent = -1,
  COLP_MOD_Disable,
  COLP_MOD_Enable
}
T_ACA_COLP_MOD;

typedef enum              /* +CLIP parameter <mode> */
{
  CLIP_MOD_NotPresent = -1,
  CLIP_MOD_Disable,
  CLIP_MOD_Enable
}
T_ACA_CLIP_MOD;

typedef enum              /* KSD operation codes */
{
  KSD_OP_NONE = 0,        /* no operation */
  KSD_OP_IRGT,            /* interrogate SS */
  KSD_OP_REG,             /* register SS */
  KSD_OP_ERS,             /* erase SS */
  KSD_OP_ACT,             /* activate SS */
  KSD_OP_DEACT            /* deactivate SS */
}
T_ACI_KSD_OP;

typedef enum                    /* KSD supplementary service codes */
{
  KSD_SS_NONE         = 0xFF,   /* no operation */
  KSD_SS_ALL_SERV     = 0x00,   /* all ss service */
  KSD_SS_CLIP         = 0x11,   /* calling line identification presentation */
  KSD_SS_CLIR         = 0x12,   /* calling line identification restriction */
  KSD_SS_COLP         = 0x13,   /* connected line identification presentation */
  KSD_SS_COLR         = 0x14,   /* connected line identification restriction */
  KSD_SS_CNAP         = 0x19,   /* calling name presentation */
  KSD_SS_ALL_FWSS     = 0x20,   /* all forwarding SS              */
  KSD_SS_CFU          = 0x21,   /* call forwarding unconditional  */
  KSD_SS_ALL_CFWSS    = 0x28,   /* all conditional forwarding SS  */
  KSD_SS_CFB          = 0x29,   /* call forwarding on mobile subscriber busy */
  KSD_SS_CFNRY        = 0x2a,   /* call forwarding on no reply    */
  KSD_SS_CFNRC        = 0x2b,   /* call forwarding on mobile subscriber not reachable */
  KSD_SS_CW           = 0x41,   /* call waiting                   */
  KSD_SS_CCBS         = 0x43,   /* completion of call to busy subscribers */
  KSD_SS_ALL_CBSS     = 0x90,   /* all barring SS                 */
  KSD_SS_BOC          = 0x91,   /* barring of outgoing calls      */
  KSD_SS_BAOC         = 0x92,   /* barring of all outgoing calls  */
  KSD_SS_BOIC         = 0x93,   /* barring of outgoing international calls */
  KSD_SS_BOICXH       = 0x94,   /* barring of outgoing international calls except those directed to the home PLMN */
  KSD_SS_BIC          = 0x99,   /* barring of incoming calls      */
  KSD_SS_BAIC         = 0x9a,   /* barring of all incoming calls  */
  KSD_SS_BICRM        = 0x9b,   /* barring of incoming calls when roaming outside home PLMN Country */
  KSD_SS_PIN1         = 0xE0,   /* PIN 1 */
  KSD_SS_PIN2         = 0xE1    /* PIN 2 */
}
T_ACI_KSD_SS;

#define KSD_BS_TeleBearerUnknown (0xFF)

typedef enum                      /* KSD basic service codes GSM 2.30 */
{
  KSD_BS_None             =  0,   /* no basic service */
  KSD_BS_AllTeleAllBearer =  9,
  KSD_BS_AllTele          = 10,
  KSD_BS_Telephony,
  KSD_BS_AllData,
  KSD_BS_AllFax,
  KSD_BS_SMS              = 16,
  KSD_BS_VoiceGroup,
  KSD_BS_AllTeleXcptSMS   = 19,
  KSD_BS_AllBearer,
  KSD_BS_AllAsync,
  KSD_BS_AllSync,
  KSD_BS_AllDataCircSync  = 24,
  KSD_BS_AllDataCircAsync,
  KSD_BS_AllDedPackAcc,
  KSD_BS_AllDedPADAcc,
  KSD_BS_AllPLMNSpecTele  = 50,
  KSD_BS_PLMNSpecTele1,
  KSD_BS_PLMNSpecTele2,
  KSD_BS_PLMNSpecTele3,
  KSD_BS_PLMNSpecTele4,
  KSD_BS_PLMNSpecTele5,
  KSD_BS_PLMNSpecTele6,
  KSD_BS_PLMNSpecTele7,
  KSD_BS_PLMNSpecTele8,
  KSD_BS_PLMNSpecTele9,
  KSD_BS_PLMNSpecTele10,
  KSD_BS_PLMNSpecTele11,
  KSD_BS_PLMNSpecTele12,
  KSD_BS_PLMNSpecTele13,
  KSD_BS_PLMNSpecTele14,
  KSD_BS_PLMNSpecTele15,
  KSD_BS_AllPLMNSpecBearer  = 70,
  KSD_BS_PLMNSpecBearer1,
  KSD_BS_PLMNSpecBearer2,
  KSD_BS_PLMNSpecBearer3,
  KSD_BS_PLMNSpecBearer4,
  KSD_BS_PLMNSpecBearer5,
  KSD_BS_PLMNSpecBearer6,
  KSD_BS_PLMNSpecBearer7,
  KSD_BS_PLMNSpecBearer8,
  KSD_BS_PLMNSpecBearer9,
  KSD_BS_PLMNSpecBearer10,
  KSD_BS_PLMNSpecBearer11,
  KSD_BS_PLMNSpecBearer12,
  KSD_BS_PLMNSpecBearer13,
  KSD_BS_PLMNSpecBearer14,
  KSD_BS_PLMNSpecBearer15,
  KSD_BS_AuxTelephony      = 89
}
T_ACI_KSD_BS;

typedef enum                      /* KSD basic service type */
{
  KSD_BS_TP_None     =  0,        /* no basic service type */
  KSD_BS_TP_Tele     =  0x83,     /* basic service teleservice */
  KSD_BS_TP_Bearer   =  0x82      /* basic service bearer service */
}
T_ACI_KSD_BS_TP;

/* teleservice */
typedef enum
{
  KSD_TS_ALL_TS        =0x0,         /* all teleservices               */
  KSD_TS_ALL_SPCH      =0x10,        /* All speech transmission services */
  KSD_TS_TLPHNY        =0x11,        /* telephony                      */
  KSD_TS_EMRGNCY       =0x12,        /* emergency calls                */
  KSD_TS_ALL_SMS       =0x20,        /* all SMS services               */
  KSD_TS_SMS_MT        =0x21,        /* SMS MT PP                      */
  KSD_TS_SMS_MO        =0x22,        /* SMS MO PP                      */
  KSD_TS_ALL_FAX       =0x60,        /* all FAX transmission services  */
  KSD_TS_FAX3_ALT_SPCH =0x61,        /* FAX group 3 alter. speech      */
  KSD_TS_FAX3_AUTO     =0x62,        /* FAX group 3 automatic          */
  KSD_TS_FAX4          =0x63,        /* FAX group 4                    */
  KSD_TS_ALL_DATA      =0x70,        /* all FAX and SMS services       */
  KSD_TS_ALL_XCPT_SMS  =0x80,        /* all FAX and speech services    */
  KSD_TS_ALL_PSSS      =0xd0,        /* all PLMN specific TS           */
  KSD_TS_PLMN1         =0xd1,        /* PLMN specific TS 1             */
  KSD_TS_PLMN2         =0xd2,        /* PLMN specific TS 2             */
  KSD_TS_PLMN3         =0xd3,        /* PLMN specific TS 3             */
  KSD_TS_PLMN4         =0xd4,        /* PLMN specific TS 4             */
  KSD_TS_PLMN5         =0xd5,        /* PLMN specific TS 5             */
  KSD_TS_PLMN6         =0xd6,        /* PLMN specific TS 6             */
  KSD_TS_PLMN7         =0xd7,        /* PLMN specific TS 7             */
  KSD_TS_PLMN8         =0xd8,        /* PLMN specific TS 8             */
  KSD_TS_PLMN9         =0xd9,        /* PLMN specific TS 9             */
  KSD_TS_PLMNA         =0xda,        /* PLMN specific TS A             */
  KSD_TS_PLMNB         =0xdb,        /* PLMN specific TS B             */
  KSD_TS_PLMNC         =0xdc,        /* PLMN specific TS C             */
  KSD_TS_PLMND         =0xdd,        /* PLMN specific TS D             */
  KSD_TS_PLMNE         =0xde,        /* PLMN specific TS E             */
  KSD_TS_PLMNF         =0xdf        /* PLMN specific TS F             */

} T_ACI_KSD_TELE_SERVICE;

/* bearer service */
typedef enum
{
  KSD_BS_ALL_BS       =0x0,         /* all bearer services            */
  KSD_BS_ALL_DATA_CDA =0x10,        /* all data CDA services          */
  KSD_BS_CDA_300      =0x11,        /* data CDA  300 bps              */
  KSD_BS_CDA_1200     =0x12,        /* data CDA 1200 bps              */
  KSD_BS_CDA_1200_75  =0x13,        /* data CDA 1200/75 bps           */
  KSD_BS_CDA_2400     =0x14,        /* data CDA 2400 bps              */
  KSD_BS_CDA_4800     =0x15,        /* data CDA 4800 bps              */
  KSD_BS_CDA_9600     =0x16,        /* data CDA 9600 bps              */
  KSD_BS_ALL_DATA_CDS =0x18,        /* all data CDS services          */
  KSD_BS_CDS_1200     =0x1a,        /* data CDS 1200 bps              */
  KSD_BS_CDS_2400     =0x1c,        /* data CDS 2400 bps              */
  KSD_BS_CDS_4800     =0x1d,        /* data CDS 4800 bps              */
  KSD_BS_CDS_9600     =0x1e,        /* data CDS 9600 bps              */
  KSD_BS_ALL_DATA_PAD =0x20,        /* all data PAD services          */
  KSD_BS_PAD_300      =0x21,        /* data PAD  300 bps              */
  KSD_BS_PAD_1200     =0x22,        /* data PAD 1200 bps              */
  KSD_BS_PAD_1200_75  =0x23,        /* data PAD 1200/75 bps           */
  KSD_BS_PAD_2400     =0x24,        /* data PAD 2400 bps              */
  KSD_BS_PAD_4800     =0x25,        /* data PAD 4800 bps              */
  KSD_BS_PAD_9600     =0x26,        /* data PAD 9600 bps              */
  KSD_BS_ALL_DATA_PDS =0x28,        /* all data PDS services          */
  KSD_BS_PDS_2400     =0x2c,        /* data PDS 2400 bps              */
  KSD_BS_PDS_4800     =0x2d,        /* data PDS 4800 bps              */
  KSD_BS_PDS_9600     =0x2e,        /* data PDS 9600 bps              */
  KSD_BS_SPCH_ALT_CDA =0x30,        /* all data CDA alter. speech     */
  KSD_BS_SPCH_ALT_CDS =0x38,        /* all data CDS alter. speech     */
  KSD_BS_SPCH_FLD_CDA =0x40,        /* all data speech followed CDA   */
  KSD_BS_SPCH_FLD_CDS =0x48,        /* all data speech followed CDA   */
  KSD_BS_ALL_DC_ASYN  =0x50,        /* all data circuit asynchronous  */
  KSD_BS_ALL_ASYN     =0x60,        /* all asynchronous services      */
  KSD_BS_ALL_DC_SYN   =0x58,        /* all data circuit synchronous   */
  KSD_BS_ALL_SYN      =0x68,        /* all synchronous services       */
  KSD_BS_ALL_PSSS     =0xd0,        /* all PLMN specific BS           */
  KSD_BS_PLMN1        =0xd1,        /* PLMN specific 1                */
  KSD_BS_PLMN2        =0xd2,        /* PLMN specific 2                */
  KSD_BS_PLMN3        =0xd3,        /* PLMN specific 3                */
  KSD_BS_PLMN4        =0xd4,        /* PLMN specific 4                */
  KSD_BS_PLMN5        =0xd5,        /* PLMN specific 5                */
  KSD_BS_PLMN6        =0xd6,        /* PLMN specific 6                */
  KSD_BS_PLMN7        =0xd7,        /* PLMN specific 7                */
  KSD_BS_PLMN8        =0xd8,        /* PLMN specific 8                */
  KSD_BS_PLMN9        =0xd9,        /* PLMN specific 9                */
  KSD_BS_PLMNA        =0xda,        /* PLMN specific A                */
  KSD_BS_PLMNB        =0xdb,        /* PLMN specific B                */
  KSD_BS_PLMNC        =0xdc,        /* PLMN specific C                */
  KSD_BS_PLMND        =0xdd,        /* PLMN specific D                */
  KSD_BS_PLMNE        =0xde,        /* PLMN specific E                */
  KSD_BS_PLMNF        =0xdf         /* PLMN specific F                */

} T_ACI_KSD_BEARER_SERVICE;

typedef enum              /* KSD SS Status */
{
  KSD_ST_NOT_VALID = 0xFF,/* no valid SS status */
  KSD_ST_NONE = 0,        /* not registered, not active, not provisioned,
                             not quiescent */
  KSD_ST_A,               /* Active                         */
  KSD_ST_R,               /* Registered                     */
  KSD_ST_RA,              /* Registered,Active              */
  KSD_ST_P,               /* Provisioned                    */
  KSD_ST_PA,              /* Provisioned,Active             */
  KSD_ST_PR,              /* Provisioned,Registered         */
  KSD_ST_PRA,             /* Provisioned,Registered,Active  */
  KSD_ST_Q,               /* Quiescent                      */
  KSD_ST_QA,              /* Quiescent,Active               */
  KSD_ST_QR,              /* Quiescent,Registered           */
  KSD_ST_QRA,             /* Quiescent,Registered,Active    */
  KSD_ST_QP,              /* Quiescent,Provisioned          */
  KSD_ST_QPA,             /* Quiescent,Provisioned,Active   */
  KSD_ST_QPR,             /* Quiescent,Provisioned,Registered */
  KSD_ST_QPRA             /* Quiescent,Provisioned,Registered,Active */
}
T_ACI_KSD_ST;

typedef enum              /* KSD CLIR option */
{
  KSD_CO_NOT_VALID = 0xFF, /* no valid CLIR option */
  KSD_CO_PERMANENT = 0x0,  /* CLIR permanent      */
  KSD_CO_TEMPORARY = 0x1,  /* CLIR temporary default */
  KSD_CO_ALLOWED   = 0x2   /* CLIR Allowed temporary default  */
}
T_ACI_KSD_CLIR_OP;

typedef enum
{
  KSD_OVR_CAT_NOT_VALID = 0xFF, /* no valid CLIR option */
  KSD_OVR_CAT_ENABLED   = 0x0,  /* Override enabled  */
  KSD_OVR_CAT_DISABLED  = 0x1   /* Override disabled */
}
T_ACI_KSD_OVRD_CTG;

typedef enum
{
  KSD_NO_ERROR                  = 0x0,
  KSD_ERR_UNKNOWN_SUBSCRIBER    = 0x1,
  KSD_ERR_ILLEGAL_SUBSCRIBER    = 0x9,
  KSD_ERR_BEARER_SVC_NOT_PROV   = 0xa,
  KSD_ERR_TELE_SVC_NOT_PROV     = 0xb,
  KSD_ERR_ILLEGAL_EQUIPMENT     = 0xc,
  KSD_ERR_CALL_BARRED           = 0xd,
  KSD_ERR_ILLEGAL_SS_OPERATION  = 0x10,
  KSD_ERR_SS_ERR_STATUS         = 0x11,
  KSD_ERR_SS_NOT_AVAIL          = 0x12,
  KSD_ERR_SS_SUBS_VIOLATION     = 0x13,
  KSD_ERR_SS_INCOMP             = 0x14,
  KSD_ERR_FAC_NOT_SUPPORTED     = 0x15,
  KSD_ERR_ABSENT_SUBS           = 0x1b,
  KSD_ERR_SYSTEM_FAIL           = 0x22,
  KSD_ERR_DATA_MISSING          = 0x23,
  KSD_ERR_UNEXPECT_DATA         = 0x24,
  KSD_ERR_PWD_REG_FAIL          = 0x25,
  KSD_ERR_NEG_PWD_CHECK         = 0x26,
  KSD_ERR_NUM_PWD_VIOLATION     = 0x2b,
  KSD_ERR_UNKNOWN_ALPHA         = 0x47,
  KSD_ERR_USSD_BUSY             = 0x48,
  KSD_ERR_MAX_NUM_MPTY_EXCEED   = 0x7e,
  KSD_ERR_RESOURCE_NOT_AVAIL    = 0x7f,
  KSD_GEN_PROB_UNRECOG_CMP      = 0xA0,
  KSD_GEN_PROB_MISTYPED_CMP     = 0xA1,
  KSD_GEN_PROB_BAD_STRUCT_CMP   = 0xA2,
  KSD_INV_PROB_DUPL_INV_ID      = 0xB0,
  KSD_INV_PROB_UNRECOG_OP       = 0xB1,
  KSD_INV_PROB_MISTYPED_PAR     = 0xB2,
  KSD_INV_PROB_RESOURCE_LIM     = 0xB3,
  KSD_INV_PROB_INIT_RELEASE     = 0xB4,
  KSD_INV_PROB_UNRECOG_LNK_ID   = 0xB5,
  KSD_INV_PROB_LNK_RES_UNEXP    = 0xB6,
  KSD_INV_PROB_UNEXP_LNK_OP     = 0xB7,
  KSD_RES_PROB_UNRECOG_INV_ID   = 0xC0,
  KSD_RES_PROB_RET_RES_UNEXP    = 0xC1,
  KSD_RES_PROB_MISTYPED_PAR     = 0xC2,
  KSD_ERR_PROB_UNRECOG_INV_ID   = 0xD0,
  KSD_ERR_PROB_RET_ERR_UNEXP    = 0xD1,
  KSD_ERR_PROB_UNRECOG_ERR      = 0xD2,
  KSD_ERR_PROB_UNEXP_ERR        = 0xD3,
  KSD_ERR_PROB_MISTYPED_PAR     = 0xD4,
  KSD_ERR_FATAL_INV_RESULT      = 0xE0,
  KSD_ERR_FATAL_CCD_DEC         = 0xE1,
  KSD_ERR_FATAL_SS_ENT          = 0xE2
}
T_ACI_KSD_ERR;

typedef enum       /* type of key sequence */
{
#ifdef SMI
  SEQGRP_KEYPAD_IND,
  SEQGRP_LCD_TEST,
  SEQGRP_SET_ABBR_DIAL,
  SEQGRP_SHOW_CALL_TABLE,

  SEQGRP_SMS_SEND,
  SEQGRP_SMS_SEND_FROM_MEM,
  SEQGRP_SMS_WRITE,
  SEQGRP_SMS_DELETE,
  SEQGRP_SMS_READ,
  SEQGRP_SMS_LIST,
  SEQGRP_SMS_READ_SINGLE,

  SEQGRP_SND_CBST,
  SEQGRP_SND_CRLP,
  SEQGRP_SND_DS,

  SEQGRP_DATA_SPEECH,
  SEQGRP_SPEECH_DATA,
#endif

  SEQGRP_CHANGE_REGISTER,
  SEQGRP_SET_REGISTER,
  SEQGRP_START_REGISTER,

  SEQGRP_ACT_SIM_LOCK,
  SEQGRP_DEACT_SIM_LOCK,
  SEQGRP_INTRGT_SIM_LOCK,

  SEQGRP_DTMF,

  SEQGRP_SUP_CLIR,
  SEQGRP_INV_CLIR,
  SEQGRP_SUP_CLIP,
  SEQGRP_INV_CLIP,
  SEQGRP_SUP_COLR,
  SEQGRP_INV_COLR,
  SEQGRP_SUP_COLP,
  SEQGRP_INV_COLP,
  SEQGRP_TTY_SERV,

  SEQGRP_CB,
  SEQGRP_CF,
  SEQGRP_CHLD,
  SEQGRP_CL,
  SEQGRP_CW,
  SEQGRP_DIAL,
  SEQGRP_DIAL_IDX,
  SEQGRP_PRSNT_IMEI,
  SEQGRP_PWD,
  SEQGRP_UBLK,
  SEQGRP_UNKNOWN,
  SEQGRP_USSD,
  SEQGRP_CCBS,
  SEQGRP_CTFR,

  SEQGRP_HOOK_OFF,
  SEQGRP_HOOK_ON,

  SEQGRP_MS_OFF,
  SEQGRP_MS_ON,

  SEQGRP_EM_MODE
}
T_KSD_SEQGRP;

typedef struct     /* parameter list of function cmh_ksdCB */
{
  UBYTE opCd;
  UBYTE ssCd;
  UBYTE bsCd;
  UBYTE *pwd;
} T_ACI_KSD_CB_PRM;

typedef struct     /* parameter list of function cmh_ksdCF */
{
  UBYTE opCd;
  UBYTE ssCd;
  UBYTE bsCd;
  UBYTE *num;
  UBYTE npi;
  UBYTE ton;
  UBYTE *sub;
  UBYTE tos;
  UBYTE oe;
  UBYTE time;
} T_ACI_KSD_CF_PRM;

typedef struct     /* parameter list of function cmh_ksdCL */
{
  UBYTE opCd;
  UBYTE ssCd;
} T_ACI_KSD_CL_PRM;

typedef struct     /* parameter list of function cmh_ksdCW */
{
  UBYTE opCd;
  UBYTE bsCd;
} T_ACI_KSD_CW_PRM;

typedef struct     /* parameter list of function cmh_ksdPWD */
{
  UBYTE ssCd;
  UBYTE *oldPwd;
  UBYTE *newPwd;
  UBYTE *newPwd2;
} T_ACI_KSD_PWD_PRM;

typedef struct     /* parameter list of function cmh_ksdUBLK */
{
  UBYTE ssCd;
  UBYTE *puk;
  UBYTE *pin;
} T_ACI_KSD_UBLK_PRM;

typedef struct     /* parameter list of function cmh_ksdUSSD */
{
  UBYTE* ussd;
} T_ACI_KSD_USSD_PRM;

typedef struct     /* parameter list of function cmh_ksdCCBS */
{
  UBYTE opCd;
  UBYTE idx;
} T_ACI_KSD_CCBS_PRM;

typedef struct     /* parameter list used for setting of */
{                  /* abbreviated dialing parameter      */
  CHAR* number;
  BYTE  index;
}
T_KSD_ABBR_DIAL;

typedef struct     /* parameter list of function sAT_PlusCBST */
{
  T_ACI_BS_SPEED speed;
  T_ACI_CBST_NAM name;
  T_ACI_CBST_CE  ce;
}
T_KSD_CBST;

typedef struct     /* parameter list for TTY/CTM service request */
{
  T_ACI_CTTY_REQ req;
}
T_KSD_CTTY;

typedef struct     /* parameter list of function sAT_PlusCHLD */
{
  T_ACI_CHLD_MOD mode;
  CHAR*          call;
}
T_KSD_CHLD;

typedef struct     /* parameter list of function sAT_PlusCTFR */
{
  CHAR      *number;
  T_ACI_TOA type;
  CHAR      *subaddr;
  T_ACI_TOS satype;
}
T_KSD_CTFR;

typedef struct     /* parameter list of function sAT_PlusCLCK */
{
  T_ACI_FAC fac;
  T_ACI_CLCK_MOD mode;
  CHAR*          passwd;
  T_ACI_CLASS    class_type;
}
T_KSD_CLCK;

typedef struct     /* parameter list of function qAT_PlusCLCK */
{
  T_ACI_FAC fac;
}
T_KSD_CLCK_QUERY;

typedef struct     /* parameter list of function sAT_PlusCLIP */
{
  T_ACA_CLIP_MOD mode;
}
T_KSD_CLIP;

typedef struct     /* parameter list of function sAT_PlusCLIR */
{
  T_ACI_CLIR_MOD mode;
}
T_KSD_CLIR;

typedef struct     /* parameter list of function sAT_PlusCOLP */
{
  T_ACA_COLP_MOD mode;
}
T_KSD_COLP;

typedef struct     /* parameter list of function sAT_PlusCOPS */
{
  T_ACI_COPS_MOD  mode;
  T_ACI_COPS_FRMT frmt;
  CHAR*           oper;
}
T_KSD_COPS;

typedef struct     /* parameter list of function sAT_PlusCRLP */
{
  SHORT iws;
  SHORT mws;
  SHORT t1;
  SHORT n2;
}
T_KSD_CRLP;

typedef struct     /* parameter list of function sAT_Dn */
{
  CHAR*             number;
  T_ACI_D_CLIR_OVRD clirOvrd;
  T_ACI_D_CUG_CTRL  cugCtrl;
  T_ACI_D_TOC       callType;
}
T_KSD_DIAL;

typedef struct     /* parameter list of function sAT_Dm */
{
  CHAR*             str;
  T_ACI_PB_STOR     mem;
  SHORT             index;
  T_ACI_D_CLIR_OVRD clirOvrd;
  T_ACI_D_CUG_CTRL  cugCtrl;
  T_ACI_D_TOC       callType;
}
T_KSD_DIAL_IDX;

typedef struct     /* parameter list of function sAT_PlusDS */
{
  T_ACI_DS_DIR  dir;
  T_ACI_DS_COMP comp;
  LONG          maxDict;
  SHORT         maxStr;
}
T_KSD_DS;

typedef struct     /* parameter list of the keypad indication */
{
  UBYTE keyCode;
  UBYTE keyStat;
}
T_KSD_KEYPAD_IND;

typedef union      /* decoded parameter for ACI function */
{
  T_ACI_KSD_CB_PRM   cb;
  T_ACI_KSD_CF_PRM   cf;
  T_ACI_KSD_CL_PRM   cl;
  T_ACI_KSD_CW_PRM   cw;
  T_ACI_KSD_PWD_PRM  pwd;
  T_ACI_KSD_UBLK_PRM ublk;
  T_ACI_KSD_USSD_PRM ussd;
  T_ACI_KSD_CCBS_PRM ccbs;

  CHAR               dtmf;
  T_KSD_ABBR_DIAL    abbrDial;
  T_KSD_CBST         cbst;
  T_KSD_CTTY         ctty;
  T_KSD_CHLD         chld;
  T_KSD_CTFR         ctfr;
  T_KSD_CLCK         clck;
  T_KSD_CLCK_QUERY   clck_query;
  T_KSD_CLIP         Clip; /* capital C in Clir, Clip and Colp is    */
  T_KSD_CLIR         Clir; /* necessary to avoid identifier conflicts*/
  T_KSD_COLP         Colp; /* (see constants in project MFW)         */
  T_KSD_COPS         cops;
  T_KSD_CRLP         crlp;
  T_KSD_DIAL         dial;
  T_KSD_DIAL_IDX     dialIdx;
  T_KSD_DS           ds;
  T_KSD_KEYPAD_IND   keypad;
}
T_KSD_SEQPARAM;

/*==== PROTOTYPES =================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_decode               |
+--------------------------------------------------------------------+

  PURPOSE : This function decodes the incoming keystroke sequence.

            <inSeq>:       key sequence, to be decoded
            <inCall>:      TRUE if MMI is within a call, otherwise
                           FALSE
            <outSeqGrp>:   sequence group
            <outRestSeq>:  rest key sequence, to be decoded by a
                           further call to this function
            <outSeqParam>: sequence parameter

            returns:       TRUE if decoding was successfull,
                           otherwise FALSE
*/
GLOBAL BOOL ksd_decode (CHAR*           inSeq,
                        BOOL            inCall,
                        T_KSD_SEQGRP*   outSeqGrp,
                        CHAR**          outRestSeq,
                        T_KSD_SEQPARAM* outSeqParam);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_getPwdHidden         |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to hide passwords which are being
            entered during input of a keystroke sequence.

            <inSeq>:   key sequence to be manipulated
            <replace>: character which is used for replacing of
                       passwords
*/
GLOBAL void ksd_getPwdHidden (CHAR* inSeq,
                              CHAR  replace);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_isSscs               |
+--------------------------------------------------------------------+

  PURPOSE : This function return whether the given string is a
            supplementary service control string.

            <inSeq>:       key sequence

            returns:       TRUE if string is a supplementary service
                           control string, otherwise FALSE
*/
GLOBAL BOOL ksd_isSscs (CHAR* inSeq);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_isSATSscs            |
+--------------------------------------------------------------------+

  PURPOSE : This function return whether the given string is a
            supplementary service control string for SAT.

            <inSeq>:       key sequence

            returns:       TRUE if string is a supplementary service
                           control string for SAT, otherwise FALSE
*/
GLOBAL BOOL ksd_isSATSscs ( CHAR* inSeq );

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_isUSSD               |
+--------------------------------------------------------------------+

  PURPOSE : This function checks whether key sequence is an
            unstructured SS command.

            <keySeq>: buffer containing the key sequence
*/
GLOBAL BOOL ksd_isUSSD (CHAR *keySeq);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_isFDNCheckSeq        |
+--------------------------------------------------------------------+

  PURPOSE : This function return whether the given string must check
            in FDN phonebook

            <inSeq>:       key sequence

            returns:       TRUE if string must check,
                           otherwise FALSE
*/
GLOBAL BOOL ksd_isFDNCheckSeq (CHAR* inSeq);


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_isBCDForUSBand       |
+--------------------------------------------------------------------+

  PURPOSE : This function checks if US band is used and key sequence
            is "0" or "00", because these key sequences shall be send
            to network as normal dialing numbers when using US band.

            <inSeq>:       key sequence

            returns:       TRUE if US band is used and key sequence
                           is "0" or "00",
                           otherwise FALSE
*/
GLOBAL BOOL ksd_isBCDForUSBand (CHAR* inSeq);
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_check_write_to_LDN   |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to decide if the keystroke sequence needs to
            be stored in the LDN or not.

            <inSeq>:       key sequence

            returns:       TRUE if the keystroke sequence doesn't contain password
                           or it is not a local string
                           otherwise FALSE
*/
GLOBAL BOOL ksd_isLDNWriteCheckSeq(CHAR*  inSeq);



/*==== EXPORT =====================================================*/

#endif


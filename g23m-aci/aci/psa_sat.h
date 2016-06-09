/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SAT
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
|  Purpose :  Definitions for the protocol stack adapter
|             SIM application toolkit ( SAT )
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_SAT_H
#define PSA_SAT_H

/*==== CONSTANTS ==================================================*/
#define MAX_SAT_CHANNEL    1    /* maximum number of channels available */

#define TPL_NONE           0    /* no transport layer */
#define TPL_DONT_CARE   0xFF   

#define TIME_STAMP_LENGTH 7

typedef struct            /* data buffer for unstructured SS data */
{
  UBYTE *ussd_str;
  UBYTE c_ussd_str;
  UBYTE dcs;
} T_sat_ussd;

typedef enum
{
  NO_CC_ACT = 0,                /* no call control action */
  CC_ACT_CAL,                   /* call control of setup parms */
  CC_ACT_SS,                    /* call control of SS string */
  CC_ACT_USSD,                  /* call control of USSD string */
  SMC_ACT_MO                    /* MO Short Message control by SIM */
} T_CC_ACT;

typedef enum
{
  NO_USR_NTF = 0,               /* no user notification */
  USR_NTF_CC_SIM,               /* call control by SIM notification */
  USR_NTF_SETUP_CAL,            /* call setup notification */
  USR_NTF_SEND_SS,              /* send SS notification */
  USR_NTF_SEND_USSD,            /* send USSD notification */
  USR_NTF_TRM_RSP               /* terminal response notification */
} T_USR_NTF;

typedef enum
{
  NO_VALID_SRQ = 0,             /* not a valid SAT request id */
  SRQ_ACI,                      /* SAT request by ACI */
  SRQ_MMI                       /* SAT request by MMI */
} T_SAT_RQ_ID;

typedef enum
{
  NO_VALID_CTX = 0,             /* not a valid cap context */
  CTX_CC_RESULT,                /* cap context CC result */
  CTX_SAT_SETUP                 /* cap context SAT call setup */
} T_SAT_CAP_CTX;

#define MAX_FU_OPS 16            /* maximum FILE UPDATE operations */

enum
{
  SAT_FU_START = 0,
  SAT_FU_SMS,
  SAT_FU_PHB,
  SAT_FU_MMI,
  SAT_FU_STOP
};

#ifdef FF_SAT_E
typedef enum
{
  OPCH_IDLE = 0,                /* open channel status idle */
  OPCH_CCSIM_REQ,               /* open channel status CC by SIM request */
  OPCH_WAIT_CNF,                /* open channel status wait for confirmation */
  OPCH_ON_DMND,                 /* open channel status on demand */
  OPCH_EST_REQ,                 /* open channel establishment request */
  OPCH_CLS_REQ,                 /* channel close request */
  OPCH_EST,                     /* channel established */
  OPCH_SUSP,                    /* channel suspended */
  OPCH_NONE
} T_SAT_OPCH_STAT;
#endif 

#ifdef FF_SAT_E
typedef enum
{
  SIM_NO_LINK = 0,              /* no link established */
  SIM_LINK_OPEN,                /* link opened */
  SIM_LINK_CNCT,                /* link connected */
  SIM_LINK_DROP                 /* link dropped */
} T_SAT_SIM_LNK_STAT;
#endif 

/*==== TYPES ======================================================*/

typedef struct
{
  UBYTE     stkCmdLen;              /* length of STK command */
  UBYTE   * stkCmd;                 /* STK command */
} T_SAT_SET_PRM;

typedef struct
{
  UBYTE cmdNr;                      /* command number */
  UBYTE cmdType;                    /* command type */
  UBYTE cmdQlf;                     /* command qualifier */
} T_SAT_CMD_DET;

typedef struct
{
  BOOL     busy;                    /* CC busy flag */
  UBYTE    owner;                   /* CC owner */
  SHORT    cId;                     /* related call id */
  UBYTE    ccAct;                   /* CC action */
} T_SAT_CC_PRM;

typedef struct
{
  SHORT    cId;                     /* related call id */
  UBYTE    cntxt;                   /* cap context */
  UBYTE    CCres;                   /* previous CC result */
} T_SAT_CAP_PRM;

typedef struct
{
  UBYTE  regStatus;                  /* Indicates the current reg status */
  T_plmn currPLMN;                   /* The current PLMN that the ME is registered to 
                                         valid only if the regStatus is Full Service */
  U16    lac;                        /* current lac */
  U16    cid;                        /* current cell id */ 
} T_SAT_LOC_STATUS_INFO;

typedef struct
{
  T_stk_cmd    *stk_cmd;            /* pointer to coded envelope of queued event */
  T_OWN        owner;               /* owner of corresponding event */
} T_SAT_QUEUE;

#define MAX_EVENT_QUEUED (4)

typedef struct
{
  ULONG        temp_list;          /* buffer to save list waiting result from MMI */
                                   /* highest bit flags if a setup list is in process */
  ULONG        list;               /* contain list of events to be monitored */
  UBYTE        c_queued;           /* count of queued events */
  T_SAT_QUEUE  queued[MAX_EVENT_QUEUED]; /* list of queued events MAX 4 (may more be needed ??)*/
} T_SAT_EVENT;

typedef BOOL (* T_SAT_FU_FUNC)(int, T_SIM_FILE_UPDATE_IND *);

#ifdef FF_SAT_E
typedef struct
{
  UBYTE chnUsdFlg;                 /* flags a used channel */
  SHORT chnRefId;                  /* refers to call id for CSD, or CID for GPRS */
  UBYTE chnType;                   /* channel type (CSD/GPRS) */
  UBYTE chnTPL;                    /* channel transport layer */
  UBYTE lnkStat;                   /* sim link status */
} T_SAT_CHN_NTRY;
#endif /* FF_SAT_E */

typedef struct SATShrdParm
{
  UBYTE         owner;                  /* identifies the used set */
  UBYTE         ntfy;                   /* user notification */
  T_SAT_SET_PRM setPrm[OWN_SRC_MAX];        /* possible sets */
  T_SAT_CC_PRM  SIMCCParm;              /* call control by SIM parameter */
  T_SAT_CAP_PRM capParm;                /* cap request parameter */
  T_SAT_LOC_STATUS_INFO locInfo;        /* Location status info for event download */
  T_SAT_CMD_DET cmdDet;                 /* command details */
  LONG          dur;                    /* redial duration */
  UBYTE        *stkCmd;                 /* points to STK command */
  USHORT        stkCmdLen;              /* length of STK command */
#ifdef TI_PS_FF_AT_P_CMD_CUST
  UBYTE         *cust1StkCmd;    /* pointer to dynamic memory storing the Stk Cmd for Cust1 Specific implementation */
  USHORT       cust1StkCmdLen; /* length of the the Stk Cmd in cust1StkCmd */
  UBYTE          cust1SimRefreshRespRqd; /* A Refresh Command has been sent to the MMI and a Response is Required */
#endif /* TI_PS_FF_AT_P_CMD_CUST */
  USHORT        fu_rsc;                 /* result code of FILE UPDATE */
  BUF_cmd_prms  stkCmdPrm;              /* proactive SIM command parameter */
  T_SIM_TOOLKIT_CNF *stkCnfPrim;        /* pending result primitive */
  T_SIM_FILE_UPDATE_IND *fu_ind;        /* actual FILE UPDATE primitive */
  T_SAT_FU_FUNC fu_func_notif;          /* registered FILE UPDATE NOTIFY handler */
  T_SAT_EVENT   event;                  /* flags events to be monitored for SAT */
//  CHAR          ToBeSentDTMF[MAX_DIAL_LEN]; /* buffers DTMF string to be sent */
//  BYTE          DTMFidx;                /* index of Send DTMF string */
  SHORT         SentUSSDid;             /* ssId of USSD sent */
  BOOL          USSDterm;               /* flags if user is aiming at terminating ussd */
  USHORT        stkError;               /* error code from SAT application */
#ifdef FF_SAT_E  
  UBYTE         opchStat;               /* open channel status */
  UBYTE         opchType;               /* open channel type */
  void         *opchPrm;                /* points to open channel parameters */
  UBYTE         opchPrmMdf;             /* flags bearer parameter modification */
  UBYTE         opchAcptSrc;            /* open channel source identifier */
  UBYTE         opchCCMdfy;             /* flags open channel call control modification */
  UBYTE         gprsNtwCs;              /* gprs network cause */
  U16             buffer_size; 
  T_SAT_CHN_NTRY chnTb;                 /* channel table, must be an array if more than one channel will be supported */
 #endif /* FF_SAT_E */  
  SHORT         sId_pwd_requested;      /* needed when sat starts a SS transaction which requires a password */
  T_SAT_FU_FUNC fu_func[MAX_FU_OPS];    /* registered FILE UPDATE handlers */
  BOOL          Cbch_EvtDnl;            /* Flag used for SAT CBCH and Event Download Indication */
  BOOL          ownSAT;                 /* Flag to decide the owner in SIM_TOOLKIT_CNF */
  BUF_cap_cnf_parms stk_ccp;            /* Capability parameters */
} T_SAT_SHRD_PRM;

typedef struct
{
  UBYTE     addLen;
  UBYTE     *add;
  UBYTE     add_content; /* when addLen = 1 */
  UBYTE     *resCC;
  T_text    *text;
  UBYTE     *at_resp;
  USHORT    at_resp_count;
#ifdef FF_SAT_E  
  UBYTE     chnStat;
  UBYTE     bearDesc;
  UBYTE     bufSize;
#endif /* FF_SAT_E */  
  UBYTE     dtt_buf[TIME_STAMP_LENGTH];
  UBYTE     lang[CLAN_CODE_LEN];
} T_ACI_SAT_TERM_RESP;

#ifdef FF_SAT_E  
typedef struct
{
  UBYTE            def_bear_prm;  /* true or false for default parameters */
  T_csd_bear_prm   csd_bear_prm;  /* if no def. parms. use CSD parms.*/
  UBYTE            v_dur;         /* duration 1 avail.*/
  T_dur            dur;           /* duration 1 */
  UBYTE            v_dur2;        /* duration 2 avail.*/
  T_dur2           dur2;          /* duration 2 */
  UBYTE            v_other_addr;  /* other addr. avail.*/
  T_other_addr     other_addr;    /* other addr.*/
  UBYTE            v_log;         /* login name avail.*/
  T_text           log;           /* login name */
  UBYTE            v_pwd;         /* password avail.*/
  T_text2          pwd;           /* password */
  UBYTE            v_itl;         /* if transp level avail.*/
  T_if_transp_lev  itl;           /* if transp level */
  UBYTE            v_dda;         /* data dest. addr. avail.*/
  T_data_dest_addr dda;           /* data dest. addr.*/
} T_SAT_CSD_PRM;  

typedef struct
{
  UBYTE            def_bear_prm;   /* true or false for default parameters */
  T_gprs_bear_prm  gprs_bear_prm;  /* if no def. parms. use GPRS parms.*/
  UBYTE            v_apn;          /* apn avail.*/
  UBYTE            c_apn;          /* apn length */
  UBYTE      apn[MAX_SAT_APN_LEN]; /* apn */
  UBYTE            v_other_addr;   /* other addr. avail.*/
  T_other_addr     other_addr;     /* other addr.*/
  UBYTE            v_itl;          /* if transp level avail.*/
  T_if_transp_lev  itl;            /* if transp level */
  UBYTE            v_dda;          /* data dest. addr. avail.*/
  T_data_dest_addr dda;            /* data dest. addr.*/
} T_SAT_GPRS_PRM;
#endif /* FF_SAT_E */  


/*==== PROTOTYPES =================================================*/

EXTERN BOOL  psaSAT_ChkEventList    ( UBYTE eventNr );

EXTERN SHORT psaSAT_STKBuildCmd     ( T_stk_cmd *stk_cmd );
EXTERN SHORT psaSAT_STKResponse     ( void );
EXTERN SHORT psaSAT_STKEnvelope     ( const T_stk_cmd *stk_cmd );
EXTERN void psaSAT_SendRefreshUserRes( SHORT len, UBYTE *satCmd );
EXTERN void  psaSAT_Init            ( void );
EXTERN void  psaSAT_BuildEnvCC      ( SHORT        cId,
                                      T_CLPTY_PRM *ss_cldPty,
                                      T_sat_ussd  *ussd, 
                                      T_MNCC_bcconf *ccp1, 
                                      T_MNCC_bcconf2 *ccp2);
EXTERN void  psaSAT_BuildEnvCB      ( UBYTE *cbMsg, SHORT cbLen );
EXTERN void  psaSAT_BuildEnvMoSmCntr( T_rp_addr sc_addr,
                                      T_tp_da   dest_addr );
EXTERN BOOL  psaSAT_BuildEnvEventDwn( UBYTE event, 
                                      SHORT callId,
                                      T_CC_INITIATER actionSrc);

EXTERN BOOL  psaSAT_SendTrmResp     ( UBYTE rspId,
                                      T_ACI_SAT_TERM_RESP *data_for_term_resp);
EXTERN void  psaSAT_InitTrmResp     ( T_ACI_SAT_TERM_RESP *init_resp );

EXTERN BOOL  psaSAT_dasmMECmd       ( BUF_cmd_prms *cmdPrm );
EXTERN void  psaSAT_SSResComp       ( T_res_comp* resCmp );
EXTERN void  psaSAT_SSErrComp       ( T_fac_inf * errCmp,
                                      BOOL      is_fac_ussd );

EXTERN void psaSAT_SSRejComp  ( UBYTE cRejectInfo );
EXTERN T_ACI_RETURN psaSAT_ss_end_ind ( SHORT sId,
                                        T_COMPONENT    *comp, 
                                        T_MNSS_END_IND *mnss_end_ind,
                                        BOOL           is_fac_ussd );

EXTERN UBYTE psaSAT_ccdErrChk       ( void );
EXTERN void  psaSAT_DumpCmd         ( T_stk_cmd * cmd );
EXTERN BOOL  psaSAT_FURegister      ( T_SAT_FU_FUNC );
EXTERN void  psaSAT_FUConfirm       ( int, USHORT );
EXTERN BOOL  psaSAT_FUNotifyRegister ( T_SAT_FU_FUNC );

#ifdef FF_SAT_E 
EXTERN SHORT psaSAT_ctbFindActCall  ( void );
EXTERN SHORT psaSAT_gprsFindFreeCntxt( void );
#endif /* FF_SAT_E */
/*==== EXPORT =====================================================*/

#ifdef PSA_SATF_C

GLOBAL T_SAT_SHRD_PRM satShrdPrm;
GLOBAL UBYTE cmpFlg = FALSE;
/*-------------------------------------------------------------------
The resulting Terminal Profile is created with the following formula:
resultingPrfl = (actualMMIPrfl & satMaskMMIPrfl) | satDefPrfl
-------------------------------------------------------------------*/
/* SAT Features supported mainly by ACI (no MMI support needed) */
const GLOBAL UBYTE satDefPrfl[MAX_STK_PRF] =
                   {
                    SAT_TP1_PRF_DNL |    /* Profile Download */
                    SAT_TP1_CB_DNL  |    /* Class 2: CBCH Data Download */
                    SAT_TP1_CC_USSD |    /* Class 3: USSD string data object
                                                     supported in call control */
                    SAT_TP1_CC_ON_REDIAL,/* Envelope Call Control always sent to 
                                            the SIM during automatic redial mode */
                    SAT_TP2_CMD_RES |    /* Command Result */
                    SAT_TP2_CC      |    /* Class 2: Call Control by SIM */
                    SAT_TP2_CC_CELL_ID | /* Class 2: Cell Identity incl. in CC */
                    SAT_TP2_MOSMC,       /* Class 3: MO Short Message Control */

                    SAT_TP3_REFRESH,     /* Class 2: REFRESH */

                    SAT_TP4_SEND_SMS,    /* Class 2: SEND SM */

                    SAT_TP5_EVENT_LIST | /* Class 3: SET UP EVENT LIST */
                    SAT_TP5_MT_CALL    | /* Class 3: MT Call */
                    SAT_TP5_CALL_CONN  | /* Class 3: Call connected */
                    SAT_TP5_CALL_DISC  | /* Class 3: Call disconnected */
                    SAT_TP5_LOC_STATUS,  /* Class 3: Location status */

                    0x00,
                    0x00,

                    SAT_TP8_PLI_DTT |    /* Class 3: PLI (Date, time, timezone) */
                    SAT_TP8_AT_CMD |     /* Class b: Run AT command */
                    SAT_TP8_CCP2_CC,     /* Class 3: 2nd CCP in SETUP CALL */

                    SAT_TP9_DTMF_CMD|    /* Class 3: SEND DTMF */
                    SAT_TP9_PLI_LANG,    /* Provide Local Information (Language) */

                    0x00,
                    0x00,
#if defined (FF_SAT_E)
                    SAT_TP12_OPEN_CHANNEL |     /* Class e: OPEN CHANNEL */
                    SAT_TP12_CLOSE_CHANNEL |    /* Class e: CLOSE CHANNEL */
                    SAT_TP12_GET_CHANNEL_STAT,  /* Class e: GET CHANNEL STATUS */
#if defined (FAX_AND_DATA)
                    SAT_TP13_CSD_SUPP_BY_ME |   /* Class e: CSD supported */
#endif /* FAX_AND_DATA */
#if defined (GPRS)
                    SAT_TP13_GPRS_SUPP_BY_ME |  /* Class e: GPRS supported */
#endif /* GPRS */
                    (SAT_TP13_NR_OF_CHAN_SUPP & (1<<5)),  /* Class e: number of channels supported */
#else 
                    0x00,
                    0x00,
#endif /* (FAX_AND_DATA OR GPRS) AND FF_SAT_E*/
                    0x00,
                    0x00,
                    0x00,
#if defined (FF_SAT_E)
                    SAT_TP17_BEARER_IND_SUPP_UDP,   /* Class e: UDP supported */
#else 
                    0x00,
#endif /* FF_SAT_E */
                    0x00,
                    0x00,
                    0x00
                    };

/* SAT Features supported together with MMI */
const GLOBAL UBYTE satMaskMMIPrfl[MAX_STK_PRF] =
                   {
                    SAT_TP1_PRF_DNL |    /* Profile Download */
                    SAT_TP1_MENU_SEL,    /* Class 2: Menu Selection */
                    SAT_TP2_CMD_RES |    /* Command Result */
                    SAT_TP2_CC      |    /* Class 2: Call Control by SIM */
                    SAT_TP2_MOSMC   |    /* Class 3: MO Short Message Control */
                    SAT_TP2_ALPHA_ID   | /* Class 2: Alpha Id Handling 9.1.3  */
                    SAT_TP2_UCS2_ENTRY | /* Class 2: UCS2 Entry supported     */
                    SAT_TP2_UCS2_DSPL  | /* Class 2: UCS2 Display supported   */
                    SAT_TP2_DSPL_EXT,    /* Class 3: Display of extended text */

                    SAT_TP3_DSPL_TXT   | /* Class 2: DISPLAY TEXT */
                    SAT_TP3_GET_INKEY  | /* Class 2: GET INKEY */
                    SAT_TP3_GET_INPUT  | /* Class 2: GET INPUT */
                    SAT_TP3_PLAY_TONE  | /* Class 2: PLAY TONE */
                    SAT_TP3_REFRESH,     /* Class 2: REFRESH */

                    SAT_TP4_SEL_ITEM   | /* Class 2: SELECT ITEM */
                    SAT_TP4_SEND_SS    | /* Class 2: SEND SS */
                    SAT_TP4_SEND_USSD  | /* Class 3: SEND USSD */
                    SAT_TP4_SETUP_CALL | /* Class 2: SETUP CALL */
                    SAT_TP4_SETUP_MENU,  /* Class 2: SETUP MENU */

                    SAT_TP5_EVENT_LIST | /* Class 3: SETUP EVENT LIST */
                    SAT_TP5_USER_ACT   | /* Class 3: User activity */
                    SAT_TP5_SCR_AVAIL,   /* Class 3: Idle Screen available */

                     SAT_TP6_BROWS_TERM|SAT_TP6_LANG_SEL,
                    0x00,

                    SAT_TP8_BIN_GET_INKEY | /* Class 3: Binary Choice in GET INKEY */
                    SAT_TP8_IDLE_TXT      | /* Class 3: SETUP IDLE MODE TEXT */
                    SAT_TP8_AI2_SETUP_CALL, /* Class 3: 2nd alpha identifier in SETUP CALL */
#if defined (FF_WAP)
                    SAT_TP9_SUST_DSPL_TXT |  /* Class 3: Sustained DISPLAY TEXT */
                    SAT_TP9_LAUNCH_BROWSER|
                    SAT_TP9_PLI_LANG |
                    SAT_TP9_LANG_NOTIFY,  /* Class 3: LAUNCH BROWSER */

#else
                    SAT_TP9_SUST_DSPL_TXT|
                    SAT_TP9_PLI_LANG |
                    SAT_TP9_LANG_NOTIFY,   /* Class 3: Sustained DISPLAY TEXT */ 
#endif                    
                    SAT_TP10_SFTKEY_SEL_ITEM |  /* soft key support SELECT ITEM */
                    SAT_TP10_SFTKEY_SETUP_MENU, /* soft key support SET UP MENU */

                    SAT_TP11_MAX_NR_SFTKEY,     /* number of soft keys available */

                    0x00,
                    0x00,

                    SAT_TP14_NR_OF_CHAR_DSPL_DWN| /* number of characters supported down ME display */
                    SAT_TP14_SCRN_SIZE_PARAM,     /* screen sizing parameter supported */

                    SAT_TP15_NR_OF_CHAR_DSPL_ACRS|/* number of characters supported across ME display */
                    SAT_TP15_VAR_SIZE_FONT,       /* variable size fonts supported */

                    SAT_TP16_DSPL_RESIZE |        /* display can be resized */
                    SAT_TP16_TEXT_WRAP |          /* text wrapping supported */
                    SAT_TP16_TEXT_SCROLL |        /* text scrolling supported */
                    SAT_TP16_WIDTH_RDCT_MENU,     /* width reduction in a menu */

                    0x00,
                    0x00,
                    0x00,
                    0x00
                   };

#ifdef TI_PS_FF_AT_P_CMD_CUST
/* SAT Features supported together with Cust1 MMI */
const GLOBAL UBYTE satMaskCust1Prfl[MAX_STK_PRF] =
                   {
                    SAT_TP1_PRF_DNL |    /* Profile Download */
                    SAT_TP1_MENU_SEL,    /* Class 2: Menu Selection */

                    SAT_TP2_CMD_RES |    /* Command Result */
                    SAT_TP2_CC      |    /* Class 2: Call Control by SIM */
                    SAT_TP2_MOSMC   |    /* Class 3: MO Short Message Control */
                    SAT_TP2_ALPHA_ID   | /* Class 2: Alpha Id Handling 9.1.3  */
                    SAT_TP2_UCS2_ENTRY | /* Class 2: UCS2 Entry supported     */
                    SAT_TP2_UCS2_DSPL  | /* Class 2: UCS2 Display supported   */
                    SAT_TP2_DSPL_EXT,    /* Class 3: Display of extended text */

                    SAT_TP3_DSPL_TXT   | /* Class 2: DISPLAY TEXT */
                    SAT_TP3_GET_INKEY  | /* Class 2: GET INKEY */
                    SAT_TP3_GET_INPUT  | /* Class 2: GET INPUT */
                    SAT_TP3_PLAY_TONE  | /* Class 2: PLAY TONE */
                    SAT_TP3_REFRESH,     /* Class 2: REFRESH */

                    SAT_TP4_SEL_ITEM   | /* Class 2: SELECT ITEM */
                    SAT_TP4_SEND_SS    | /* Class 2: SEND SS */
                    SAT_TP4_SEND_USSD  | /* Class 3: SEND USSD */
                    SAT_TP4_SETUP_CALL | /* Class 2: SETUP CALL */
                    SAT_TP4_SETUP_MENU,  /* Class 2: SETUP MENU */

                    SAT_TP5_EVENT_LIST | /* Class 3: SETUP EVENT LIST */
                    SAT_TP5_USER_ACT   | /* Class 3: User activity */
                    SAT_TP5_SCR_AVAIL,   /* Class 3: Idle Screen available */

                    SAT_TP6_LANG_SEL,
                    0x00,

                    SAT_TP8_BIN_GET_INKEY | /* Class 3: Binary Choice in GET INKEY */
                    SAT_TP8_IDLE_TXT      | /* Class 3: SETUP IDLE MODE TEXT */
                    SAT_TP8_AI2_SETUP_CALL, /* Class 3: 2nd alpha identifier in SETUP CALL */
#if defined (FF_WAP)
                    SAT_TP9_SUST_DSPL_TXT |  /* Class 3: Sustained DISPLAY TEXT */
                    SAT_TP9_LAUNCH_BROWSER,  /* Class 3: LAUNCH BROWSER */
#else
                    SAT_TP9_SUST_DSPL_TXT,   /* Class 3: Sustained DISPLAY TEXT */ 
#endif                    
                    SAT_TP10_SFTKEY_SEL_ITEM |  /* soft key support SELECT ITEM */
                    SAT_TP10_SFTKEY_SETUP_MENU, /* soft key support SET UP MENU */

                    SAT_TP11_MAX_NR_SFTKEY,     /* number of soft keys available */

                    0x00,
                    0x00,

                    SAT_TP14_NR_OF_CHAR_DSPL_DWN| /* number of characters supported down ME display */
                    SAT_TP14_SCRN_SIZE_PARAM,     /* screen sizing parameter supported */

                    SAT_TP15_NR_OF_CHAR_DSPL_ACRS|/* number of characters supported across ME display */
                    SAT_TP15_VAR_SIZE_FONT,       /* variable size fonts supported */

                    SAT_TP16_DSPL_RESIZE |        /* display can be resized */
                    SAT_TP16_TEXT_WRAP |          /* text wrapping supported */
                    SAT_TP16_TEXT_SCROLL |        /* text scrolling supported */
                    SAT_TP16_WIDTH_RDCT_MENU,     /* width reduction in a menu */

                    0x00,
                    0x00,
                    0x00,
                    0x00
                   };
#endif /* TI_PS_FF_AT_P_CMD_CUST */
#else

EXTERN T_SAT_SHRD_PRM satShrdPrm;
EXTERN UBYTE cmpFlg;
EXTERN UBYTE satDefPrfl[MAX_STK_PRF];
EXTERN UBYTE satMaskMMIPrfl[MAX_STK_PRF];
#ifdef TI_PS_FF_AT_P_CMD_CUST
EXTERN UBYTE satMaskCust1Prfl[MAX_STK_PRF];
#endif /* TI_PS_FF_AT_P_CMD_CUST */
#endif /* PSA_SATF_C */

#endif /* PSA_SAT_H */

/*==== EOF =======================================================*/

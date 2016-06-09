/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PSA
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
|             Call Control ( CC )
+-----------------------------------------------------------------------------
*/

#ifndef PSA_CC_H
#define PSA_CC_H

#include "aci_lst.h"

/*==== CONSTANTS ==================================================*/
#define DEF_CLIR_SUP (MNCC_CLR_SUP)              /* default CLIR suppression */
#define DEF_BC1_UR   (MNCC_UR_NOT_PRES)          /* default user rate BC 1 */
#define DEF_BC2_UR   (MNCC_UR_NOT_PRES)          /* default user rate BC 2 */
#define DEF_BC1_BS   (MNCC_BEARER_SERV_NOT_PRES) /* default bearer service BC 1 */
#define DEF_BC2_BS   (MNCC_BEARER_SERV_NOT_PRES) /* default bearer service BC 2 */
#define DEF_BC1_CE   (MNCC_CONN_ELEM_NOT_PRES)   /* default connection element BC 1 */
#define DEF_BC2_CE   (MNCC_CONN_ELEM_NOT_PRES)   /* default connection element BC 2 */
#define DEF_BC1_SB   (MNCC_STOP_1_BIT)           /* default stop bits BC 1 */
#define DEF_BC2_SB   (MNCC_STOP_1_BIT)           /* default stop bits BC 2 */
#define DEF_BC1_DB   (MNCC_DATA_8_BIT)           /* default data bits BC 1 */
#define DEF_BC2_DB   (MNCC_DATA_8_BIT)           /* default data bits BC 2 */
#define DEF_BC1_PR   (MNCC_PARITY_NONE)          /* default parity BC 1 */
#define DEF_BC2_PR   (MNCC_PARITY_NONE)          /* default parity BC 2 */
#define DEF_BC1_FC   (MNCC_NO_FLOW_CONTROL)      /* default flow control BC 1 */
#define DEF_BC2_FC   (MNCC_NO_FLOW_CONTROL)      /* default flow control BC 2 */
#define DEF_BC1_MT   (MNCC_MT_NONE)              /* default flow control BC 1 */
#define DEF_BC2_MT   (MNCC_MT_NONE)              /* default flow control BC 2 */
#define DEF_BC1_TC   (MNCC_ITC_NONE)             /* default transfer cap BC 1 */
#define DEF_BC1_RA   (MNCC_RATE_ADAPT_NONE)      /* default rate adaption BC 1 */
#define DEF_UD_MD    (MNCC_MD_NOT_PRES)          /* default more user data */
#define DEF_UD_CL    (MNCC_CL_NOT_PRES)          /* default congest level user data */
#define DEF_RPT_IND  (MNCC_RI_NOT_PRES)          /* default repeat indicator */
#define DEF_OS_TOS   (MNCC_TOS_NOT_PRES)         /* default type of subaddress */
#define MAX_ALPHA    (20)                   /* maximum length of alpha identifier */
#define MAX_DTMF_DIG (MAX_DIAL_LEN)         /* maximum number of DTMF digits */

#define NO_ENTRY      (-1)        /* not a valid entry */



typedef enum                      /* call status */
{
  NO_VLD_CS = 0,                  /* not a valid call status */
  CS_IDL,                         /* call idle */
  CS_ACT_REQ,                     /* call active request */
  CS_ACT,                         /* call active */
  CS_HLD_REQ,                     /* call hold request */
  CS_HLD,                         /* call held */
  CS_DSC_REQ,                     /* call disconnect request */
  CS_CPL_REQ,                     /* call completion request */
  CS_MDF_REQ,                     /* call modification request */
  CS_SAT_REQ                     /* call SAT request */
#ifdef FF_SAT_E   
  , CS_SAT_CSD_REQ                  /* call SAT open CSD channel request */
#endif /* FF_SAT_E */
} T_CC_CLST;

typedef enum                      /* data status */
{
  NO_VLD_DS = 0,                  /* not a valid data status */
  DS_IDL,                         /* data idle */
  DS_ACT_REQ,                     /* data active request */
  DS_ACT,                         /* data active */
  DS_DSC_REQ,                     /* data disconnect request */
  DS_ABO_REQ,                     /* data abort request */
  DS_MDF_REQ,                     /* data modification request */
  DS_REST_REQ,                    /* data reestablishment request */
  DS_STOP_REQ,                    /* data stop request */
  DS_TCH_MDF                      /* data TCH modification expected */
} T_CC_DTST;

typedef enum
{
  NO_VLD_CT = 0,                  /* not a valid call type */
  CT_MOC,                         /* mobile originated call */
  CT_MTC,                         /* mobile terminated call */
  CT_NI_MOC,                      /* network initiated mobile originated call */
  CT_MOC_RDL                      /* redialling mobile originated call */
} T_CC_CLTP;

typedef enum
{
  NO_VLD_AS = 0,                  /* not a valid alert state */
  AS_IDL,                         /* alerting not sended */
  AS_PND,                         /* alerting pending */
  AS_SND                          /* alerting sended */
} T_CC_ALST;

typedef enum
{
  NO_VLD_MT = 0,                  /* not a valid message type */
  MT_SETUP,                       /* setup message */
  MT_DISC,                        /* disconnect message */
  MT_ALRT,                        /* alert message */
  MT_PROC,                        /* proceeding message */
  MT_SYNC,                        /* synchronization message */
  MT_PROGR,                       /* progress message */
  MT_CONN                         /* connected message */
} T_CC_MSGT;

typedef enum
{
  NO_VLD_BCRI = 0,                /* not a valid bc request id */
  BCRI_SAT                        /* bc request by SAT */
} T_BC_RQID;

typedef enum
{
  NO_VLD_CCBSS = 0,               /* not a valid CCBS status */
  CCBSS_PSSBL,                    /* CCBS is possible */
  CCBSS_REQ                       /* CCBS registration requested */
} T_CC_CCBSS;

typedef enum
{
  NO_VLD_CD = 0,                  /* not a valid CD status */
  CD_Requested,                   /* CD Request sent */
  CD_Failed,                      /* CD Request failed (Reject, Error) */
  CD_Succeeded,                   /* CD Request succeeded (Result) */
  CD_Notified                     /* CD Notification received */
} T_CD_STAT;

/* Wap States for the parameter CCShrdParm.wapStat */
#if defined (FF_WAP) || defined (FF_SAT_E)
typedef enum
{
  CC_WAP_STACK_DOWN = 0, /* Wapstack is down  */
  CC_WAP_STACK_UP        /* Wapstack is activ */
} T_CC_WAP;
#endif

typedef enum
{
  END_UNDEFINED = -1,
  NEAR_END,
  FAR_END
} T_CC_INITIATER;

typedef enum
{
  TTY_STATE_NONE,
  TTY_STATE_IDLE,
  TTY_STATE_SYNC,
  TTY_STATE_BCAP,
  TTY_STATE_WAIT,
  TTY_STATE_ACTIVE
} T_CC_TTY_STATE;

typedef enum
{
  ALS_CMD_NONE,
  ALS_CMD_SET,
  ALS_CMD_TEST
} T_CC_ALS_CMD;

/*==== TYPES ======================================================*/
typedef struct CCDTMFPrm
{
  SHORT cId;                      /* DTMF related call id */
  UBYTE cnt;                      /* count of DTMF to send */
  UBYTE cur;                      /* current DTMF digit */
  UBYTE dig[MAX_DTMF_DIG+1];      /* buffer for DTMF digits to send +'\0'*/
} T_CC_DTMF_PRM;

/*
 * called party, dynamic structure
 */
typedef struct
{
  UBYTE ton;                      /* type of number                */
  UBYTE npi;                      /* numbering plan identification */
  UBYTE c_called_num;             /* number of BCD digits          */
  UBYTE *called_num;              /* bcd (unpacked)                */
} T_dyn_called_party;

/*
 * redirecting party, dynamic structure
 */
typedef struct
{
  UBYTE ton;                      /* type of number                */
  UBYTE npi;                      /* numbering plan identification */
  UBYTE present;                  /* presentation indicator        */
  UBYTE screen;                   /* screening indicator           */
  UBYTE c_redir_num;              /* number of BCD digits          */
  UBYTE *redir_num;               /* bcd (unpacked)                */
} T_dyn_redir_party;

/*
 * subaddress, dynamic structure
 */
typedef struct
{
  UBYTE tos;                      /* type of subaddress            */
  UBYTE odd_even;                 /* odd / even indicator          */
  UBYTE c_subaddr;                /* length of subaddress info     */
  UBYTE *subaddr;                 /* subaddress information                             */
} T_dyn_redir_party_sub;

typedef struct CCCallTabl
{
  UBYTE     ti;                         /* transaction identifier */
  UBYTE     calStat;                    /* call status */
  UBYTE     calType;                    /* type of call */
  UBYTE     alrtStat;                   /* alerting status */
  BOOL      inBndTns;                   /* in-band tones flag */
  UBYTE     prgDesc;                    /* progress description */
  T_MNCC_bcpara  BC[2];                      /* bearer capabilities */
  UBYTE     curBC;                      /* current bearer capability */
  T_MNCC_calling_party     clgPty;           /* calling party address */
  T_MNCC_calling_party_sub clgPtySub;        /* calling party subaddress */
  T_dyn_called_party  cldPty;           /* called party address */
  T_MNCC_called_party_sub  cldPtySub;        /* called party subaddress */
  T_dyn_redir_party   rdrPty;           /* Redirecting party */
  T_dyn_redir_party_sub rdrPtySub;      /* Redirecting party subaddress */
  /*CHAR      alphId[MAX_ALPHA+1];*/    /* alpha identifier */
  T_ACI_PB_TEXT       alphIdUni;        /* alpha identifier */
  UBYTE     rptInd;                     /* repeat indicator */
  UBYTE     sigInf;                     /* signal information */
  UBYTE     prio;                       /* priority of call */
  UBYTE     CLIRsup;                    /* CLIR suppression */
  UBYTE     mptyStat;                   /* multiparty status */
  UBYTE     iId;                        /* invoke id */
  UBYTE     srvStat;                    /* service status */
  UBYTE     srvType;                    /* type of service */
  UBYTE     SSver;                      /* supplementary service version */
  UBYTE     opCode;                     /* SS operation code */
  UBYTE     CUGidx;                     /* CUG index */
  UBYTE     CUGprf;                     /* preferential CUG */
  UBYTE     OAsup;                      /* OA suppress */
  USHORT    rslt;                       /* result */
  USHORT    nrmCs;                      /* normal cause */
  USHORT    rejCs;                      /* rejection cause */
  UBYTE     failType;                   /* type of SS failure */
  UBYTE     rejPrb;                     /* reject problem code */
  UBYTE     errCd;                      /* return error code */
  UBYTE     ssDiag;                     /* SS diagnostic (CQ 23619 - %DIAG) */
  UBYTE     SATinv;                     /* SAT invocation flag */
  UBYTE     CCBSstat;                   /* CCBS status */
  UBYTE     CDStat;                     /* CD status */
  UBYTE     curCmd;                     /* current command executing */
  S8        curSrc;                     /* current command source */
  BOOL      dtmfCmd;                    /* AT_CMD_VTS or AT_CMD_NONE */
  UBYTE     dtmfMode;                   /* DTMF_MOD_AUTO/DTMF_MOD_MAN_START/DTMF_MOD_MAN_STOP */
  T_OWN     dtmfSrc;                    /* current command source for DTMF */
  T_OWN     calOwn;                     /* owner of call */
#ifdef SIM_TOOLKIT
  BOOL      SatDiscEvent;               /* flag for DISC event for SAT */
#endif
  UBYTE     rdlCnt;                     /* redial counter */
  UBYTE     rdlTimIndex;                /* redial timer index for 5th and more attempts */
  USHORT    curCs;                      /* current cause of rejected call intended for %CPI reports*/
  UBYTE     numRawCauseBytes;           /* Number of cause bytes-for SAT evt Download CTS cases*/
  UBYTE     *rawCauseBytes;             /* Pointer to cause value sent by network*/

} T_CC_CALL_TBL;


typedef struct CCShrdParm
{
  T_ACI_CMOD_MOD CMODmode;
  SHORT cIdFail;                  /* holds failed call identifier */
  SHORT cIdMPTY;                  /* holds multiparty root call id */
  SHORT nrOfMOC;                  /* number of current MOC's */
  SHORT nrOfMTC;                  /* number of current MTC's */
  UBYTE chMod;                    /* channel mode */
  UBYTE chType;                   /* channel type */
  USHORT syncCs;                  /* synchronisation cause */
  BOOL  TCHasg;                   /* TCH assignment flag */
#ifdef FAX_AND_DATA
  T_ACI_BS_SPEED  CBSTspeed;      /* parameters passed to CC by CC_CNFG_REQ: for MTC */
  T_ACI_CBST_NAM  CBSTname;       /* parameters passed to CC by CC_CNFG_REQ: for MTC */
  T_ACI_CBST_CE   CBSTce;         /* parameters passed to CC by CC_CNFG_REQ: for MTC */
#endif /* FAX_AND_DATA */
  UBYTE snsMode;                  /* single numbering scheme mode */
  UBYTE iIdNxt;                   /* next available invoke id */
  UBYTE cmpType;                  /* component type */
  UBYTE msgType;                  /* message type */
  T_CC_CALL_TBL *ctb[MAX_CALL_NR];/* max. nr of calls pointers to call table */
  USHORT ccCs[MAX_CALL_NR];       /* Last CC cause for qAT_PlusCEER() */
  S8 callType[MAX_CALL_NR];       /* Call type T_CC_CALL_TYPE has to survive call end */
  T_CC_DTMF_PRM dtmf;             /* dtmf parameter */
  T_ACI_LIST *facility_list;      /* List with stored MNCC_FACILITY_IND */
  UBYTE wapStat;                  /* Wap connection status */
  UBYTE als_cmd;                  /* ALS command: set or query */
  UBYTE aocRsmpPend;              /* resumption of AOC pending */
  BOOL BC0_send_flag;             /* Flag the 1st bearer capability for data call */
  BOOL BC1_send_flag;             /* Flag the 2nd bearer capability for data call */
  UBYTE datStat;                  /* data connection status */
#ifdef FF_TTY
  UBYTE ctmReq;                   /* TTY/CTM Service to be requested */
  UBYTE ctmState;                 /* State of TTY/CTM Service */
  UBYTE ttyCmd;                   /* Type of TTY: HCO, VCO, bidirectional */
  UBYTE ctmOvwr;                  /* overwrite CTM request for next call */
#endif
  T_ACI_CVHU_MODE cvhu;           /* Voice Hangup Control */
}
T_CC_SHRD_PRM;

/*==== PROTOTYPES =================================================*/

EXTERN SHORT psaCC_NewCall     ( SHORT cId );
EXTERN void psaCC_AcceptCall   ( SHORT cId );
EXTERN void psaCC_ClearCall    ( SHORT cId );
EXTERN void psaCC_HoldCall     ( SHORT cId );
EXTERN void psaCC_RetrieveCall ( SHORT cId );
EXTERN SHORT psaCC_ModifyCall  ( SHORT cId );
EXTERN void psaCC_Config       ( void );
EXTERN void psaCC_BuildMPTY    ( SHORT cId );
EXTERN void psaCC_Hold_RetrieveMPTY ( SHORT     cId, 
                                      T_CC_CLST call_stat,
                                      T_CC_CLST call_stat_new,
                                      UBYTE     cId_new,
                                      UBYTE     opc);
EXTERN void psaCC_SplitMPTY    ( SHORT cId );
EXTERN int  psaCC_CountMPTY    ( void );

EXTERN SHORT psaCC_ECT         ( SHORT cId );

EXTERN SHORT psaCC_SendDTMF ( SHORT cId,
                              UBYTE digit,
                              UBYTE mode );

EXTERN void psaCC_send_satevent( UBYTE event,
                                 SHORT callId ,
                                 T_CC_INITIATER actionSrc,
                                 BOOL check_SatDiscEvent );

GLOBAL void psaCC_init_mtcbearer( void );
GLOBAL void  psaCC_Init ( void );

EXTERN SHORT      psaCC_ctbNewEntry      ( void );
EXTERN SHORT      psaCC_ctbFindTi        ( UBYTE ti2Find );
EXTERN SHORT      psaCC_ctbFindCall      ( T_OWN     calOwn,
                                           T_CC_CLST calStat,
                                           T_CC_CLTP calType );
EXTERN BOOL       psaCC_ctbCallInUse     ( void );
EXTERN BOOL       psaCC_ctbAnyCallInUse  ( void );
EXTERN SHORT      psaCC_ctbDialNr2CldAdr ( SHORT cId, CHAR * pDialStr );
EXTERN CHAR*      psaCC_ctbClrAdr2Num    ( SHORT cId, CHAR * pNumBuf, UBYTE maxSize );
EXTERN CHAR*      psaCC_ctbClrAdr2Sub    ( SHORT cId, CHAR * pSubBuf );
EXTERN CHAR*      psaCC_ctbCldAdr2Num    ( SHORT cId, CHAR * pNumBuf, UBYTE maxSize );
EXTERN CHAR*      psaCC_ctbCldAdr2Sub    ( SHORT cId, CHAR * pSubBuf );
EXTERN CHAR*      psaCC_ctbRdrAdr2Num    ( SHORT cId, CHAR * pNumBuf, UBYTE maxSize );
EXTERN CHAR*      psaCC_ctbRdrAdr2Sub    ( SHORT cId, CHAR * pSubBuf );
EXTERN T_ACI_PB_TEXT* psaCC_ctbGetAlpha  ( SHORT cId );
#ifdef TRACING
/*
EXTERN void       psaCC_ctbDump          ( void );
EXTERN void       psaCC_ctbDumpBC        ( void );
*/
EXTERN void       psaCC_shrPrmDump       ( void );
#endif /* TRACING */
EXTERN void       psaCC_chngCalTypCnt    ( SHORT cId, SHORT dlt );
EXTERN void       psaCC_chkPrgDesc       ( SHORT cId, UBYTE prgDesc,
                                           UBYTE msgType );
EXTERN SHORT      psaCC_getMOCTi         ( SHORT cId );
EXTERN void       psaCC_retMOCTi         ( UBYTE ti );
EXTERN void       psaCC_setSpeechMode    ( void );
EXTERN BOOL       psaCC_phbSrchNumPlnTxt ( CHAR           * inNum,
                                           UBYTE          * inoutMaxLen,
                                           T_ACI_PB_TEXT  * outTxt );
EXTERN BOOL       psaCC_phbMfwSrchNumPlnTxt ( CHAR          * inNum,
                                              T_ACI_PB_TEXT * outTxt );
EXTERN void       psaCC_phbSrchNum       ( SHORT cId, T_CC_CLTP call_type );
EXTERN BOOL       psaCC_phbSrchName      ( T_ACI_CMD_SRC  srcId,
                                           T_ACI_PB_TEXT  *srchName,
                                           T_CLPTY_PRM    *calPrm );
EXTERN UBYTE      psaCC_phbSrchECC       ( CHAR* dialStr, BOOL srchECC );
EXTERN BOOL       psaCC_phbNtryFnd       ( UBYTE phb,
                                           T_CLPTY_PRM* calPrm );
EXTERN void       psaCC_phbAddNtry       ( UBYTE phb, SHORT cId,
                                           UBYTE clTp, T_CLPTY_PRM *cldPty );
EXTERN void       psaCC_asmBuildMPTY     ( void );
EXTERN void       psaCC_asmHoldMPTY      ( void );
EXTERN void       psaCC_asmRetrieveMPTY  ( void );
EXTERN void       psaCC_asmSplitMPTY     ( void );
EXTERN void       psaCC_asmECT           ( void );
EXTERN void       psaCC_asmCUGInfo       ( SHORT cId );
EXTERN void       psaCC_asmCDReq         ( const CHAR      *number,
                                           const T_ACI_TOA *type,
                                           const CHAR      *subaddr,
                                           const T_ACI_TOS *satype);
EXTERN void       psaCC_asmComponent     ( SHORT cId );
EXTERN void       psaCC_asmCCBSReq       ( SHORT cId );
EXTERN void       psaCC_dasmInvokeCmp    ( SHORT cId, T_inv_comp *invCmp );
EXTERN void       psaCC_dasmResultCmp    ( SHORT cId, T_res_comp *resCmp );
EXTERN void       psaCC_dasmErrorCmp     ( SHORT cId, T_err_comp *errCmp );
EXTERN void       psaCC_dasmRejectCmp    ( SHORT cId, T_rej_comp *rejCmp );
EXTERN void       psaCC_ProcessCmp       ( T_MNCC_FACILITY_IND *mncc_facility_ind );
EXTERN void       psaCC_InitCtbNtry      ( SHORT idx );
EXTERN void       psaCC_FreeRdrPty       ( SHORT idx );
EXTERN void       psaCC_FreeCtbNtry      ( SHORT idx );
EXTERN BOOL       psaCC_ctbIsValid       ( SHORT cId );
EXTERN T_CC_CALL_TBL * psaCC_ctb         ( SHORT cId );
EXTERN void       psaCC_DumpFIE          ( T_MNCC_fac_inf *fie );
EXTERN void       psaCC_DTMFSent         ( SHORT cId );
EXTERN void       psaCC_StopDTMF         ( SHORT cId );
EXTERN SHORT      psaCC_BCapCode         ( UBYTE reqId,
                                           SHORT cId );

EXTERN SHORT      psaCC_BCapDecode       ( UBYTE reqId, 
                                           UINT16 bcLen1,
                                           UBYTE *bc1,
                                           UINT16 bcLen2,
                                           UBYTE *bc2);
EXTERN SHORT      psaCC_MPTY             ( SHORT cId,
                                           UBYTE mpty_event );

EXTERN void       psaCC_DTMFTimeout      ( void );

/*==== EXPORT =====================================================*/

#ifdef PSA_CCF_C

GLOBAL T_CC_SHRD_PRM ccShrdPrm;

#else

EXTERN T_CC_SHRD_PRM ccShrdPrm;

#endif /* PSA_CCF_C */


#endif /* PSA_CC_H */
/*==== EOF =======================================================*/

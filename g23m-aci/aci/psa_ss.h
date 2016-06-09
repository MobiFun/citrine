/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SS
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
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_SS_H
#define PSA_SS_H

/*==== CONSTANTS ==================================================*/
#define DEF_SS_VER     (0)        /* default SS version */

#define MAX_SS_NR      (4)        /* maximum number of calls */
#define TEST_STR_LEN  (80)        /* maximum length of test parameter string */

#define NO_ENTRY      (-1)        /* not a valid entry */
#define SS_NO_PRM     (0xFF)      /* parameter not present */
                                  
typedef enum                      /* service status */
{                                 
  NO_VLD_SSS = 0,                 /* not a valid service status */
  SSS_IDL,                         /* service idle */
  SSS_ACT,                         /* service active */
  SSS_PWD_REQ,                     /* service password request */
  SSS_PWD_SND,                     /* service password sent */
  SSS_PWD_VRF                      /* service password verified */
} T_SS_CLST;                      
                                  
typedef enum                      
{                                 
  NO_VLD_ST = 0,                  /* not a valid service type */
  ST_MOS,                         /* mobile originated service */
  ST_MTS                          /* mobile terminated service */
} T_SS_SSTP;                      
                                  
typedef enum                      
{                                 
  NO_VLD_SSF = 0,                 /* not a valid SS failure type */
  SSF_SS_ERR,                     /* SS error component */
  SSF_GEN_PRB,                    /* SS reject comp. general problem */
  SSF_INV_PRB,                    /* SS reject comp. invoke problem */
  SSF_ERR_PRB,                    /* SS reject comp. return error problem */
  SSF_RSL_PRB,                    /* SS reject comp. return result problem */
  SSF_CCD_DEC,                    /* CCD decode error */
  SSF_SS_ENT                      /* SS entity error */
} T_SS_FAIL;                      
                                  
typedef enum                      /* PSA notification events */
{
  SS_NTF_NEW_TRN = 0,             /* new SS transaction started */
  SS_NTF_END_TRN,                 /* existing SS transaction stopped */
  SS_NTF_CNT_TRN,                 /* existing transaction continued */
  SS_NTF_TRN_FAIL,                /* SS transaction failed */
  SS_NTF_TST,                     /* for test purposes only */
  SS_NTF_MAX                      /* maximum SS notification event */

} T_SS_NTF;

typedef enum
{
  CT_INV      = 0xA1,     /* invoke component type */
  CT_RET_RSLT,            /* return result component type */
  CT_RET_ERR,             /* return error component type */
  CT_RET_REJ,             /* reject component type */

  CT_MAX

} T_CMP_TYPE;

typedef enum
{
  BS_BEAR_NONE        = 0xFF, /* no valid bearer service */
  BS_BEAR_SRV         = 0x82, /* bearer service type */
  BS_TELE_SRV,                /* teleservice type */

  BS_TP_MAX

} T_BS_TYPE;


/*==== TYPES ======================================================*/
typedef struct SSServiceTabl
{
  BOOL              ntryUsdFlg;           /* flags this entry as used */
  UBYTE             ti;                   /* transaction identifier */
  UBYTE             iId;                  /* invoke id */
  UBYTE             srvStat;              /* service status */
  UBYTE             srvType;              /* type of service */
  UBYTE             SSver;                /* supplementary service version */
  UBYTE             orgOPC;               /* originated SS operation code */
  UBYTE             opCode;               /* SS operation code */
  UBYTE             ssCode;               /* SS service code */
  USHORT            ClassType;            /* holds class queried by user */
  UBYTE             ussdReqFlg;           /* flags USSD request */
  UBYTE             ussd_operation;       /* flags an on-going USSD operation */
  UBYTE             failType;             /* type of SS failure */
  USHORT            entCs;                /* entity failure cause */
  UBYTE             rejPrb;               /* reject problem code */
  UBYTE             errCd;                /* return error code */
  UBYTE             errPrms;               /* Parameters associated with the Error */
  T_ACI_AT_CMD      curCmd;               /* current command executing */
  T_OWN             srvOwn;               /* owner of service */
  T_MNSS_BEGIN_REQ *save_prim;            /* address where pending trans is memorized */
} T_SS_SRV_TBL;


typedef struct SSShrdParm
{
  T_SS_SRV_TBL stb[MAX_SS_NR];      /* service table for max. nr of services */
  UBYTE iIdNxt;                     /* next available invoke id */
  SHORT nrOfMOS;                    /* number of current MOS's */
  SHORT nrOfMTS;                    /* number of current MTS's */
  UBYTE cmpType;                    /* component type */
  UBYTE ussdLen;                    /* length of ussd string */
  UBYTE ussdDcs;                    /* DCS of ussd string */
  UBYTE ussdBuf[MAX_USSD_STRING];   /* buffers ussd string */
  USHORT mltyTrnFlg;                 /* holds id flag of pending transactions */
} T_SS_SHRD_PRM;

/*==== PROTOTYPES =================================================*/

void  psaSS_Init ( void );

EXTERN SHORT  psaSS_NewTrns  ( SHORT sId );
EXTERN SHORT  psaSS_EndTrns  ( SHORT sId );
EXTERN SHORT  psaSS_CntTrns  ( SHORT sId );

EXTERN void  psaSS_asmEmptyRslt     ( void );
EXTERN void  psaSS_asmErrorRslt     ( SHORT sId, UBYTE err );
EXTERN void  psaSS_asmInterrogateSS ( UBYTE ssCode, UBYTE bscSrvType, 
                                      UBYTE bscSrv);
EXTERN void  psaSS_asmRegisterSS    ( UBYTE ssCode, UBYTE bscSrvType, 
                                      UBYTE bscSrv, UBYTE ton, UBYTE npi, 
                                      UBYTE *num, UBYTE tos, UBYTE oe,
                                      UBYTE *sub, UBYTE time );
EXTERN void  psaSS_asmEraseSS       ( UBYTE ssCode, UBYTE bscSrvType, 
                                      UBYTE bscSrv );
EXTERN void  psaSS_asmActivateSS    ( UBYTE ssCode, UBYTE bscSrvType, 
                                      UBYTE bscSrv );
EXTERN void  psaSS_asmDeactivateSS  ( UBYTE ssCode, UBYTE bscSrvType, 
                                      UBYTE bscSrv );
EXTERN void  psaSS_asmRegisterPWD   ( UBYTE ssCode );
EXTERN void  psaSS_asmVerifyPWD     ( UBYTE *pwd );
EXTERN void  psaSS_asmProcUSSDReq   ( UBYTE dcs, UBYTE *ussd, UBYTE len );
EXTERN void  psaSS_asmCnfUSSDReq    ( UBYTE dcs, UBYTE *ussd, UBYTE len );
EXTERN void  psaSS_asmCnfUSSDNtfy   ( void );
EXTERN BOOL  psaSS_asmUSSDProt1     ( SHORT sId );
#if 0  /* For further study, so not yet used */
EXTERN void  psaSS_asmActivateCCBS  ( void );
#endif
EXTERN void  psaSS_asmDeactivateCCBS( UBYTE idx );
EXTERN SHORT psaSS_stbNewEntry      ( void );
EXTERN SHORT psaSS_stbFindTi        ( UBYTE ti2Find );
EXTERN SHORT psaSS_stbFindInvId     ( UBYTE invId2Find );
EXTERN void  psaSS_stbDump          ( void );
EXTERN void  psaSS_chngSrvTypCnt    ( SHORT sId, SHORT dlt );
EXTERN SHORT psaSS_getMOSTi         ( SHORT sId );
EXTERN void  psaSS_retMOSTi         ( SHORT sId );
EXTERN SHORT psaSS_stbFindUssdReq   ( void );
EXTERN SHORT psaSS_stbFindActSrv    ( SHORT sId );
EXTERN void  psaSS_InitStbNtry      ( SHORT idx );
EXTERN void  psaSS_DumpFIE          ( T_fac_inf *fie );
EXTERN void  psaSS_dasmInvokeCmp    ( SHORT sId,
                                      T_inv_comp *invCmp );
EXTERN void  psaSS_dasmResultCmp    ( SHORT sId, 
                                      T_res_comp *resCmp );
EXTERN T_ACI_RETURN  psaSS_dasmErrorCmp     ( SHORT sId,
                                              T_err_comp *errCmp, 
                                              BOOL is_fac_ussd  );
EXTERN T_ACI_RETURN  psaSS_dasmRejectCmp    ( SHORT sId, 
                                              T_rej_comp *rejCmp, 
                                              BOOL is_fac_ussd  );
EXTERN T_ACI_RETURN  psaSS_ss_end_ind       ( SHORT sId, 
                                              T_COMPONENT *comp, 
                                              BOOL is_fac_ussd );
EXTERN SHORT psaSS_GetPendingTrn    ( void );
EXTERN void  psaSS_KillAllPendingTrn( void );


/*==== EXPORT =====================================================*/

#ifdef PSA_SSF_C

GLOBAL T_SS_SHRD_PRM ssShrdPrm;

#else

EXTERN T_SS_SHRD_PRM ssShrdPrm;

#endif /* PSA_SSF_C */

 
#endif /* PSA_SS_H */

/*==== EOF =======================================================*/

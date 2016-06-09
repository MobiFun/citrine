/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_FD
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

#ifndef ACI_FD_H
#define ACI_FD_H

/*==== DEFINES ====================================================*/
#define MAX_ID_CHAR  (20+1) /* maximum length of loc ID in chars */
#define MAX_NSF_BYTE (90)   /* maximum length of NSF in bytes */

/*==== TYPES ======================================================*/

#ifdef FF_FAX

/*--- +FCLASS -----------------------------------------------------*/
typedef enum
{
  FCLASS_CLASS_NotPresent = -1,
  FCLASS_CLASS_Data,
  FCLASS_CLASS_Fax2     = 2,
  FCLASS_CLASS_Voice    = 8,
  FCLASS_CLASS_Fax20    = 20
}
T_ACI_FCLASS_CLASS;

/*--- +FET --------------------------------------------------------*/
typedef enum
{
  FET_PPM_NotPresent  = -1,
  FET_PPM_Mps,
  FET_PPM_Eom,
  FET_PPM_Eop,
  FET_PPM_PriMps,
  FET_PPM_PriEom,
  FET_PPM_PriEop
}
T_ACI_FET_PPM;

/*--- +FHS --------------------------------------------------------*/
typedef enum
{
  FHS_STAT_NotPresent     = -1,
  FHS_STAT_NormEnd,
  FHS_STAT_RingDetect,
  FHS_STAT_CallAbort,
  FHS_STAT_NoLoopCurr,
  FHS_STAT_RingbackNoAns,
  FHS_STAT_RingbackAns,
  FHS_STAT_UnspcPhAErr    = 16,
  FHS_STAT_NoAns,
  FHS_STAT_UnspcTxPhBErr  = 32,
  FHS_STAT_NoRemoteRxTx,
  FHS_STAT_ComrecErrTx,
  FHS_STAT_InvComrecCmd,
  FHS_STAT_RsprecErrTx,
  FHS_STAT_DcsNoRes,
  FHS_STAT_DisDtcRcvd,
  FHS_STAT_TrainFail,
  FHS_STAT_InvRsprecRes,
  FHS_STAT_UnspcTxPhCErr  = 64,
  FHS_STAT_UnspcImgFrmtErr,
  FHS_STAT_ImgCnvErr,
  FHS_STAT_DteDceDataUndrflw,
  FHS_STAT_UnrecDataCmd,
  FHS_STAT_ImgErrLineLen,
  FHS_STAT_ImgErrPageLen,
  FHS_STAT_ImgErrCmprCode,
  FHS_STAT_UnspcTxPhDErr  = 80,
  FHS_STAT_RsprecErrD,
  FHS_STAT_NoMpsRes,
  FHS_STAT_InvMpsRes,
  FHS_STAT_NoEopRes,
  FHS_STAT_InvEopRes,
  FHS_STAT_NoEomRes,
  FHS_STAT_InvEomRes,
  FHS_STAT_50PinPip,
  FHS_STAT_UnspcRxPhBErr  = 112,
  FHS_STAT_RsprecErrRx,
  FHS_STAT_ComrecErrRx,
  FHS_STAT_T30T2Timeout,
  FHS_STAT_T30T1Timeout,
  FHS_STAT_UnspcRxPhCErr  = 144,
  FHS_STAT_MissEol,
  FHS_STAT_BadCRCFrm,
  FHS_STAT_DceDteBufOvrflw,
  FHS_STAT_UnspcRxPhDErr  = 160,
  FHS_STAT_RsprecInvRes,
  FHS_STAT_ComrecInvRes,
  FHS_STAT_A0PinPip,
  FHS_STAT_PhBSndRsrvd1   = 224,
  FHS_STAT_PhBSndRsrvd2,
  FHS_STAT_PhBSndRsrvd3,
  FHS_STAT_PhCSndRsrvd1,
  FHS_STAT_PhCSndRsrvd2,
  FHS_STAT_PhDSndRsrvd1,
  FHS_STAT_PhDSndRsrvd2,
  FHS_STAT_PhDSndRsrvd3,
  FHS_STAT_PhBRcvNoResp,
  FHS_STAT_PhBRcvInvResp,
  FHS_STAT_PhBRcvRsrvd3,
  FHS_STAT_PhCRcvRsrvd1,
  FHS_STAT_PhCRcvRsrvd2,
  FHS_STAT_PhDRcvNoResp,
  FHS_STAT_PhDRcvInvResp,
  FHS_STAT_PhDRcvRsrvd3,
  FHS_STAT_SgnNotAllwd,
  FHS_STAT_FADRmtStnErr,
  FHS_STAT_FADLclStnErr,
  FHS_STAT_FADOwnErr,
  FHS_STAT_FADGnrlErr
}
T_ACI_FHS_STAT;

/*--- +FPS --------------------------------------------------------*/
typedef enum
{
  FPS_PPR_NotPresent  = -1,
  FPS_PPR_Mcf         =  1,
  FPS_PPR_Rtn,
  FPS_PPR_Rtp,
  FPS_PPR_Pin,
  FPS_PPR_Pip
}
T_ACI_FPS_PPR;

/*--- +FCR --------------------------------------------------------*/
typedef enum
{
  FCR_VAL_NotPresent  = -1,
  FCR_VAL_NoRcvCap,
  FCR_VAL_RcvCap
}
T_ACI_FCR_VAL;

/*--- +FIS +FCC +FCR ----------------------------------------------*/
typedef enum
{
  F_VR_NotPresent     = -1,
  F_VR_R8X3_85,
  F_VR_R8X7_7,
  F_VR_R8X15_4,
  F_VR_R16X15_4       = 4,
  F_VR_200X100        = 8,
  F_VR_200X200        = 16,
  F_VR_200X400        = 32,
  F_VR_300X300        = 64
}
T_ACI_F_VR;

typedef enum
{
  F_BR_NotPresent = -1,
  F_BR_2400,
  F_BR_4800,
  F_BR_7200,
  F_BR_9600,
  F_BR_12000,
  F_BR_14400
}
T_ACI_F_BR;

typedef enum
{
  F_WD_NotPresent = -1,
  F_WD_1728,
  F_WD_2048,
  F_WD_2432,
  F_WD_1216,
  F_WD_864
}
T_ACI_F_WD;

typedef enum
{
  F_LN_NotPresent = -1,
  F_LN_A4,
  F_LN_B4,
  F_LN_Unlimited
}
T_ACI_F_LN;

typedef enum
{
  F_DF_NotPresent = -1,
  F_DF_1D_MdfHuff,
  F_DF_2D_MdfRd_T4,
  F_DF_2D_Uncomp,
  F_DF_2D_MdfRd_T6
}
T_ACI_F_DF;

typedef enum
{
  F_EC_NotPresent = -1,
  F_EC_DisableECM,
  F_EC_EnableECM,
  F_EC_EnableHalfDup,
  F_EC_EnableFullDup
}
T_ACI_F_EC;

typedef enum
{
  F_BF_NotPresent     = -1,
  F_BF_DisableFileTrnsf,
  F_BF_EnableBFT,
  F_BF_DocuTrnsfMode,
  F_BF_EdifactMode    = 4,
  F_BF_BasicTrnsfMode = 8,
  F_BF_CharMode       = 16,
  F_BF_MixMode        = 32,
  F_BF_ProcMode       = 64
}
T_ACI_F_BF;

typedef enum
{
  F_ST_NotPresent = -1,
  F_ST_0_0,
  F_ST_5_5,
  F_ST_10_5,
  F_ST_10_10,
  F_ST_20_10,
  F_ST_20_20,
  F_ST_40_20,
  F_ST_40_40
}
T_ACI_F_ST;

typedef enum
{
  F_JP_NotPresent     = -1,
  F_JP_DisableJPEG,
  F_JP_EnableJPEG,
  F_JP_FullColor,
  F_JP_EnablePrefHuff = 4,
  F_JP_12BitsPelComp  = 8,
  F_JP_NoSubsmpl      = 16,
  F_JP_CustIllum      = 32,
  F_JP_CustGamutRange = 64
}
T_ACI_F_JP;

/*--- +FLP --------------------------------------------------------*/
typedef enum
{
  FLP_VAL_NotPresent  = -1,
  FLP_VAL_NoPollDoc,
  FLP_VAL_PollDoc
}
T_ACI_FLP_VAL;

/*--- +FAP --------------------------------------------------------*/
typedef enum
{
  FAP_VAL_NotPresent  = -1,
  FAP_VAL_Disabled,
  FAP_VAL_Enabled
}
T_ACI_FAP_VAL;

/*--- +FSP --------------------------------------------------------*/
typedef enum
{
  FSP_VAL_NotPresent = -1,
  FSP_VAL_PollDisabled,
  FSP_VAL_PollEnable
}
T_ACI_FSP_VAL;

/*--- +FIE --------------------------------------------------------*/
typedef enum
{
  FIE_VAL_NotPresent = -1,
  FIE_VAL_IgnorePRI,
  FIE_VAL_AcceptPRI
}
T_ACI_FIE_VAL;

/*--- +FCQ --------------------------------------------------------*/
typedef enum
{
  FCQ_RQ_NotPresent = -1,
  FCQ_RQ_CQCDisabled,
  FCQ_RQ_CQCEnabled,
  FCQ_RQ_CQCandCorrection
}
T_ACI_FCQ_RQ;

typedef enum
{
  FCQ_TQ_NotPresent = -1,
  FCQ_TQ_CQCDisabled,
  FCQ_TQ_CQCEnabled,
  FCQ_TQ_CQCandCorrection
}
T_ACI_FCQ_TQ;

/*--- +FND --------------------------------------------------------*/
typedef enum
{
  FND_VAL_NotPresent = -1,
  FND_VAL_MsgTypeDCS,
  FND_VAL_NonStandard
}
T_ACI_FND_VAL;

/*--- +FFC --------------------------------------------------------*/
typedef enum
{
  FFC_VRC_NotPresent = -1,
  FFC_VRC_Ignored,
  FFC_VRC_Enabled,
  FFC_VRC_Conversion1D,
  FFC_VRC_Conversion2D
}
T_ACI_FFC_VRC;

typedef enum
{
  FFC_DFC_NotPresent = -1,
  FFC_DFC_Ignored,
  FFC_DFC_Enabled,
  FFC_DFC_Conversion
}
T_ACI_FFC_DFC;

typedef enum
{
  FFC_LNC_NotPresent = -1,
  FFC_LNC_Ignored,
  FFC_LNC_Enabled,
  FFC_LNC_Conversion1D,
  FFC_LNC_Conversion2D
}
T_ACI_FFC_LNC;

typedef enum
{
  FFC_WDC_NotPresent = -1,
  FFC_WDC_Ignored,
  FFC_WDC_Enabled,
  FFC_WDC_Conversion
}
T_ACI_FFC_WDC;

/*--- +FIT --------------------------------------------------------*/
typedef enum
{
  FIT_ACT_NotPresent = -1,
  FIT_ACT_OnHookRst,
  FIT_ACT_OnHook
}
T_ACI_FIT_ACT;

/*--- +FBO --------------------------------------------------------*/
typedef enum
{
  FBO_VAL_NotPresent = -1,
  FBO_VAL_DirCDirBD,
  FBO_VAL_RvrCDirBD,
  FBO_VAL_DirCRvrBD,
  FBO_VAL_RvrCRvrBD
}
T_ACI_FBO_VAL;

#endif /* FF_FAX */

/*---- Structs ----------------------------------------------------*/

typedef struct
{
  struct
  {
    UBYTE rpr_stat;
    UBYTE tpr_stat;
    UBYTE idr_stat;
    UBYTE nsr_stat;
  } FNR;
  UBYTE FEA_stat;
  UBYTE FBU_stat;
} FDCmdVars;

/*---- Prototypes -------------------------------------------------*/

#ifdef FF_FAX

EXTERN T_ACI_RETURN sAT_PlusFDT   (T_ACI_CMD_SRC        srcId);
EXTERN T_ACI_RETURN sAT_PlusFDR   (T_ACI_CMD_SRC        srcId);
EXTERN T_ACI_RETURN sAT_PlusFIP   (T_ACI_CMD_SRC        srcId);
EXTERN T_ACI_RETURN sAT_PlusFKS   (T_ACI_CMD_SRC        srcId);
EXTERN T_ACI_RETURN sAT_PlusFCR   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_FCR_VAL        value);
EXTERN T_ACI_RETURN qAT_PlusFCR   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_FCR_VAL      * value);
EXTERN T_ACI_RETURN sAT_PlusFLI   (T_ACI_CMD_SRC        srcId,
                                   char               * idStr);
EXTERN T_ACI_RETURN qAT_PlusFLI   (T_ACI_CMD_SRC        srcId,
                                   char               * idStr);
EXTERN T_ACI_RETURN sAT_PlusFPI   (T_ACI_CMD_SRC        srcId,
                                   CHAR               * idStr );
EXTERN T_ACI_RETURN qAT_PlusFPI   (T_ACI_CMD_SRC        srcId,
                                   CHAR               * idStr );
EXTERN T_ACI_RETURN sAT_PlusFSA   (T_ACI_CMD_SRC        srcId,
                                   CHAR               * subStr );
EXTERN T_ACI_RETURN qAT_PlusFSA   (T_ACI_CMD_SRC        srcId,
                                   CHAR               * subStr );
EXTERN T_ACI_RETURN sAT_PlusFPA   (T_ACI_CMD_SRC        srcId,
                                   CHAR               * sepStr );
EXTERN T_ACI_RETURN qAT_PlusFPA   (T_ACI_CMD_SRC        srcId,
                                   CHAR               * sepStr );
EXTERN T_ACI_RETURN sAT_PlusFPW   (T_ACI_CMD_SRC        srcId,
                                   CHAR               * pwdStr );
EXTERN T_ACI_RETURN qAT_PlusFPW   (T_ACI_CMD_SRC        srcId,
                                   CHAR               * pwdStr );
EXTERN T_ACI_RETURN sAT_PlusFCC   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_F_VR           vr,
                                   T_ACI_F_BR           br,
                                   T_ACI_F_WD           wd,
                                   T_ACI_F_LN           ln,
                                   T_ACI_F_DF           df,
                                   T_ACI_F_EC           ec,
                                   T_ACI_F_BF           bf,
                                   T_ACI_F_ST           st,
                                   T_ACI_F_JP           jp );
EXTERN T_ACI_RETURN qAT_PlusFCC   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_F_VR         * vr,
                                   T_ACI_F_BR         * br,
                                   T_ACI_F_WD         * wd,
                                   T_ACI_F_LN         * ln,
                                   T_ACI_F_DF         * df,
                                   T_ACI_F_EC         * ec,
                                   T_ACI_F_BF         * bf,
                                   T_ACI_F_ST         * st,
                                   T_ACI_F_JP         * jp );
EXTERN T_ACI_RETURN sAT_PlusFIS   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_F_VR           vr,
                                   T_ACI_F_BR           br,
                                   T_ACI_F_WD           wd,
                                   T_ACI_F_LN           ln,
                                   T_ACI_F_DF           df,
                                   T_ACI_F_EC           ec,
                                   T_ACI_F_BF           bf,
                                   T_ACI_F_ST           st,
                                   T_ACI_F_JP           jp );
EXTERN T_ACI_RETURN qAT_PlusFIS   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_F_VR         * vr,
                                   T_ACI_F_BR         * br,
                                   T_ACI_F_WD         * wd,
                                   T_ACI_F_LN         * ln,
                                   T_ACI_F_DF         * df,
                                   T_ACI_F_EC         * ec,
                                   T_ACI_F_BF         * bf,
                                   T_ACI_F_ST         * st,
                                   T_ACI_F_JP         * jp );
EXTERN T_ACI_RETURN qAT_PlusFCS   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_F_VR         * vr,
                                   T_ACI_F_BR         * br, 
                                   T_ACI_F_WD         * wd,
                                   T_ACI_F_LN         * ln, 
                                   T_ACI_F_DF         * df,
                                   T_ACI_F_EC         * ec, 
                                   T_ACI_F_BF         * bf,
                                   T_ACI_F_ST         * st,
                                   T_ACI_F_JP         * jp);
EXTERN T_ACI_RETURN sAT_PlusFCLASS(T_ACI_CMD_SRC        srcId,
                                   T_ACI_FCLASS_CLASS   class_type);
EXTERN T_ACI_RETURN qAT_PlusFCLASS(T_ACI_CMD_SRC        srcId,
                                   T_ACI_FCLASS_CLASS * class_type);
EXTERN T_ACI_RETURN sAT_PlusFNS   (T_ACI_CMD_SRC        srcId,
                                   UBYTE                len,
                                   UBYTE              * nsf );
EXTERN T_ACI_RETURN qAT_PlusFNS   (T_ACI_CMD_SRC        srcId,
                                   UBYTE              * len,
                                   UBYTE              * nsf );
EXTERN T_ACI_RETURN sAT_PlusFLP   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FLP_VAL        value );
EXTERN T_ACI_RETURN qAT_PlusFLP   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FLP_VAL      * value );
EXTERN T_ACI_RETURN sAT_PlusFSP   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FSP_VAL        value );
EXTERN T_ACI_RETURN qAT_PlusFSP   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FSP_VAL      * value );
EXTERN T_ACI_RETURN sAT_PlusFCR   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FCR_VAL        value );
EXTERN T_ACI_RETURN qAT_PlusFCR   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FCR_VAL      * value );
EXTERN T_ACI_RETURN sAT_PlusFAP   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FAP_VAL        sub,
                                   T_ACI_FAP_VAL        sep,
                                   T_ACI_FAP_VAL        pwd );
EXTERN T_ACI_RETURN qAT_PlusFAP   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FAP_VAL      * sub,
                                   T_ACI_FAP_VAL      * sep,
                                   T_ACI_FAP_VAL      * pwd );
EXTERN T_ACI_RETURN sAT_PlusFIE   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FIE_VAL        value );
EXTERN T_ACI_RETURN qAT_PlusFIE   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FIE_VAL      * value );
EXTERN T_ACI_RETURN sAT_PlusFPS   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FPS_PPR        ppr );
EXTERN T_ACI_RETURN qAT_PlusFPS   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FPS_PPR      * ppr );
EXTERN T_ACI_RETURN sAT_PlusFCQ   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_FCQ_RQ         rq,
                                   T_ACI_FCQ_TQ         tq );
EXTERN T_ACI_RETURN qAT_PlusFCQ   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_FCQ_RQ       * rq,
                                   T_ACI_FCQ_TQ       * tq );
EXTERN T_ACI_RETURN sAT_PlusFRQ   (T_ACI_CMD_SRC        srcId, 
                                   SHORT                pgl, 
                                   SHORT                cbl );
EXTERN T_ACI_RETURN qAT_PlusFRQ   (T_ACI_CMD_SRC        srcId, 
                                   SHORT              * pgl, 
                                   SHORT              * cbl );
EXTERN T_ACI_RETURN qAT_PlusFHS   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FHS_STAT     * status );
EXTERN T_ACI_RETURN sAT_PlusFMS   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_F_BR           br);
EXTERN T_ACI_RETURN qAT_PlusFMS   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_F_BR         * br);
EXTERN T_ACI_RETURN sAT_PlusFND   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FND_VAL        value);
EXTERN T_ACI_RETURN qAT_PlusFND   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FND_VAL      * value);
EXTERN T_ACI_RETURN sAT_PlusFFC   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FFC_VRC        vrc,
                                   T_ACI_FFC_DFC        dfc,
                                   T_ACI_FFC_LNC        lnc,
                                   T_ACI_FFC_WDC        wdc);
EXTERN T_ACI_RETURN qAT_PlusFFC   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FFC_VRC      * vrc,
                                   T_ACI_FFC_DFC      * dfc,
                                   T_ACI_FFC_LNC      * lnc,
                                   T_ACI_FFC_WDC      * wdc);
EXTERN T_ACI_RETURN sAT_PlusFEA   (T_ACI_CMD_SRC        srcId, 
                                   SHORT                value );
EXTERN T_ACI_RETURN qAT_PlusFEA   (T_ACI_CMD_SRC        srcId, 
                                   SHORT              * value );
EXTERN T_ACI_RETURN sAT_PlusFCT   (T_ACI_CMD_SRC        srcId, 
                                   SHORT                value );
EXTERN T_ACI_RETURN qAT_PlusFCT   (T_ACI_CMD_SRC        srcId, 
                                   SHORT              * value );
EXTERN T_ACI_RETURN sAT_PlusFIT   (T_ACI_CMD_SRC        srcId, 
                                   SHORT                time,
                                   T_ACI_FIT_ACT        act );
EXTERN T_ACI_RETURN qAT_PlusFIT   (T_ACI_CMD_SRC        srcId, 
                                   SHORT              * time,
                                   T_ACI_FIT_ACT      * act );
EXTERN T_ACI_RETURN qAT_PlusFBS   (T_ACI_CMD_SRC        srcId, 
                                   SHORT              * tbs,
                                   SHORT              * rbs );
EXTERN T_ACI_RETURN sAT_PlusFBO   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FBO_VAL        value );
EXTERN T_ACI_RETURN qAT_PlusFBO   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FBO_VAL      * value );
#endif /* FF_FAX */


/*---- Call Back Prototypes for MMI ------------------------------*/

#ifdef CMH_F_C /*lint -save -e18 */ /* Turn off Lint errors for this "construct" */

#ifdef FF_FAX

EXTERN void rAT_PlusFHT   (void);
EXTERN void rAT_PlusFHR   (void);
EXTERN void rAT_PlusFCI   (void);
EXTERN void rAT_PlusFIS   (void);
EXTERN void rAT_PlusFCS   (void);
EXTERN void rAT_PlusFTC   (void);
EXTERN void rAT_PlusFHS   (void);
EXTERN void rAT_PlusFCO   (void);
EXTERN void rAT_PlusFSA   (void);
EXTERN void rAT_PlusFPA   (void);
EXTERN void rAT_PlusFPW   (void);
EXTERN void rAT_PlusFET   (void);
EXTERN void rAT_PlusFVO   (void);
EXTERN void rAT_PlusFPO   (void);
EXTERN void rAT_PlusFTI   (void);
EXTERN void rAT_PlusFCI   (void);
EXTERN void rAT_PlusFPI   (void);
EXTERN void rAT_PlusFNF   (void);
EXTERN void rAT_PlusFNS   (void);
EXTERN void rAT_PlusFNC   (void);
EXTERN void rAT_PlusFPS   (void);

#endif /* FF_FAX */

/*lint -restore */
#else

#ifdef FF_FAX

EXTERN void rAT_PlusFHT   (USHORT           len, 
                           UBYTE          * hdlc);
EXTERN void rAT_PlusFHR   (USHORT           len, 
                           UBYTE          * hdlc);
EXTERN void rAT_PlusFCI   (CHAR           * rmtId);

EXTERN void rAT_PlusFIS   (T_ACI_F_VR       vr,
                           T_ACI_F_BR       br,
                           T_ACI_F_WD       wd,
                           T_ACI_F_LN       ln,
                           T_ACI_F_DF       df,
                           T_ACI_F_EC       ec,
                           T_ACI_F_BF       bf,
                           T_ACI_F_ST       st,
                           T_ACI_F_JP       jp );
EXTERN void rAT_PlusFCS   (T_ACI_F_VR       vr,
                           T_ACI_F_BR       br,
                           T_ACI_F_WD       wd,
                           T_ACI_F_LN       ln,
                           T_ACI_F_DF       df,
                           T_ACI_F_EC       ec,
                           T_ACI_F_BF       bf,
                           T_ACI_F_ST       st,
                           T_ACI_F_JP       jp );
EXTERN void rAT_PlusFTC   (T_ACI_F_VR       vr,
                           T_ACI_F_BR       br,
                           T_ACI_F_WD       wd,
                           T_ACI_F_LN       ln,
                           T_ACI_F_DF       df,
                           T_ACI_F_EC       ec,
                           T_ACI_F_BF       bf,
                           T_ACI_F_ST       st,
                           T_ACI_F_JP       jp );
EXTERN void rAT_PlusFHS   (T_ACI_FHS_STAT   stat);
EXTERN void rAT_PlusFCO   (void);
EXTERN void rAT_PlusFSA   (CHAR           * sub);
EXTERN void rAT_PlusFPA   (CHAR           * sep);
EXTERN void rAT_PlusFPW   (CHAR           * pwd);
EXTERN void rAT_PlusFET   (T_ACI_FET_PPM    ppm);
EXTERN void rAT_PlusFVO   (void);
EXTERN void rAT_PlusFPO   (void);
EXTERN void rAT_PlusFTI   (CHAR           * tsi);
EXTERN void rAT_PlusFCI   (CHAR           * csi);
EXTERN void rAT_PlusFPI   (CHAR           * cig);
EXTERN void rAT_PlusFNF   (USHORT           len, 
                           UBYTE          * nsf);
EXTERN void rAT_PlusFNS   (USHORT           len, 
                           UBYTE          * nss);
EXTERN void rAT_PlusFNC   (USHORT           len,
                           UBYTE          * nsc);
EXTERN void rAT_PlusFPS   (T_ACI_FPS_PPR    ppr, 
                           SHORT            lc, 
                           SHORT            blc, 
                           SHORT            cblc, 
                           SHORT            lbc);
#endif /* FF_FAX */

#endif                          

/*--------------- call-backs for AT CI ----------------------------*/

#ifdef CMH_F_C /*lint -save -e18 */ /* Turn off Lint errors for this "construct" */

#ifdef FF_FAX

EXTERN void rCI_PlusFHT   (void);
EXTERN void rCI_PlusFHR   (void);
EXTERN void rCI_PlusFCI   (void);
EXTERN void rCI_PlusFIS   (void);
EXTERN void rCI_PlusFCS   (void);
EXTERN void rCI_PlusFTC   (void);
EXTERN void rCI_PlusFHS   (void);
EXTERN void rCI_PlusFCO   (void);
EXTERN void rCI_PlusFSA   (void);
EXTERN void rCI_PlusFPA   (void);
EXTERN void rCI_PlusFPW   (void);
EXTERN void rCI_PlusFET   (void);
EXTERN void rCI_PlusFVO   (void);
EXTERN void rCI_PlusFPO   (void);
EXTERN void rCI_PlusFTI   (void);
EXTERN void rCI_PlusFCI   (void);
EXTERN void rCI_PlusFPI   (void);
EXTERN void rCI_PlusFNF   (void);
EXTERN void rCI_PlusFNS   (void);
EXTERN void rCI_PlusFNC   (void);
EXTERN void rCI_PlusFPS   (void);

#endif /* FF_FAX */

/*lint -restore */
#else

#ifdef FF_FAX

EXTERN void rCI_PlusFHT   (USHORT           len, 
                           UBYTE          * hdlc);
EXTERN void rCI_PlusFHR   (USHORT           len, 
                           UBYTE          * hdlc);
EXTERN void rCI_PlusFCI   (CHAR           * rmtId);
EXTERN void rCI_PlusFIS   (T_ACI_F_VR       vr,
                           T_ACI_F_BR       br,
                           T_ACI_F_WD       wd,
                           T_ACI_F_LN       ln,
                           T_ACI_F_DF       df,
                           T_ACI_F_EC       ec,
                           T_ACI_F_BF       bf,
                           T_ACI_F_ST       st,
                           T_ACI_F_JP       jp );
EXTERN void rCI_PlusFCS   (T_ACI_F_VR       vr,
                           T_ACI_F_BR       br,
                           T_ACI_F_WD       wd,
                           T_ACI_F_LN       ln,
                           T_ACI_F_DF       df,
                           T_ACI_F_EC       ec,
                           T_ACI_F_BF       bf,
                           T_ACI_F_ST       st,
                           T_ACI_F_JP       jp );
EXTERN void rCI_PlusFTC   (T_ACI_F_VR       vr,
                           T_ACI_F_BR       br,
                           T_ACI_F_WD       wd,
                           T_ACI_F_LN       ln,
                           T_ACI_F_DF       df,
                           T_ACI_F_EC       ec,
                           T_ACI_F_BF       bf,
                           T_ACI_F_ST       st,
                           T_ACI_F_JP       jp );
EXTERN void rCI_PlusFHS   (T_ACI_FHS_STAT   stat);
EXTERN void rCI_PlusFCO   (void);
EXTERN void rCI_PlusFSA   (CHAR           * sub);
EXTERN void rCI_PlusFPA   (CHAR           * sep);
EXTERN void rCI_PlusFPW   (CHAR           * pwd);
EXTERN void rCI_PlusFET   (T_ACI_FET_PPM    ppm);
EXTERN void rCI_PlusFVO   (void);
EXTERN void rCI_PlusFPO   (void);
EXTERN void rCI_PlusFTI   (CHAR           * tsi);
EXTERN void rCI_PlusFCI   (CHAR           * csi);
EXTERN void rCI_PlusFPI   (CHAR           * cig);
EXTERN void rCI_PlusFNF   (USHORT           len, 
                           UBYTE          * nsf);
EXTERN void rCI_PlusFNS   (USHORT           len, 
                           UBYTE          * nss);
EXTERN void rCI_PlusFNC   (USHORT           len,
                           UBYTE          * nsc);
EXTERN void rCI_PlusFPS   (T_ACI_FPS_PPR    ppr, 
                           SHORT            lc, 
                           SHORT            blc, 
                           SHORT            cblc, 
                           SHORT            lbc);
#endif /* FF_FAX */

#endif

/*==== EOF ========================================================*/

#ifdef ACI_FD_CMD_C
GLOBAL FDCmdVars fd;
#else
EXTERN FDCmdVars fd;
#endif

#endif

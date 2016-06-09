/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SS
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
|  Purpose :  Definitions for the command handler of the 
|             Subscriber Identity Module ( SIM )
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_SS_H
#define CMH_SS_H

/*==== CONSTANTS ==================================================*/
#define MAX_CF_FEAT_NR  (5) /* maximum number of forwarding feature
                               list entries */
#define MAX_CB_INFO_NR  (5) /* maximum number of barring info list
                               entries */
#define MAX_CW_BSG_NR   (5) /* maximum number of basic service group
                               entries */
#define MAX_CC_FEAT_NR  (5) /* maximum number of call completion feature
                               entries */
#define CCFC_TIME_MAX   (30)/* +CCFC parameter <time> */
#define CCFC_TIME_MIN   (5) 

/*==== TYPES ======================================================*/

/* Structure for Index to String mapping, i.e. to each number in "idx" field corresponds "cod" string*/
typedef struct
{
  BYTE idx; 
  char* cod; 
} T_IDX_TO_STR_MAP;

/* Structure for Index to Digits mapping, i.e. to each number in "idx" field corresponds "cod" number*/
typedef struct
{
  SHORT idx; 
  SHORT cod; 
} T_IDX_TO_DIG_MAP;

typedef enum
{
  CUSDR_EXT_USSD_RES_Not_Pending = 0,
  CUSDR_EXT_USSD_RES_Request,
  CUSDR_EXT_USSD_RES_Notify
}
T_CUSDR_EXT_USSD_RES;

 
/*==== PROTOTYPES =================================================*/
/* Implements Measure # 85 */
EXTERN BOOL cmhSS_checkCCFC_RSN(T_ACI_CCFC_RSN reason, UBYTE *ssCd);
/* Implements Measure # 166 */
EXTERN BOOL cmhSS_getSSCd(T_ACI_FAC fac, UBYTE *ssCd);
EXTERN BOOL checkSSforFDN(char *cSSString);
EXTERN T_ACI_RETURN checkFDNSendSS(T_ACI_CMD_SRC srcId, CHAR *pSSString);

EXTERN char *mapIdxToString(const T_IDX_TO_STR_MAP *table, BYTE index);
EXTERN BYTE mapIdxToDigit(const T_IDX_TO_DIG_MAP *table, BYTE index, BYTE * result);

#ifdef SIM_TOOLKIT
  EXTERN T_ACI_RETURN cmhSS_SendToSAT(T_ACI_CMD_SRC srcId, CHAR *pSSString);
#endif /* SIM_TOOLKIT */

EXTERN T_ACI_RETURN cmhSS_CF_SAT_Handle(T_ACI_CMD_SRC srcId,
                                    T_ACI_CCFC_RSN reason,
                                    T_ACI_CCFC_MOD mode,
                                    CHAR*          number,
                                    T_ACI_TOA*     type,
                                    T_ACI_CLASS    class_type,
                                    CHAR*          subaddr,
                                    T_ACI_TOS*     satype,
                                    SHORT          time);
EXTERN T_ACI_RETURN cmhSS_CW_SAT_Handle(T_ACI_CMD_SRC srcId,
                                    T_ACI_CCFC_MOD mode,
                                    T_ACI_CLASS    class_type);

T_ACI_RETURN cmhSS_CLIP_SAT_Handle(T_ACI_CMD_SRC srcId);
T_ACI_RETURN cmhSS_CLIR_SAT_Handle(T_ACI_CMD_SRC srcId);
T_ACI_RETURN cmhSS_COLP_SAT_Handle(T_ACI_CMD_SRC srcId);
T_ACI_RETURN cmhSS_COLR_SAT_Handle(T_ACI_CMD_SRC srcId);
T_ACI_RETURN cmhSS_CCBS_SAT_Handle(T_ACI_CMD_SRC srcId, T_ACI_CCBS_MOD mode, SHORT idx);
T_ACI_RETURN cmhSS_CNAP_SAT_Handle(T_ACI_CMD_SRC srcId);
T_ACI_RETURN cmhSS_Call_Barr_SAT_Handle(T_ACI_CMD_SRC srcId, 
                        T_ACI_CLCK_MOD mode, 
                        T_ACI_FAC fac, 
                        CHAR  *passwd, 
                        T_ACI_CLASS    class_type);


EXTERN SHORT cmhSS_TransFail             ( SHORT sId );

EXTERN T_ACI_CLASS cmhSS_GetClass        ( T_basicService * bs );
EXTERN T_ACI_CLASS cmhSS_GetClassType    ( UBYTE bsTp, UBYTE bsCd );
EXTERN T_ACI_CLASS cmhSS_GetCbClassType  ( UBYTE bsTp, UBYTE bsCd );
EXTERN T_ACI_CLASS cmhSS_GetClassLst     ( T_basicServiceGroupList * bs );
EXTERN BOOL        cmhSS_CheckClass      ( T_ACI_CLASS class_type,
                                           UBYTE *bs1, UBYTE *bst1,
                                           UBYTE *bs2, UBYTE *bst2,
                                           BOOL  *mltyTrnFlg );
EXTERN BOOL        cmhSS_CheckCbClass    ( T_ACI_CLASS class_type,
                                           UBYTE *bs1, UBYTE *bst1,
                                           UBYTE *bs2, UBYTE *bst2,
                                           BOOL  *mltyTrnFlg );
EXTERN BOOL cmhSS_CheckClassInterr       ( T_ACI_CLASS class_type);
EXTERN BOOL cmhSS_CheckCbClassInterr     ( T_ACI_CLASS class_type);
EXTERN void        cmhSS_GetBscSrv       ( T_basicService * p, 
                                           UBYTE * bs,
                                           UBYTE * bst );
EXTERN BOOL        cmhSS_CheckBscSrv     ( UBYTE bsCd,
                                           UBYTE *bs, UBYTE *bst, T_ACI_CLASS *class_type );
EXTERN void        cmhSS_flagTrn         ( SHORT sId, USHORT * flags );
EXTERN BOOL        cmhSS_tstAndUnflagTrn ( SHORT sId, USHORT * flags );
EXTERN void        cmhSS_SSResultFailure ( SHORT sId );
EXTERN void        cmhSS_SSInterrogated  ( SHORT sId, 
                                           T_INTERROGATE_SS_RES *irgtSS );
EXTERN void        cmhSS_SSRegistrated   ( SHORT sId, 
                                           T_REGISTER_SS_RES *regSS );
EXTERN void        cmhSS_SSErased        ( SHORT sId, 
                                           T_ERASE_SS_RES *ersSS );
EXTERN void        cmhSS_SSActivated     ( SHORT sId, 
                                           T_ACTIVATE_SS_RES *actSS );
EXTERN void        cmhSS_SSDeactivated   ( SHORT sId, 
                                           T_DEACTIVATE_SS_RES *deactSS );
EXTERN void        cmhSS_getPassword     ( SHORT sId, 
                                           T_GET_PWD_INV *getPWD );
EXTERN void        cmhSS_SSPWDRegistrated( SHORT sId, 
                                           T_REGISTER_PWD_RES *regPWD );
EXTERN void        cmhSS_CCNtryErased    ( SHORT sId, 
                                           T_ERASE_CC_ENTRY_RES *ersCCNtry );
EXTERN void        cmhSS_USSDRequest     ( SHORT sId, 
                                           T_USSD_REQ_INV *ussdReq );
EXTERN void        cmhSS_USSDNotify      ( SHORT sId, 
                                           T_USSD_NOTIFY_INV *ussdNtfy );
EXTERN void        cmhSS_USSDReqProcessed( SHORT sId, 
                                           T_PROCESS_USSD_REQ_RES *prcUSSDReq );
EXTERN void        cmhSS_USSDDatProcessed( SHORT sId, 
                                           T_PROCESS_USSD_RES *prcUSSDdat );
EXTERN void        cmhSS_FwdChckSS       ( SHORT sId );

#ifdef KSD_H
EXTERN T_ACI_RETURN cmhSS_ksdCL          ( T_ACI_CMD_SRC srcId,
                                           T_ACI_KSD_CL_PRM * clPrm );
/* Implements Measure # 38 */
/* cmhSS_ksdCWCBStartTrans() directly called from cmhCC_chkKeySeq() */
EXTERN T_ACI_RETURN cmhSS_ksdUBLK        ( T_ACI_CMD_SRC srcId,
                                           T_ACI_KSD_UBLK_PRM * ublkPrm );
EXTERN T_ACI_RETURN cmhSS_ksdPW          ( T_ACI_CMD_SRC srcId,
                                           T_ACI_KSD_PWD_PRM * pwPrm );
/* Implements Measure # 38 */
/* cmhSS_ksdCWCBStartTrans() directly called from cmhCC_chkKeySeq() */
EXTERN T_ACI_RETURN cmhSS_ksdCF          ( T_ACI_CMD_SRC srcId,
                                           T_ACI_KSD_CF_PRM * cfPrm );
EXTERN T_ACI_RETURN cmhSS_ksdUSSD        ( T_ACI_CMD_SRC srcId,
                                           T_ACI_KSD_USSD_PRM * ussdPrm );
EXTERN T_ACI_RETURN cmhSS_ksdCCBS        ( T_ACI_CMD_SRC srcId,
                                           T_ACI_KSD_CCBS_PRM * ccbsPrm );
EXTERN T_ACI_RETURN cmhSS_ksdIMEI        ( T_ACI_CMD_SRC srcId );
#endif /* KSD_H */

EXTERN UBYTE        cmhSS_ksdGetOpCd     ( UBYTE opcSS );
EXTERN void         cmhSS_ksdBuildErrRslt( SHORT sId, T_ACI_KSIR * ksStat, UBYTE err );
EXTERN UBYTE        cmhSS_ksdFillFwdFeatList( T_forwardingFeatureList * ffSS,
                                              T_CF_FEAT * cfFeat );
EXTERN UBYTE        cmhSS_ksdFillCbFeatList( T_callBarringFeatureList * bfSS,
                                             T_CB_INFO * cbInfo );
EXTERN UBYTE        cmhSS_ksdFillBSGList ( T_basicServiceGroupList * bsgSS,
                                           T_Cx_BSG * cxBSG );
EXTERN void         cmhSS_ksdFillFwdRes  ( T_forwardingInfo * fiSS,
                                           T_ACI_KSIR *ksStat,
                                           T_CF_FEAT * cfFeat );
EXTERN void         cmhSS_ksdFillCbRes   ( T_callBarringInfo * biSS,
                                           T_ACI_KSIR *ksStat,
                                           T_CB_INFO * cbInfo );
EXTERN void         cmhSS_ksdFillCwRes   ( T_ssData   *datSS,
                                           T_ACI_KSIR *ksStat,
                                           T_Cx_BSG   *cwBSG );
EXTERN UBYTE        cmhSS_ksdFillCCBSFeatList( T_ccbsFeatureList * ccbsfSS,
                                               T_CC_FEAT * ccFeat );

#ifdef TI_PS_FF_AT_P_CMD_CSCN
EXTERN T_ACI_RETURN cmhSS_sendFie        ( T_ACI_FAC_DIR        tDirection,
                                           T_ACI_FAC_TRANS_TYPE tType,
                                           T_fac_inf           *fie );
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

EXTERN UBYTE cmhSS_getCdFromImei         ( T_ACI_IMEI* imei );

/* Implements Measure # 38 */
EXTERN T_ACI_RETURN cmhSS_ksdCWCBStartTrans( T_ACI_CMD_SRC srcId,  
                                             T_ACI_KSD_CMD ksdCmd,
                                             T_KSD_SEQPARAM * seqPrm );

/*==== EXPORT =====================================================*/
#endif /* CMH_SS_H */

/*==== EOF =======================================================*/

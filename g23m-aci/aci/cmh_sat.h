/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH
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
|             SIM application toolkit ( SAT )
+----------------------------------------------------------------------------- 
*/ 

#ifdef SIM_TOOLKIT

#ifndef CMH_SAT_H
#define CMH_SAT_H

/*==== CONSTANTS ==================================================*/
#define SAT_REDIAL  (0x80)      /* flag SAT redial condition */
#define SAT_REDIAL_ECCBE                (0x40)      /* Flag "Envelope Call Control 
                                                     * Always Sent To SIM"
                                                     *  bit enabled Condition 
                                                     */

#define SAT_GPRS_INV_CAUSE 0
#ifdef FF_SAT_E 
typedef enum 
{
  SAT_GPRS_ACT = 0, 
  SAT_GPRS_DEACT, 
  SAT_GPRS_ATT_FAILED, 
  SAT_GPRS_ACT_FAILED,
  SAT_GPRS_SUSPEND,
  SAT_GPRS_RESUME
} T_SAT_GPRS_CB_STAT;
#endif /* FF_SAT_E */
/*==== TYPES ======================================================*/
typedef struct
{
  SHORT          cId;
  T_CLPTY_PRM    clpty;
  T_ussd_string  ussd_str;
} T_SAT_PND_SETUP;

typedef struct
{
  T_addr        addr;
  T_subaddr     sub_addr;
  BUF_cap_cnf_parms   ccp;/* Capability configuration params added to redial structure */
} T_RDL_CC_ENV;

/*==== PROTOTYPES =================================================*/
EXTERN SHORT cmhSAT_STKCmdCnf              ( void );
EXTERN SHORT cmhSAT_STKCmdInd              ( void );
EXTERN SHORT cmhSAT_STKUsrNtfy             ( void );
#ifdef TI_PS_FF_AT_P_CMD_CUST
EXTERN SHORT cmhSAT_Cust1StkCmdInd  ( void );
#endif /* TI_PS_FF_AT_P_CMD_CUST */
EXTERN void         cmhCC_SatDTMFsent      ( SHORT cId );
EXTERN T_ACI_RETURN cmhSAT_CalCntrlBySIM   ( SHORT cId );
EXTERN T_ACI_RETURN cmhSAT_SSCntrlBySIM    ( T_CLPTY_PRM *cldPty,
                                             UBYTE own );
EXTERN T_ACI_RETURN cmhSAT_USSDCntrlBySIM  ( T_sat_ussd *ussd,
                                             UBYTE own );
EXTERN T_ACI_RETURN cmhSAT_DatDwnLdCB      ( UBYTE* cbMsg, SHORT cbLen );
EXTERN T_ACI_RETURN cmhSAT_MoSmCntr        ( T_rp_addr sc_addr,
                                             T_tp_da dest_addr,
                                             UBYTE       owner);
EXTERN void         cmhCC_SatDTMFsent      ( SHORT cId );
EXTERN T_ACI_RETURN cmhSAT_EventDwn        ( UBYTE event, SHORT callId,
                                             T_CC_INITIATER actionSrc );
EXTERN BOOL         cmhSAT_ResCalCntrlBySIM( UBYTE* resId, void *ccRes );
EXTERN BOOL         cmhSAT_ResSSCntrlBySIM ( UBYTE* resId, void *ccRes );
EXTERN BOOL         cmhSAT_ResUSSDCntrlBySIM ( UBYTE* resId, void *ccRes );
EXTERN BOOL         cmhSAT_ResSMCntrlBySIM ( UBYTE* resId, void *ccRes );
/* ACI-SPR-18200: added aci_events_only parameter */
EXTERN BOOL         cmhSAT_setupEvent_Test ( T_SETUP_EVENT *set_event, 
                                             BOOL *aci_events_only );
/* ACI-SPR-18200: Function to copy unproc events to MMI */
EXTERN BOOL         cmhSAT_copyUnprocEvents ( UBYTE* eventList,
                                              UBYTE  eventListLen,
                                              UBYTE  oldEventListLen);
EXTERN void         cmhSAT_setupEvent_Perform ( void );
EXTERN BOOL         cmhSAT_setupCall       ( T_SETUP_CALL * cal );
EXTERN BOOL         cmhSAT_sendSS          ( T_SEND_SS * ss );
EXTERN BOOL         cmhSAT_sendUSSD        ( T_SEND_USSD * ussd );
EXTERN BOOL         cmhSAT_sendDTMF        ( T_SEND_DTMF *dtmf );
EXTERN BOOL         cmhSAT_sendSM          ( T_SEND_SM * sm );
EXTERN BOOL         cmhSAT_runAt           ( T_RUN_AT *run_at);
EXTERN BOOL         cmhSAT_launchBrowser   ( T_LAUNCH_BROWSER* launchBrowser );
EXTERN BOOL         cmhSAT_provLocalInfo   ( void );
EXTERN void         cmhSAT_UserRejCall     ( UBYTE calStat );
EXTERN void         cmhSAT_CallCncted      ( void );
EXTERN UBYTE        cmhSAT_NtwErr          ( UBYTE cs );
EXTERN void         cmhSAT_UserClear       ( void );
EXTERN BOOL         cmhSAT_ChckCmdDet      ( void );
EXTERN LONG         cmhSAT_ChckRedial      ( SHORT cId,
                                             UBYTE v_dur, 
                                             T_dur * dur );
EXTERN BOOL         cmhSAT_UserAcptCall    ( SHORT acptId, UBYTE srcId );
EXTERN BOOL         cmhSAT_StartPendingCall( void );
EXTERN void         cmhSAT_fillSetupPrm    ( SHORT cId,
                                             T_addr* adr,
                                             T_subaddr *sub );
EXTERN void         cmhSAT_fillSetupBC     ( SHORT  cId,
                                             UBYTE  bearer_serv_1,
                                             UBYTE  bearer_serv_2  );
/* EXTERN void         cmhSAT_chkDTMF         ( SHORT cId, T_addr* adr ); */
EXTERN void         cmhSAT_ResCapCode      ( USHORT cause, 
                                             T_MNCC_bcconf* bc1,
                                             T_MNCC_bcconf2* bc2 );
EXTERN void         cmhSAT_ResCapDecode    ( USHORT cause, 
                                             T_MNCC_bcpara* bc1,
                                             T_MNCC_bcpara* bc2);
EXTERN BOOL         cmhSAT_SetupCalAfterCCRes ( UBYTE ownNotSAT,
                                                SHORT cId,
                                                UBYTE CCres);
EXTERN BOOL         cmhSAT_CheckSetEventResp ( void );

#ifdef FF_SAT_E
EXTERN BOOL         cmhSAT_CloseChannel      ( void );
EXTERN BOOL         cmhSAT_SendData          ( void );
EXTERN BOOL         cmhSAT_GetChannelStatus  ( void );
EXTERN BOOL         cmhSAT_OpenChannelReq    ( T_OPEN_CHANNEL *opchCmd );
EXTERN void         cmhSAT_storeCSDPrms      ( T_OPEN_CHANNEL *opchCmd );
EXTERN void         cmhSAT_storeGPRSPrms     ( T_OPEN_CHANNEL *opchCmd );
EXTERN void         cmhSAT_cleanupOpChnPrms  ( void );
EXTERN void         cmhSAT_OpChnResetCnctFlag( void );
EXTERN void         cmhSAT_OpChnFailed       ( UBYTE cause, 
                                               T_ACI_SAT_TERM_RESP *resp_data );
EXTERN void         cmhSAT_OpChnAlert        ( SHORT cId );
EXTERN BOOL         cmhSAT_OpChnChkTmpProblem( void );
EXTERN void         cmhSAT_OpChnUDPActiveCsd (T_ACI_RETURN result);
EXTERN void         cmhSAT_OpChnUDPConfCsd   (T_ACI_RETURN result);
EXTERN void         cmhSAT_OpChnUDPDeactCsd  (T_ACI_RETURN result);

EXTERN void         cmhSAT_OpBIPChnOpen      (UBYTE bipConn, UBYTE chnId);
EXTERN void         cmhSAT_OpChnCnct         (UBYTE dtiConn, UBYTE chnId);
EXTERN void         cmhSAT_OpChnClose        (UBYTE bipConn, UBYTE chnId);
EXTERN void         cmhSAT_OpChnCSDDown      (SHORT cId, UBYTE tpl );
#ifdef GPRS 
EXTERN void         cmhSAT_OpChnGPRSDeact    ( void );
#endif /* GPRS */
EXTERN BOOL         cmhSAT_OpChnChckCSD      (UBYTE tpl);
EXTERN void         cmhSAT_OpChnSIMFail      (UBYTE dtiConn, UBYTE bipConn, UBYTE chnId);
EXTERN void         cmhSAT_OpChnSIMCnctReq   (UBYTE unit);
EXTERN void         cmhSAT_OpChnChckBear     (void);
#ifdef DTI
EXTERN void         cmhSAT_OpChnSetPPP       (UBYTE chnType);
EXTERN void         cmhSAT_OpChnUDPActivateGprs( void );
#endif
EXTERN void         cmhSAT_OpChnStatEvnt     ( void );
EXTERN BOOL         cmhSAT_OpChnGPRSPend     ( SHORT cid, UBYTE opchStat );
EXTERN void         cmhSAT_OpChnGPRSStat     (T_SAT_GPRS_CB_STAT stat, UBYTE cause);
EXTERN void         cmhSAT_OpChnUDPConfGprs  (void);
EXTERN void         cmhSAT_OpChnUDPDeactGprs (void);
EXTERN void         cmhSAT_UserRejCntxt      ( void );
EXTERN BOOL         cmhSAT_UserAcptCntxt     ( UBYTE srcId );
EXTERN void         cmhSAT_cnvrtAPN2NetworkAdr( UBYTE *apn, UBYTE c_apn, UBYTE *dom_name );
#endif /* SAT E */

EXTERN void         cmhSAT_CBMDestroyList  ( void );

/*==== EXPORT =====================================================*/
#ifdef CMH_SATF_C

GLOBAL T_ENT_STAT         satEntStat;
GLOBAL T_SAT_PND_SETUP    satPndSetup;
/* GLOBAL T_CC_DTMF_PRM      satDtmfBuf; */
GLOBAL T_RDL_CC_ENV       satRdlCCEnv;  /* Used to store the CC Envelope when Envelope call 
                                           control bit is enabled. */
GLOBAL T_MNSMS_SUBMIT_REQ *sat_mnsms_submit_req;

#else

EXTERN T_ENT_STAT       satEntStat;
EXTERN T_SAT_PND_SETUP  satPndSetup;
/* EXTERN T_CC_DTMF_PRM    satDtmfBuf; */
EXTERN T_RDL_CC_ENV     satRdlCCEnv;

EXTERN T_MNSMS_SUBMIT_REQ *sat_mnsms_submit_req;

#endif /* CMH_SATF_C */

#endif /* CMH_SAT_H */

#endif /* #ifdef SIM_TOOLKIT */

/*==== EOF =======================================================*/

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
|  Purpose :  Definitions for the command handler of Call Control
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_CC_H
#define CMH_CC_H

/* #include "cmh.h" */


/*==== CONSTANTS ==================================================*/

typedef enum
{
  NO_CHLD_ADD_INFO = 0,   /* no additional CHLD info */
  CHLD_ADD_INFO_RTV_CAL,  /* additional CHLD info, retrieve call */
  CHLD_ADD_INFO_ACC_CAL,  /* additional CHLD info, accept call */
  CHLD_ADD_INFO_DIAL_CAL  /* additional CHLD info, dial call */
} T_CHLD_ADD_INFO;

typedef enum
{
  NO_VLD_CC_SIM_QLF = 0,  /* not a valid SIM CC qualifier */
  CC_SIM_YES,             /* yes, perform SIM CC check */
  CC_SIM_NO               /* no, do not perform SIM CC check */
} T_CC_SIM_QLF;

typedef enum
{
  TTY_STOP,               /* stop TTY Service */
  TTY_PAUSE,              /* pause TTY Service (call held/modified) */
  TTY_TCH,                /* TCH assigned */
  TTY_START               /* start TTY Service, if possible */
} T_CC_TTY_ACTION;

typedef enum
{
  NO_VLD_CC_CALL_TYPE = -1,
  VOICE_CALL = 0
#ifdef FAX_AND_DATA
  ,
  TRANS_CALL,
  NON_TRANS_CALL,
  FAX_CALL,
  UNSUP_DATA_CALL
#endif /* FAX_AND_DATA */
#ifdef CO_UDP_IP
  ,
  UDPIP_CALL
#endif /* CO_UDP_IP */ 
#ifdef FF_PPP
  ,
  PPP_CALL
#endif /* FF_PPP */
#ifdef FF_GPF_TCPIP
  ,
  TCPIP_CALL
#endif /* FF_GPF_TCPIP */
}
T_CC_CALL_TYPE;

/*==== TYPES ======================================================*/


/*==== PROTOTYPES =================================================*/

/* 
 * PSA notification events
 */
EXTERN void cmhCC_CallConnected   ( SHORT cId );
EXTERN void cmhCC_CallDisconnected( SHORT cId );
EXTERN void cmhCC_CallReleased    ( SHORT cId );
EXTERN void cmhCC_CPIReleaseMsg    ( SHORT cId );
EXTERN void cmhCC_CPIrejectMsg    ( SHORT cId );
EXTERN void cmhCC_CallProceeding  ( SHORT cId );
EXTERN void cmhCC_CallAlerted     ( SHORT cId );
EXTERN void cmhCC_CallModified    ( SHORT cId );
EXTERN void cmhCC_IncomingCall    ( SHORT cId );
/*EXTERN void cmhCC_DisconnectCall  ( SHORT cId );*/ // HM 11-May-2005: Dead code
EXTERN void cmhCC_Synchronized    ( SHORT cId );  
EXTERN void cmhCC_CallRetrieved   ( SHORT cId );
EXTERN void cmhCC_CallHeld        ( SHORT cId );
EXTERN void cmhCC_RA_Activated    ( SHORT cId );
EXTERN void cmhCC_RA_Deactivated  ( SHORT cId );
EXTERN void cmhCC_RA_Modified     ( SHORT cId );
EXTERN void cmhCC_L2R_Failed      ( void );
EXTERN void cmhCC_T30_Activated   ( void );
EXTERN void cmhCC_T30_Deactivated ( void );
EXTERN void cmhCC_T30_Failed      ( void );
EXTERN void cmhCC_T30_RmtCaps     ( void );
EXTERN void cmhCC_SSTransFail     ( SHORT cId );


EXTERN void         cmhCC_get_active_als_mode( T_ACI_CMD_SRC srcId, T_ACI_ALS_MOD *mode );
EXTERN void         cmhCC_init_cldPty     ( T_CLPTY_PRM *cldPty );
EXTERN BOOL         cmhCC_SendDTMFdig     ( T_ACI_AT_CMD cmd, SHORT cId, CHAR digit, UBYTE mode);
EXTERN SHORT        cmhCC_find_call_for_DTMF( void );
EXTERN BOOL         is_call_ok_for_dtmf   ( SHORT cId );
EXTERN T_ACI_RETURN cmhCC_chkShortString  ( T_ACI_CMD_SRC srcId,
                                            SHORT         cId,
                                            T_CLPTY_PRM   *cldPty);

EXTERN void         cmhCC_PrepareCmdEnd   (SHORT cId, UBYTE *cmdBuf, UBYTE *srcBuf);

EXTERN UBYTE        cmhCC_set_speech_serv ( T_CC_CMD_PRM   *pCCCmdPrm); 

#ifdef FAX_AND_DATA
EXTERN UBYTE        cmhCC_SelCE           ( T_ACI_CBST_CE  ce );
EXTERN UBYTE        cmhCC_SelServ         ( T_ACI_CBST_NAM name );
EXTERN UBYTE        cmhCC_SelRate         ( T_ACI_BS_SPEED speed );
EXTERN UBYTE        cmhCC_SelMT           ( T_ACI_BS_SPEED speed );
EXTERN UBYTE        cmhCC_SelTransferCap  ( T_ACI_BS_SPEED speed );
EXTERN UBYTE        cmhCC_SelRateAdaption ( T_ACI_BS_SPEED speed);
EXTERN UBYTE        cmhCC_SelStopBit      ( T_ACI_CMD_SRC srcId );
EXTERN UBYTE        cmhCC_SelDataBit      ( T_ACI_CMD_SRC srcId );
EXTERN UBYTE        cmhCC_SelParity       ( T_ACI_CMD_SRC srcId );
#endif /* FAX_AND_DATA */
EXTERN SHORT        cmhCC_GetCallType_from_bearer( void * bearCap );
#ifdef FAX_AND_DATA
EXTERN SHORT        cmhCC_GetSrvType      ( void * bearCap );
EXTERN SHORT        cmhCC_GetDataRate     ( void * bearCap );
EXTERN SHORT        cmhCC_GetParity       ( void * bearCap );
EXTERN SHORT        cmhCC_GetFormat       ( void * bearCap );
#endif /* FAX_AND_DATA */
EXTERN T_ACI_CLASS  cmhCC_GetCallClass    ( SHORT cId );
EXTERN T_ACI_TOA*   cmhCC_ctbGetCldNumTyp ( SHORT cId,
                                            T_ACI_TOA * pToaBuf );
EXTERN T_ACI_TOS*   cmhCC_ctbGetCldSubTyp ( SHORT cId,
                                            T_ACI_TOS * pTosBuf );
EXTERN T_ACI_TOA*   cmhCC_ctbGetClrNumTyp ( SHORT cId,
                                            T_ACI_TOA * pToaBuf );
EXTERN T_ACI_TOS*   cmhCC_ctbGetClrSubTyp ( SHORT cId,
                                            T_ACI_TOS * pTosBuf );
EXTERN T_ACI_TOA*   cmhCC_ctbGetRdrNumTyp ( SHORT cId, 
                                            T_ACI_TOA * pToaBuf );
EXTERN T_ACI_TOS*   cmhCC_ctbGetRdrSubTyp ( SHORT cId, 
                                            T_ACI_TOS * pTosBuf );
EXTERN void         cmhCC_SndDiscRsn      ( SHORT cId );
EXTERN BOOL         cmhCC_ChckInCallMdfy  ( SHORT cId, UBYTE cmd );
EXTERN void         cmhCC_flagCall        ( SHORT cId,
                                            USHORT * flags );
EXTERN BOOL         cmhCC_tstAndUnflagCall( SHORT cId,
                                            USHORT * flags );
EXTERN void         cmhCC_ClearCall       ( SHORT cId, 
                                            USHORT cs,
                                            T_ACI_CMD_SRC srcId,
                                            UBYTE cmd, 
                                            SHORT *waitId);
EXTERN void         cmhCC_NewCall         ( SHORT cId, 
                                            T_ACI_CMD_SRC srcId,
                                            T_ACI_AT_CMD cmd );
EXTERN void         cmhCC_RetrieveCall    ( SHORT cId,
                                            T_ACI_CMD_SRC srcId );
EXTERN void         cmhCC_AcceptCall      ( SHORT cId, 
                                            T_ACI_CMD_SRC srcId,
                                            T_ACI_AT_CMD cmd );
EXTERN void         cmhCC_HoldCall        ( SHORT cId, 
                                            T_ACI_CMD_SRC srcId, 
                                            T_ACI_AT_CMD cmd );
EXTERN BOOL         cmhCC_atdsendok       ( SHORT cId );

EXTERN void         send_CSSX_notification(SHORT ctbIdx,
                                            T_ACI_CSSX_CODE cssx_code,
                                            SHORT           index,
                                            CHAR            *number,
                                            T_ACI_TOA       *toa,
                                            CHAR            *subaddr,
                                            T_ACI_TOS       *tos);

EXTERN void         cmhrat_ccbs           ( UBYTE           srcId,
                                            T_ACI_CCBS_IND  ccbs_ind,
                                            T_ACI_CCBS_STAT status,
                                            T_ACI_CCBS_SET  *setting );
EXTERN T_CC_CALL_TYPE cmhCC_getcalltype   ( SHORT cId );
EXTERN void         cmhCC_NotifySS        ( SHORT cId,
                                            T_NOTIFY_SS_INV *ntfySS );
EXTERN void         cmhCC_CheckSS         ( SHORT cId );
EXTERN void         cmhCC_MPTYBuild       ( SHORT cId,
                                            T_BUILD_MPTY_RES *bldMPTY );
EXTERN void         cmhCC_MPTYSplit       ( SHORT cId,
                                            T_SPLIT_MPTY_RES *splMPTY );
EXTERN void         cmhCC_MPTYHeld        ( SHORT cId,
                                            T_HOLD_MPTY_RES *hldMPTY );
EXTERN void         cmhCC_MPTYRetrieved   ( SHORT cId,
                                            T_RETRIEVE_MPTY_RES *rtvMPTY );
EXTERN void         cmhCC_MPTYTimeout     ( void );
EXTERN void         cmhCC_ECTTimeout      ( void );
EXTERN void         cmhCC_CCBSRegistered  ( SHORT cId,
                                            T_ACC_REGISTER_CC_ENTRY_RES *CCBSreg );
EXTERN void         cmhCC_CDRegistered    ( SHORT cId );
EXTERN T_ACI_RETURN cmhCC_chkKeySeq       ( T_ACI_CMD_SRC srcId,
                                            T_CLPTY_PRM *cldPty,
                                            T_ACI_D_TOC *callType,
                                            T_ACI_D_CLIR_OVRD *CLIRovrd,
                                            T_CC_SIM_QLF ccSIMQlf );
EXTERN T_ACI_RETURN cmhCC_Dial            ( T_ACI_CMD_SRC srcId,
                                            T_CLPTY_PRM     * cldPty,
                                            T_ACI_D_CLIR_OVRD clirOvrd,
                                            T_ACI_D_CUG_CTRL  cugCtrl,
                                            T_ACI_D_TOC       callType );
EXTERN T_ACI_RETURN cmhCC_fillSetupPrm    ( SHORT              cId,
                                            T_ACI_CMD_SRC      srcId,
                                            T_CLPTY_PRM       *cldPty,
                                            T_MNCC_bcpara     *bc,
                                            UBYTE              prio,
                                            T_ACI_D_CLIR_OVRD  clirOvrd,
                                            T_ACI_D_CUG_CTRL   cugCtrl,
                                            T_ACI_D_TOC        callType );
EXTERN void         cmhCC_chkDTMFDig      ( CHAR  *num,
                                            SHORT cId,
                                            USHORT length,
                                            BOOL within_dial_string );
EXTERN void         cmhCC_DTMFsent        ( SHORT cId );
EXTERN void         cmhCC_DTMFstopped     ( SHORT cId );
EXTERN BOOL         is_digit_dtmf_separator(CHAR digit);
#ifdef FF_TTY
EXTERN T_ACI_CTTY_TRX cmhCC_getTTYtrx_state (int ttyTrxState);
EXTERN void         cmhCC_notifyTTY       ( T_ACI_CTTY_NEG neg,
                                            T_ACI_CTTY_TRX trx );
EXTERN void         cmhCC_TTY_Control     ( SHORT cId, UBYTE action );
#else
#define cmhCC_TTY_Control(_x_,_y_)
#endif

EXTERN void cmhCC_ChngWaitingToIncoming   ( void );
EXTERN void cmhCC_checkALS_Support        ( );
EXTERN void cmhCC_checkALS_Support_cb     ( SHORT aId );
EXTERN void cmhCC_checkALS_Support_2      ( );
EXTERN void cmhCC_checkALS_Support_cb_2   ( SHORT aId );
GLOBAL void cmhCC_checkALS_Support_exec   ( UBYTE flag );

EXTERN T_ACI_RETURN cmhCC_sendFie( T_ACI_FAC_DIR tDirection,
                                      SHORT         cId,
                                      T_MNCC_fac_inf    *fie );
#if defined(_TARGET_)
EXTERN T_ACI_RETURN cmhCC_rd_mode_FFS(T_ACI_CC_REDIAL_MODE rdl_mode,
                                      T_ACI_CC_RDL_FFS ffs_mode);
#endif /* _TARGET_ */
EXTERN void cmhCC_redialTimeout ( void );
EXTERN void cmhCC_satTimeout (void );
EXTERN void cmhCC_redialCheck(SHORT cId);
EXTERN T_ACI_RETURN cmhCC_redialChkBlackl(SHORT cId);
EXTERN void cmhCC_CheckRedialTimer( BOOL sim_toolkit_enable );
/*==== EXPORT =====================================================*/

#ifdef CMH_CCF_C

GLOBAL T_CHLD_ADD_INFO CHLDaddInfo;
GLOBAL T_ACI_CC_REDIAL_PAR rdlPrm;
#else

EXTERN T_CHLD_ADD_INFO CHLDaddInfo;
EXTERN T_ACI_CC_REDIAL_PAR rdlPrm;
#endif  /* CMH_CCF_C */


#endif /* CMH_CC_H */

/*==== EOF =======================================================*/

/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_T30
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
|  Purpose :  Definitions for the command handler of T30.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_T30_H
#define CMH_T30_H

#ifdef FF_FAX

/*==== CONSTANTS ==================================================*/
#define PRI_MRK         (8) /* procedure interrupt marker */

/*==== TYPES ======================================================*/
typedef enum
{
  NO_VLD_FRT = 0,           /* not a valid frame type */
  FRT_DIS,                  /* DIS frame type */
  FRT_DTC,                  /* DTC frame type */
  FRT_DCS                   /* DCS frame type */
} T_T30_FRTP;

typedef struct              /* negotiated parameters */
{
  T_ACI_F_VR  FCSvr;
  T_ACI_F_BR  FCSbr;
  T_ACI_F_WD  FCSwd;
  T_ACI_F_LN  FCSln;
  T_ACI_F_DF  FCSdf;
  T_ACI_F_EC  FCSec;
  T_ACI_F_BF  FCSbf;
  T_ACI_F_ST  FCSst;
  T_ACI_F_JP  FCSjp;

} T_T30_NGT_PRMS;

typedef struct entT30Status          /* entity status */
{
  T_ACI_CMD_SRC  entOwn;          /* entity owner */
  T_ACI_AT_CMD   curCmd;          /* current command processing */
  BOOL           isTempDisconnected;            
} T_T30_ENT_STAT;

/*==== PROTOTYPES =================================================*/

EXTERN void           cmhT30_RTN             ( void );
EXTERN void           cmhT30_RTP             ( void );
EXTERN void           cmhT30_HDLCRpt         ( void );
EXTERN void           cmhT30_ProcIntInst     ( void );
EXTERN void           cmhT30_ProcInt         ( void );
EXTERN void           cmhT30_PageReceivedPRI ( void );
EXTERN void           cmhT30_ProcEndPRI      ( void );
EXTERN void           cmhT30_DocReceivedPRI  ( void );
EXTERN void           cmhT30_NextPage        ( void );
EXTERN void           cmhT30_NextDoc         ( void );
EXTERN void           cmhT30_PageReceived    ( void );
EXTERN void           cmhT30_FTT             ( void );
EXTERN void           cmhT30_PreambleRcvd    ( void );
EXTERN void           cmhT30_ProcEnd         ( void );
EXTERN void           cmhT30_DocReceived     ( void );
EXTERN void           cmhT30_Disconnected    ( void );
EXTERN void           cmhT30_Disconnect      ( void );
EXTERN void           cmhT30_Deactivated     ( void );
EXTERN void           cmhT30_TransCmpl       ( void );
EXTERN void           cmhT30_CapRmtSite      ( void );
EXTERN void           cmhT30_Activated       ( void );

EXTERN void         cmhT30_Failure      ( void );
EXTERN T_ACI_RETURN cmhT30_Activate     ( T_ACI_CMD_SRC srcId, 
                                          T_ACI_AT_CMD cmdId,
                                          SHORT cId );
EXTERN T_ACI_RETURN cmhT30_Deactivate   ( void );
EXTERN T_ACI_RETURN cmhT30_Modify       ( void );
EXTERN T_ACI_RETURN cmhT30_SendCaps     ( T_ACI_CMD_SRC srcId, 
                                          T_T30_FRTP frmTyp );
EXTERN T_ACI_F_BR     cmhT30_Chn2BitRate  ( void );
EXTERN USHORT         cmhT30_SelChnRate   ( void );
EXTERN UBYTE          cmhT30_SelHlfRate   ( void );
EXTERN USHORT         cmhT30_SelUsrRate   ( void );
EXTERN UBYTE          cmhT30_SelBitOrder  ( T_ACI_CMD_SRC srcId );
EXTERN BOOL           cmhT30_Chk4TCHAdpt  ( void );
EXTERN T_ACI_BS_SPEED cmhT30_GetDataRate  ( void );
EXTERN T_ACI_F_VR     cmhT30_GetResolution( void * p, 
                                            T_T30_FRTP frmTyp );
EXTERN T_ACI_F_BR     cmhT30_GetBitRate   ( void * p, 
                                            T_T30_FRTP frmTyp );
EXTERN T_ACI_F_WD     cmhT30_GetPageWidth ( void * p );
EXTERN T_ACI_F_LN     cmhT30_GetPageLength( void * p );
EXTERN T_ACI_F_DF     cmhT30_GetDataComp  ( void * p );
EXTERN T_ACI_F_EC     cmhT30_GetErrCorr   ( void * p );
EXTERN T_ACI_F_BF     cmhT30_GetFileTrnsfr( void * p );
EXTERN T_ACI_F_ST     cmhT30_GetScanTime  ( void * p );
EXTERN T_ACI_F_JP     cmhT30_GetJPEG      ( void * p );
EXTERN void           cmhT30_RstNgtPrms   ( void );
EXTERN void           cmhT30_InitFAXPrms  ( T_ACI_CMD_SRC srcId );
EXTERN void           cmhT30_BuildSndFrm  ( T_ACI_CMD_SRC srcId, 
                                            T_T30_FRTP frmTyp );
/* never called !!? EXTERN void           cmhT30_PPMRcvd      ( UBYTE ppm ); */
EXTERN void           cmhT30_PRIRcvd      ( void );
EXTERN void           cmhT30_NgtDCEPrms   ( T_ACI_CMD_SRC srcId );
EXTERN UBYTE          cmhT30_GetPpr       ( T_ACI_FPS_PPR ppr );
EXTERN void           cmhT30_FITTimeout   ( void );
EXTERN void           cmhT30_StopFIT      ( void );
EXTERN void           cmhT30_StartFIT     ( void );

/* call-back for DTI Mng */
#ifdef DTI
EXTERN BOOL T30_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type);
#endif /* DTI */

/*==== EXPORT =====================================================*/
#ifdef CMH_T30F_C

GLOBAL T_T30_ENT_STAT t30EntStat;
GLOBAL T_T30_NGT_PRMS t30NgtPrms;
GLOBAL T_ACI_FHS_STAT FHSstat; 
GLOBAL BOOL           pageSentFlg   = TRUE;
GLOBAL BOOL           DTCSentFlg    = FALSE;
GLOBAL BOOL           PRIRcvdFlg    = FALSE;
GLOBAL BOOL           ppmPendFlg    = FALSE;
GLOBAL BOOL           FITRunFlg     = FALSE;

#else

EXTERN T_T30_ENT_STAT t30EntStat;
EXTERN T_T30_NGT_PRMS t30NgtPrms;
EXTERN T_ACI_FHS_STAT FHSstat; 
EXTERN BOOL           pageSentFlg;
EXTERN BOOL           DTCSentFlg;
EXTERN BOOL           PRIRcvdFlg;
EXTERN BOOL           ppmPendFlg;
EXTERN BOOL           FITRunFlg;
#endif /* CMH_T30F_C */

#endif /* FF_FAX */

#endif /* CMH_T30_H */

/*==== EOF =======================================================*/


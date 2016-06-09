/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_RA
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
|  Purpose :  Definitions for the command handler of Rate Adaptation
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_RA_H
#define CMH_RA_H

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/

typedef struct entRaStatus          /* entity status */
{
  T_ACI_CMD_SRC  entOwn;          /* entity owner */
  T_ACI_AT_CMD   curCmd;          /* current command processing */
  BOOL           isTempDisconnected;            
} T_RA_ENT_STAT;

/*==== PROTOTYPES =================================================*/
EXTERN void         cmhRA_Deactivated( SHORT cId );
EXTERN void         cmhRA_Activated  ( SHORT cId );
EXTERN UBYTE        cmhRA_SelTrfProt ( SHORT cId );
EXTERN UBYTE        cmhRA_SelChnRate ( void );
EXTERN UBYTE        cmhRA_SelUsrRate ( SHORT cId );
EXTERN UBYTE        cmhRA_SelDataBits( SHORT cId );
EXTERN UBYTE        cmhRA_SelStopBits( SHORT cId ); 
EXTERN T_ACI_RETURN cmhRA_Activate   ( T_ACI_CMD_SRC srcId, 
                                       T_ACI_AT_CMD cmdId,
                                       SHORT cId );
EXTERN T_ACI_RETURN cmhRA_Deactivate ( void );
EXTERN T_ACI_RETURN cmhRA_SendBreak  ( T_ACI_CMD_SRC srcId,
                                       USHORT        break_len);
EXTERN T_ACI_RETURN cmhRA_Escape     ( void );

#ifdef FF_FAX
EXTERN UBYTE        cmhRA_SelBitOrder( T_ACI_CMD_SRC srcId );
EXTERN T_ACI_RETURN cmhRA_Modify     ( T_ACI_CMD_SRC srcId,
                                       SHORT         cId );   
#endif /* FF_FAX */

/*==== EXPORT =====================================================*/

/* FAX_AND_DATA related Command Parameters... moved from cmh.h */

typedef struct l2rCmdPrm      /* command parameters related to L2R */
{
  USHORT        CRLPiws;
  USHORT        CRLPmws;
  USHORT        CRLPt1;
  USHORT        CRLPn2;
  T_ACI_DS_DIR  DSdir;
  T_ACI_DS_COMP DScomp;
  LONG          DSmaxDict;
  SHORT         DSmaxStr;
} T_L2R_CMD_PRM;

#ifdef FF_FAX

typedef struct t30CmdPrm      /* command parameters related to T30 */
{
  T_ACI_FCLASS_CLASS  FCLASSclass;
  T_ACI_FCR_VAL       FCRval;
  T_ACI_F_VR          FCCvr;
  T_ACI_F_BR          FCCbr;
  T_ACI_F_WD          FCCwd;
  T_ACI_F_LN          FCCln;
  T_ACI_F_DF          FCCdf;
  T_ACI_F_EC          FCCec;
  T_ACI_F_BF          FCCbf;
  T_ACI_F_ST          FCCst;
  T_ACI_F_JP          FCCjp;
  T_ACI_F_VR          FISvr;
  T_ACI_F_BR          FISbr;
  T_ACI_F_WD          FISwd;
  T_ACI_F_LN          FISln;
  T_ACI_F_DF          FISdf;
  T_ACI_F_EC          FISec;
  T_ACI_F_BF          FISbf;
  T_ACI_F_ST          FISst;
  T_ACI_F_JP          FISjp;
  CHAR                FLIstr[MAX_ID_CHAR];
  CHAR                FPIstr[MAX_ID_CHAR];
  T_ACI_FLP_VAL       FLPval;
  T_ACI_FAP_VAL       FAPsub;
  T_ACI_FAP_VAL       FAPsep;
  T_ACI_FAP_VAL       FAPpwd;
  CHAR                FSAsub[MAX_ID_CHAR];
  CHAR                FPAsep[MAX_ID_CHAR];
  CHAR                FPWpwd[MAX_ID_CHAR];
  UBYTE               FNSoct[MAX_NSF_BYTE];
  UBYTE               FNSlen;
  T_ACI_FCQ_RQ        FCQrq;
  T_ACI_F_BR          FMSbr;
  T_ACI_FPS_PPR       FPSppr;
  T_ACI_FSP_VAL       FSPval;
  T_ACI_FIE_VAL       FIEval;
  T_ACI_FIT_ACT       FITact;
  LONG                FITtime;
  T_ACI_FBO_VAL       FBOval;

} T_T30_CMD_PRM;

#endif /* FF_FAX */

typedef struct fnd_cmhCmdPrm      /* fax_n_data handler command parameter */
{
  T_L2R_CMD_PRM l2rCmdPrm;
#ifdef FF_FAX
  T_T30_CMD_PRM t30CmdPrm;
#endif
} FND_T_CMH_PRM;


#ifdef CMH_RAF_C

GLOBAL T_RA_ENT_STAT raEntStat;
GLOBAL FND_T_CMH_PRM  fnd_cmhPrm[CMD_SRC_MAX];
#else

EXTERN T_RA_ENT_STAT raEntStat;
EXTERN FND_T_CMH_PRM  fnd_cmhPrm[CMD_SRC_MAX];
#endif /* CMH_RAF_C */

#endif /* CMH_RA_H */

/*==== EOF =======================================================*/

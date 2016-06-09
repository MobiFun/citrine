/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_MM
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
|             Mobility Mangement
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_MM_H
#define CMH_MM_H


/*==== CONSTANTS ==================================================*/
#define PNN_SHORT_NAME_IEI	0x45
#define PNN_LONG_NAME_IEI	  0x43

/*==== TYPES ======================================================*/

/* According to ITE-E 212 MNC shall not be more that 3 digits 
     Thus 0xffff is invalid number and means that rules are same 
     for all Operators in the country*/
#define ALL_MNC 0x7fff 

#define MM_FFS_OPER_DIR "/gsm/cops"
#define MM_FFS_OPER_FILE "/gsm/cops/operimsi"


typedef struct FFSPLMNIMSI
{
  UBYTE mcc[SIZE_MCC];
  UBYTE mnc[SIZE_MNC];
  UBYTE  IMSI[MAX_IMSI-1];
} T_FFSPLMNIMSI;

/* Type for ECCIgnoreTable. See descr for ECCIgnoreTable*/
typedef struct ECCIgnoreRec
{
  SHORT mcc;
  SHORT mnc;
  char *ecc;
} T_ECCIgnoreRec;

/* Type for CPHS ONS reade state*/
typedef enum {
  ONS_READ_NOT_DONE = 0,
  ONS_READING,
  ONS_READ_OVER,
  ONS_READ_FAIL
} T_ONS_READ_STATE;

/*==== PROTOTYPES =================================================*/
EXTERN SHORT            cmhMM_Registered   ( void );
EXTERN SHORT            cmhMM_Deregistered ( void );
EXTERN SHORT            cmhMM_SelNetwork ( USHORT cause );
EXTERN SHORT            cmhMM_NetworkLst ( void );

EXTERN BOOL             cmhMM_FindPLMN     ( T_OPER_ENTRY * plmnDesc, 
                                             SHORT mcc, SHORT mnc, U16 lac, BOOL nw_search );
EXTERN BOOL             cmhMM_getBandSettings( UBYTE *SetBands,
                                               UBYTE *AllowedBands );
EXTERN BOOL             cmhMM_isBandAllowed( UBYTE band,
                                             UBYTE AllowedBands );
EXTERN BOOL             cmhMM_writeSetBand ( UBYTE setband );
EXTERN BOOL             cmhMM_FindNumeric  ( T_OPER_ENTRY * plmnDesc,
                                             const CHAR * numStr );
EXTERN BOOL             cmhMM_FindName     ( T_OPER_ENTRY * plmnDesc,
                                             const CHAR * longStr,
                                                   T_ACI_CPOL_FRMT format );
EXTERN void             cmhMM_CnvrtPLMN2INT( const UBYTE * BCDmcc,
                                             const UBYTE * BCDmnc,
                                                   SHORT * mccBuf,
                                                   SHORT * mncBuf );
EXTERN void             cmhMM_CnvrtINT2PLMN(       SHORT INTmcc,
                                                   SHORT INTmnc,
                                                   UBYTE * mccBuf,
                                                   UBYTE * mncBuf );
EXTERN T_ACI_CREG_STAT  cmhMM_GetNregCREGStat( void );
EXTERN T_ACI_CME_ERR    cmhMM_GetNregCMEStat ( void );
EXTERN USHORT           cmhMM_GetOperLstLen  ( void );
EXTERN void             cmhMM_CnvrtTrmPCMOpNam( T_OPER_ENTRY *plmnDesc, void *pPCMBuf );
EXTERN void             cmhMM_Ntfy_NtwRegistrationStatus( T_ACI_CREG_STAT creg );
EXTERN SHORT            cmhMM_CipheringInd  ( UBYTE ciph );
EXTERN BOOL             cmhMM_OpCheckName ();  /* EONS function */
EXTERN void             cmhMM_OpCheckList ();/* EONS function */
EXTERN BOOL             cmhMM_OpReadName ( UBYTE rec_num);/* EONS function */
EXTERN void             cmhMM_OpReadNameCb ( SHORT table_id);/* EONS function */
EXTERN void             cmhMM_OpSetPNNLst(); /* EONS function */
EXTERN void             cmhMM_OpExtractNameCB ( SHORT table_id); /* EONS function */
EXTERN BOOL             cmhMM_OpUpdateName (); /* EONS function */
EXTERN void             cmhMM_GetCmerSettings (T_ACI_CMD_SRC srcId, T_ACI_MM_CMER_VAL_TYPE *sCmerSettings );
EXTERN BOOL             cmhMM_ChkIgnoreECC(CHAR *dialStr);

EXTERN T_ACI_RETURN cmhMM_OperatorSelect(T_ACI_CMD_SRC srcId,
                                   T_ACI_NRG_RGMD regMode,
                                    T_ACI_NRG_SVMD srvMode,
                                    T_ACI_NRG_FRMT oprFrmt,
                                    CHAR          *opr);
                                    
EXTERN void cmhMM_OperatorQuery( T_ACI_CMD_SRC srcId,
                                    T_ACI_COPS_FRMT format,
                                    CHAR           *oper);

EXTERN BOOL  cmhMM_OperatorStoreInFFS(UBYTE* mcc, UBYTE* mnc, UBYTE* IMSI);
EXTERN BOOL cmhMM_OperatorReadFromFFS(UBYTE* mcc, UBYTE* mnc, UBYTE* IMSI);




EXTERN BOOL             cmhMM_GetActingHPLMN(SHORT * mccBuf, SHORT * mncBuf);
EXTERN BOOL             cmhMM_ONSReadName ();//EONS function
EXTERN void             cmhMM_ONSReadNameCb ( SHORT table_id);//EONS function
EXTERN void             cmhMM_Reset_ONSDesc ();

/*==== EXPORT =====================================================*/

#ifdef CMH_MMF_C

GLOBAL T_ENT_STAT mmEntStat;
GLOBAL BOOL regReqPnd = FALSE;
#else

EXTERN T_ENT_STAT mmEntStat;
EXTERN BOOL regReqPnd;

#endif /* CMH_MMF_C */
#endif /* CMH_MM_H */

/*==== EOF =======================================================*/

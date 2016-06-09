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
|             Subscriber Identity Module ( SIM )
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_SIM_H
#define CMH_SIM_H

/*==== CONSTANTS ==================================================*/
 
#define ACI_SIZE_EF_CCP          14
#define ACI_MIN_SIZE_EF_MSISDN   14
#define ACI_MIN_SIZE_EF_PLMN_SEL 24
#define ACI_LEN_PLMN_SEL_NTRY     3   /* length of an entry in bytes */
#define ACI_LEN_PLMN_SEL_FLD     MAX_PREF_PLMN /* Moved to SAP SIM to be consistent with MM */

 
#define ACI_LEN_LAN_NTRY          2   /* length of an entry in bytes */
#define ACI_MAX_LAN_NTRY          10   /* number of supported entries */
#define ACI_MIN_SIZE_EF_LAN       16
#define ACI_LEN_LAN_FLD (ACI_LEN_LAN_NTRY*ACI_MAX_LAN_NTRY)


#define ACI_LEN_LAN_LP_NTRY       1   /* length of an entry in bytes */
#define ACI_MAX_LAN_LP_NTRY       5   /* number of supported entries */
#define ACI_LEN_LAN_LP_FLD (ACI_LEN_LAN_LP_NTRY*ACI_MAX_LAN_LP_NTRY)

#define SIM_ALS_LOCK    (SIM_FDN_DISABLE+1) /* To distinguish between the different operations */
#define SIM_ALS_UNLOCK  (SIM_FDN_DISABLE+2) /* which require a CHV2 authentication */

#ifdef FF_CPHS_REL4
#define ACI_SIZE_EF_CFIS         16
#define ACI_SIZE_EF_MBDN         34
#define ACI_SIZE_EF_MBI           4
#define ACI_SIZE_EF_MWIS          5
#endif /* FF_CPHS_REL4 */

typedef enum 
{
  EF_STAT_UNKNWN = 0,       /* status of EF unknown */
  EF_STAT_READ,             /* EF already read */
  EF_STAT_NOT_AVAIL         /* EF is not available */ 
} EF_STAT;

typedef enum
{
  CPOL_ACT_None = 0,        /* no action */
  CPOL_ACT_Write,           /* write entry */
  CPOL_ACT_Delete,          /* delete entry */
  CPOL_ACT_Read,            /* read entry */
  CPOL_ACT_Test             /* test number of entries */
} ACI_CPOL_ACT;
 
typedef enum
{
  CLAN_ACT_None = 0,        /* no action */
  CLAN_ACT_Write,           /* write entry */
  CLAN_ACT_Read             /* read entry */
} ACI_CLAN_ACT;

/*==== TYPES ======================================================*/

/*==== PROTOTYPES =================================================*/

EXTERN void            cmhSIM_SIMActivated       ( void );
EXTERN void            cmhSIM_PINVerified        ( void );
EXTERN void            cmhSIM_SIMSync            ( void );
EXTERN void            cmhSIM_SIMInserted        ( void );
EXTERN void            cmhSIM_SIMRemoved         ( void );
#ifdef FF_DUAL_SIM
EXTERN void            cmhSIM_SIMSelected        ( void );
#endif /*FF_DUAL_SIM*/
EXTERN void            cmhSIM_FillInPIN          ( CHAR*     PINStr,
                                                   CHAR*     PINFld, 
                                                   UBYTE     len      );
EXTERN void            cmhSIM_GetHomePLMN        ( SHORT*    mccBuf,
                                                   SHORT*    mncBuf   );
GLOBAL BOOL            cmhSIM_plmn_equal_sim     ( SHORT     bcch_mcc,
                                                   SHORT     bcch_mnc, 
                                                   SHORT     sim_mcc, 
                                                   SHORT     sim_mnc);
EXTERN BOOL            cmhSIM_plmn_is_hplmn      ( SHORT     bcch_mcc, 
                                                   SHORT     bcch_mnc);
EXTERN T_ACI_RETURN    cmhSIM_ReqMsisdn          ( T_ACI_CMD_SRC srcId,
                                                   UBYTE     record   );
EXTERN void            cmhSIM_CnfMsisdn          ( SHORT     table_id );
EXTERN T_ACI_RETURN    cmhSIM_ReqCcp             ( T_ACI_CMD_SRC srcId,
                                                   UBYTE     record   );
EXTERN void            cmhSIM_CnfCcp             ( SHORT     table_id );
EXTERN void            cmhSIM_RdCnfPlmnSel       ( SHORT     table_id );
EXTERN void            cmhSIM_WrCnfPlmnSel       ( SHORT     table_id );
EXTERN T_ACI_CME_ERR   cmhSIM_GetCmeFromSim      ( USHORT    errCode  );
EXTERN SHORT           cmhSIM_FillPlmnSelList    ( UBYTE              index, 
                                                   T_ACI_CPOL_FRMT    frmt,
                                                   T_ACI_CPOL_OPDESC* operLst,
                                                   UBYTE              length,
                                                   UBYTE*             pData );

EXTERN T_ACI_RETURN     getSupLangFromPCM       ( T_ACI_LAN_SUP *lanlst, 
                                                  SHORT         *lastIdx  );

EXTERN BOOL           checkSuppLang             (T_ACI_LAN_SUP  *lanlst,
                                                  SHORT          lastIdx, 
                                                  T_ACI_LAN_SUP  *clng);

EXTERN BOOL           checkSuppLangInLP         ( T_ACI_LAN_SUP  *lanlst,
                                                  SHORT          lastIdx, 
                                                  T_ACI_LAN_SUP  *clng);
                                                   
EXTERN SHORT           cmhSIM_UsdPlmnSelNtry     ( UBYTE              length,
                                                   UBYTE*             pData  );
EXTERN void            cmhSIM_CmpctPlmnSel       ( UBYTE              length,
                                                   UBYTE*             pData );
EXTERN T_ACI_BS_SPEED  cmhSIM_GetUserRate        ( UBYTE  userRate );
EXTERN T_ACI_CNUM_ITC  cmhSIM_GetItc             ( UBYTE  itc      );
EXTERN T_ACI_CNUM_SERV cmhSIM_GetSrvFromSync     ( UBYTE  sync     );
EXTERN T_ACI_CNUM_SERV cmhSIM_GetSrvFromItc      ( UBYTE  itc      );
EXTERN void            cmhSIM_getMncMccFrmPLMNsel( const UBYTE* ntry, 
                                                         SHORT* mcc, 
                                                         SHORT* mnc );
EXTERN BOOL            cmhSIM_GetCodedPLMN       ( const CHAR *oper, 
                                                   T_ACI_CPOL_FRMT format, 
                                                   UBYTE *sim_plmn );
EXTERN T_ACI_RETURN    cmhSIM_DelPlmnSel         ( T_ACI_CMD_SRC srcId,
                                                   SHORT index,
                                                   T_ACI_CPOL_MOD mode );
EXTERN T_ACI_RETURN    cmhSIM_FndEmptyPlmnSel    ( T_ACI_CMD_SRC srcId,
                                                   UBYTE *plmn );
EXTERN T_ACI_RETURN    cmhSIM_UpdPlmnSel         ( T_ACI_CMD_SRC srcId,
                                                   SHORT index,
                                                   UBYTE *plmn,
                                                   T_ACI_CPOL_MOD mode );
EXTERN T_ACI_RETURN    cmhSIM_ChgPlmnSel         ( T_ACI_CMD_SRC srcId,
                                                   SHORT index,
                                                   SHORT index2 );
EXTERN void            cmhSIM_SIMResponseData    ( T_SIM_TRNS_RSP_PRM* rsp );

EXTERN void            cmhSIM_RdCnfLangELP        ( SHORT     table_id );


EXTERN void            cmhSIM_RdCnfLangPrfELP      (SHORT     table_id);

EXTERN void            cmhSIM_RdCnfLangLP           ( SHORT     table_id );


EXTERN void            cmhSIM_RdCnfLangPrfLP        (SHORT     table_id);

EXTERN BOOL            cmhSIM_AD_Update             ( int, T_SIM_FILE_UPDATE_IND *);
EXTERN BOOL            cmhSIM_CSP_Update             ( int, T_SIM_FILE_UPDATE_IND *);
EXTERN BOOL            cmhSIM_ONS_Update             ( int, T_SIM_FILE_UPDATE_IND *);
EXTERN void cmhSIM_Read_AD_cb(SHORT table_id);
EXTERN void cmhSIM_Read_AD_cb(SHORT table_id);
EXTERN T_opl_field* cmhSIM_GetOPL();
EXTERN void cmhSIM_GetSrvTab( UBYTE*  ptrSrvTab );
EXTERN void cmhSIM_Get_CSP_cb(SHORT table_id);
EXTERN void cmhSIM_Read_CSP_cb(SHORT table_id);

#ifdef FF_CPHS_REL4
EXTERN void            cmhSIM_RdCnfCfis( SHORT table_id );
EXTERN void            cmhSIM_WrCnfCfis( SHORT table_id );
EXTERN T_ACI_RETURN    cmhSIM_WrCfis (T_ACI_CMD_SRC srcId,
                                      T_ACI_CFIS_MOD mode, UBYTE index,
                                      UBYTE mspId,UBYTE cfuStat,CHAR *number, 
                                      T_ACI_TOA *type,UBYTE cc2_id);
EXTERN void cmhSIM_RdCnfMwis( SHORT table_id );
EXTERN void cmhSIM_WrCnfMwis( SHORT table_id );
EXTERN T_ACI_RETURN cmhSIM_WrMwis (T_ACI_CMD_SRC srcId,
                                   T_ACI_MWIS_MOD mode,
                                   UBYTE mspId,
                                   T_ACI_MWIS_MWI *mwis);
EXTERN T_ACI_RETURN cmhSIM_WrMbdn (T_ACI_CMD_SRC srcId,
                                   T_ACI_MBN_MODE mode, UBYTE index,
                                   CHAR* number, T_ACI_TOA* type,
                                   UBYTE cc2_id, T_ACI_PB_TEXT *text);
EXTERN void cmhSIM_WrCnfMbdn ( SHORT table_id );
EXTERN void cmhSIM_RdCnfMbi ( SHORT table_id );
EXTERN void cmhSIM_RdCnfMbdn ( SHORT table_id );
#endif /* FF_CPHS_REL4 */

#ifdef SIM_PERS_OTA
EXTERN BOOL         cmhSIM_Register_Read_DCK     ( int, T_SIM_FILE_UPDATE_IND *);
EXTERN void cmhSIM_Read_DCK_cb(SHORT table_id);
EXTERN void cmhSIM_Read_DCK_init_cb(SHORT table_id);
#endif

#ifdef SIM_PERS
EXTERN void cmhSIM_UnlockTimeout(void);
#endif

/* EONS */
EXTERN void cmhSIM_OpReadOplRcdCb(SHORT table_id);
EXTERN void cmhSIM_OpReadPnnRcdCb(SHORT table_id);
EXTERN BOOL cmhSIM_OpUpdate(int ref, T_SIM_FILE_UPDATE_IND*  fu);
EXTERN void cmhSIM_InitOperatorName();
EXTERN BOOL cmhSIM_StartOperatorName();
EXTERN BOOL cmhSIM_UpdateOperatorName(USHORT reqDataFld);

EXTERN void cmhSIM_AD_Updated(UBYTE ad_len, UBYTE* ad_data);

#ifdef DTI
EXTERN BOOL SIM_ENT_CSDconnect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type);
#endif
#if defined (GPRS) AND defined (DTI)
EXTERN BOOL SIM_SNDCP_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type);
EXTERN BOOL SIM_ENT_GPRSconnect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type);
#endif

EXTERN T_ACI_RETURN cmhSIM_Req_or_Write_PlmnSel ( T_ACI_CMD_SRC srcId, 
                                                  T_SIM_ACTP accType );
EXTERN void cmhSIM_CardUnblocked_PINChanged ( BOOL PINChanged );
EXTERN void cmhSIM_PINEnabledDisabled ( void );
EXTERN BOOL cmhSIM_ReqLanguagePrf_LP_or_ELP  ( USHORT reqDataFld,
                                               UBYTE recMax,
                                               UBYTE *exchData,
                                               void (*rplyCB)(SHORT) );
EXTERN T_ACI_RETURN cmhSIM_ReqLanguage_LP_or_ELP  ( T_ACI_CMD_SRC srcId,
                                                    USHORT reqDataFld );

/*==== EXPORT =====================================================*/

#ifdef CMH_SIMF_C

GLOBAL T_ENT_STAT     simEntStat;

GLOBAL T_ACI_CFUN_FUN CFUNfun         = CFUN_FUN_Minimum;

GLOBAL UBYTE          CNUMMaxRec      = 0;
GLOBAL T_ACI_CNUM_LST CNUMMsisdn;
GLOBAL UBYTE          CNUMMsisdnIdx   = 0;
GLOBAL UBYTE          CNUMLenEfMsisdn = 0;
GLOBAL UBYTE          EfPLMNselStat   = EF_STAT_UNKNWN;
GLOBAL UBYTE          CPOLSimEfDataLen;
GLOBAL UBYTE          CPOLSimEfData[ACI_LEN_PLMN_SEL_FLD];

GLOBAL UBYTE          CLANimEfDataLen;
GLOBAL UBYTE          CLANSimEfData[ACI_LEN_LAN_FLD];
GLOBAL UBYTE          CLANSimEfDataLP[ACI_LEN_LAN_LP_FLD];
#ifdef FF_CPHS_REL4
GLOBAL UBYTE          CFISIndex       = 0; 
GLOBAL UBYTE          MWISIndex       = 0;
GLOBAL UBYTE          MBI_Index       = 0;
#endif
#else

EXTERN T_ENT_STAT     simEntStat;

EXTERN T_ACI_CFUN_FUN CFUNfun;

EXTERN UBYTE          CNUMMaxRec;
EXTERN T_ACI_CNUM_LST CNUMMsisdn;
EXTERN UBYTE          CNUMMsisdnIdx;
EXTERN UBYTE          CNUMLenEfMsisdn;
EXTERN UBYTE          EfPLMNselStat;
EXTERN UBYTE          CPOLSimEfDataLen;
EXTERN UBYTE          CPOLSimEfData[];

EXTERN UBYTE          CLANimEfDataLen;
EXTERN UBYTE          CLANSimEfData[];
EXTERN UBYTE          CLANSimEfDataLP[];
#ifdef FF_CPHS_REL4
EXTERN UBYTE          CFISIndex ;
EXTERN UBYTE          MWISIndex;
EXTERN UBYTE          MBI_Index;
#endif
#endif /* CMH_SIMF_C */

#endif /* CMH_SIM_H */

/*==== EOF =======================================================*/

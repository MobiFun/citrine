/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
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
|  Purpose :  Definitions for the AT Command Handler
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_H
#define CMH_H

/*==== CONSTANTS ==================================================*/
typedef enum
{
  RAT_OK  = 0,
  RAT_CONNECT,
  RAT_CME,
  RAT_NO_CARRIER,
  RAT_CRING,
  RAT_DR,
  RAT_CR,
  RAT_CPIN,
  RAT_COPS,
  RAT_CREG,
  RAT_CCWA,
  RAT_CLIP,
  RAT_CDIP,  
  RAT_COLP,
  RAT_CRING_OFF,

#ifdef FF_FAX
  RAT_FCO,
  RAT_FIS,
  RAT_FTI,
  RAT_FCS,
  RAT_FCI,
#endif

  RAT_CMS,
  RAT_CSMS,
  RAT_CMGS,
  RAT_CMSS,
  RAT_CMGW,
  RAT_CDS,
  RAT_CMGC,
  RAT_CMGD,
  RAT_CMGR,
  RAT_CMGL,
  RAT_CMTI,
  RAT_CMT,
  RAT_CBM,
  RAT_CPMS,

#ifdef FF_FAX
  RAT_FHT,
  RAT_FHR,
  RAT_FSA,
  RAT_FPA,
  RAT_FPW,
  RAT_FET,
  RAT_FVO,
  RAT_FPO,
  RAT_FPI,
  RAT_FNF,
  RAT_FNS,
  RAT_FNC,
  RAT_FHS,
  RAT_FPS,
  RAT_FTC,
#endif

  RAT_ILRR,
  RAT_BUSY,
  RAT_NO_ANSWER,
  RAT_SIMREM,
  RAT_CLIR,
  RAT_COLR,
  RAT_CSSI,
  RAT_CSSU,
  RAT_CUSD,
  RAT_CCFC,
  RAT_CLCK,
  RAT_CIMI,
  RAT_SATI,
  RAT_SATE,
  RAT_KSIR,
  RAT_CPI,
  RAT_CTYI,
  RAT_CNUM,
  RAT_CPOL,
  RAT_CCCM,
  RAT_CTV,
  RAT_SATN,
  RAT_SATA,
  RAT_SMS_READY,
  RAT_PHB_STATUS,
  RAT_SIMINS,
  RAT_CRSM,
  RAT_CSIM,
  RAT_CCBS,
  RAT_CCWV,
  RAT_CNAP,
  RAT_SIG_SMS,
  RAT_CLAN,
  RAT_CLAE,
  RAT_CSQ,
  RAT_ALS,
  RAT_CTZV,
  RAT_P_CREG,
#ifdef REL99
  RAT_P_CMGRS,
#endif /* REL99 */
#ifdef GPRS
  RAT_CGACT,
  RAT_CGDATA,
  RAT_CGANS,
  RAT_CGEREP,
  RAT_CGREG,
  RAT_QOS_MOD,
  RAT_SNCNT,
  RAT_P_CGREG,
#ifdef REL99
  RAT_CGCMOD,
#endif
  RAT_P_CGEV,
#endif  /* GPRS */

#ifdef FF_EM_MODE
  RAT_EM,
  RAT_EMET,
  RAT_EMETS,
#endif /* FF_EM_MODE */

#ifdef FF_CPHS
  RAT_CPNUMS,
  RAT_CPVWI,
  RAT_CPROAM,
#endif /* FF_CPHS */

  RAT_CIEV,
  RAT_RDL,
#ifdef TI_PS_FF_AT_P_CMD_RDLB
  RAT_RDLB,
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
  RAT_CCCN,
  RAT_CSSN,

  RAT_CSTAT,
  RAT_Z,
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
  RAT_CPRSM,
#endif /*TI_PS_FF_AT_P_CMD_CPRSM*/
  RAT_P_CTZV,
  RAT_P_CPRI,
  RAT_P_SIMEF,
  RAT_P_CNIV,
  RAT_P_COPS,
  RAT_P_CMGR,
  RAT_P_CMGL,
#ifdef FF_CPHS_REL4
  RAT_P_CFIS,
  RAT_P_MWIS,
  RAT_P_MWI,
  RAT_P_MBI,
  RAT_P_MBDN,
#endif /* FF_CPHS_REL4 */
  RAT_MAX

} RAT_ID;

/*==== TYPES ======================================================*/
typedef struct entStatus          /* entity status */
{
  T_ACI_CMD_SRC  entOwn;          /* entity owner */
  T_ACI_AT_CMD   curCmd;          /* current command processing */
} T_ENT_STAT;

typedef struct mmCmdPrm          /* command parameters related to MM */
{
  T_ACI_COPS_FRMT COPSfrmt;
  T_ACI_NRG_RGMD  NRGregMode;
  T_ACI_NRG_SVMD  NRGsrvMode;
  T_ACI_NRG_FRMT  NRGoprFrmt;
  T_ACI_CSQ_MODE  CSQworkStat;
  T_ACI_CTZR_MODE CTZRMode;
  T_ACI_CTZU_MODE CTZUMode;
  T_ACI_PCTZV_MODE PCTZVMode;
  T_ACI_CNIV_MODE CNIVMode;
  T_ACI_IND_MODE  sIndicationParam;
} T_MM_CMD_PRM;

typedef struct ccCmdPrm          /* command parameters related to CC */
{
  T_ACI_CLIR_MOD  CLIRmode;
  T_ACI_DCD_MOD   DCDmode;
  T_ACI_TOA       CSTAtoa;
  BOOL            CSTAdef;
  T_ACI_CHLD_MOD  CHLDmode;
  T_ACI_CCUG_IDX  CCUGidx;
  T_ACI_CCUG_INFO CCUGinfo;
  T_ACI_CCUG_MOD  CCUGmode;
  USHORT           mltyCncFlg;
  USHORT           mltyDscFlg;
  T_ACI_ALS_MOD   ALSmode;      /* mode for Alternate Line Service */
  T_ACI_CTTY_MOD  CTTYmode;     /* TTY Service notification */
  T_ACI_CC_CSCN_MOD CSCNcc_mode;
} T_CC_CMD_PRM;

typedef struct simCmdPrm      /* command parameters related to SIM */
{
  UBYTE             CNUMActRec;
  UBYTE             CNUMOutput;
  T_ACI_CPOL_FRMT   CPOLfrmt;
  UBYTE             CPOLact;
  UBYTE             CPOLidx;
  UBYTE             CPOLidx2;
  T_ACI_CPOL_MOD    CPOLmode;
  UBYTE             CPOLplmn[3];
  UBYTE             CLANact;
} T_SIM_CMD_PRM;

typedef struct ssCmdPrm       /* command parameters related to SS */
{
  UBYTE           CXXXpwd[MAX_PWD_NUM+1];
  UBYTE           CXXXnewPwd[MAX_PWD_NUM+1];
  UBYTE           CXXXnewPwd2[MAX_PWD_NUM+1];
  USHORT           mltyTrnFlg;
  T_ACI_SS_CSCN_MOD CSCNss_mode;
} T_SS_CMD_PRM;

typedef struct phbCmdPrm      /* command parameters related to PHB */
{
  T_ACI_PB_STOR   cmhStor;
  UBYTE           phbStor;      /* T_PHB_TYPE */
  USHORT          order_num;    /* Next matching entry, 0 if none */
  SHORT           fndRec;
  UBYTE           wrtRec;
  T_ACI_CSVM_MOD  CSVMmode;
  T_ACI_CLAE_MOD  CLAEmode;
  UBYTE           curCmd;
  UBYTE           temp_cmhStor; /* These data types are used to hold */
  UBYTE           temp_phbStor; /* the phone book storage types temprorily, in
                                   case of SIM PIN-2 verification. These values
                                   are assigned to the actual storage fileds
                                   when SIM PIN-2 verification confirmation
                                   comes without any ERROR. */
} T_PHB_CMD_PRM;

/* Fax and data related command parameters have been moved to a specific
structure in cmh_ra.h */
typedef struct cmhCmdPrm      /* handler command parameter */
{
  T_MM_CMD_PRM  mmCmdPrm;
  T_CC_CMD_PRM  ccCmdPrm;
  T_SIM_CMD_PRM simCmdPrm;
  T_SS_CMD_PRM  ssCmdPrm;
  T_PHB_CMD_PRM phbCmdPrm;

} T_CMH_PRM;

/*==== PROTOTYPES =================================================*/
EXTERN void       cmh_Init            ( void );
EXTERN void       cmh_Reset           ( T_ACI_CMD_SRC srcId, BOOL atz );
EXTERN BOOL       cmh_IsVldCmdSrc     ( T_ACI_CMD_SRC cmdSrc );
EXTERN UBYTE      cmh_mergeTOA        ( UBYTE ton, UBYTE npi );
EXTERN UBYTE      cmh_mergeTOS        ( UBYTE tos, UBYTE oe );
EXTERN void       cmh_demergeTOA      ( UBYTE toa, UBYTE* ton, UBYTE* npi );
EXTERN void       cmh_demergeTOS      ( UBYTE tosa, UBYTE* tos, UBYTE* oe );
EXTERN CHAR*      cmh_setToaDef       ( CHAR* number, T_ACI_TOA *toa );
EXTERN USHORT     cmh_packBCD         ( UBYTE *bcd_out, const UBYTE *char_in,
                                        USHORT len_in);
EXTERN USHORT     cmh_unpackBCD       ( UBYTE *char_out, const UBYTE *bcd_in,
                                        USHORT len_in);
EXTERN void       cmh_unpackSCTS      ( T_ACI_VP_ABS *scts,
                                        const UBYTE *buf_in );
EXTERN void       cmh_logRslt         ( T_ACI_CMD_SRC dest, RAT_ID rat,
                                        T_ACI_AT_CMD cmd,   SHORT cId,
                                        T_ACI_BS_SPEED spd, T_ACI_CME_ERR err );
EXTERN void       cmh_cvtToDefGsm     ( CHAR*   in,
                                        CHAR*   out,
                                        USHORT* len );
EXTERN void       cmh_cvtFromDefGsm   ( CHAR*   in,
                                        USHORT  len,
                                        CHAR*   out );
EXTERN UBYTE      cmh_set_delayed_call( UCHAR (*call) (void*), void* arg );
EXTERN UBYTE      cmh_start_delayed_call ( ULONG ms );
EXTERN UBYTE      cmh_timeout         ( USHORT handle );
EXTERN SHORT      cmh_bldCalPrms      ( char * pDialStr, T_CLPTY_PRM * calPrm );
EXTERN UBYTE      qAT_CallActive        ( void );
EXTERN void        cmhSMS_disableAccess ( void );
EXTERN T_ACI_RETURN cmhSIM_ReadTranspEF ( T_ACI_CMD_SRC srcId,
                                          T_ACI_AT_CMD  cmd,
                                          BOOL          v_path_info,
                                          T_path_info   *path_info_ptr,
                                          USHORT        datafield,
                                          USHORT        offset,
                                          UBYTE         explen,
                                          UBYTE       * exchData,
                                          void      (*rplyCB)(SHORT));
EXTERN T_ACI_RETURN cmhSIM_WriteTranspEF (T_ACI_CMD_SRC srcId,
                                          T_ACI_AT_CMD  cmd,
                                          BOOL          v_path_info,
                                          T_path_info   *path_info_ptr,
                                          USHORT        datafield,
                                          USHORT        offset,
                                          UBYTE         datalen,
                                          UBYTE       * exchData,
                                          void      (*rplyCB)(SHORT));
#ifdef GPRS
GLOBAL void cp_pdp_primitive(T_SMREG_PDP_ACTIVATE_CNF * pdp_activate_cnf,
                             T_PPP_PDP_ACTIVATE_RES *activate_result);
#endif
GLOBAL T_ACI_RETURN cmhSIM_WriteRecordEF (T_ACI_CMD_SRC srcId,
                                          T_ACI_AT_CMD  cmd,
                                          BOOL          v_path_info,
                                          T_path_info   *path_info_ptr,
                                          USHORT        datafield,
                                          UBYTE         record,
                                          UBYTE         datalen,
                                          UBYTE       * exchData,
                                          void      (*rplyCB)(SHORT));
GLOBAL T_ACI_RETURN cmhSIM_ReadRecordEF ( T_ACI_CMD_SRC srcId,
                                          T_ACI_AT_CMD  cmd,
                                          BOOL          v_path_info,
                                          T_path_info   *path_info_ptr,
                                          USHORT        datafield,
                                          UBYTE         record,
                                          UBYTE         explen,
                                          UBYTE       * exchData,
                                          void      (*rplyCB)(SHORT));
GLOBAL T_ACI_RETURN cmhSMS_ReadParams (T_ACI_CMD_SRC  srcId,
                                       T_ACI_AT_CMD   cmd,
                                       SHORT          recNr);
GLOBAL T_ACI_RETURN cmhSMS_WriteParams (T_ACI_CMD_SRC  srcId,
                                        T_ACI_AT_CMD   cmd,
                                        SHORT          recNr);
GLOBAL T_ACI_RETURN cmhSIM_GetSIMError ( T_ACI_CMD_SRC srcBuf,
                                         T_ACI_AT_CMD cmdBuf  );
GLOBAL T_ACI_RETURN cmhSIM_CheckSimPinStatus ( T_ACI_CMD_SRC srcBuf,
                                               T_ACI_AT_CMD cmdBuf );
GLOBAL BOOL cmhSIM_CheckSimStatus();
GLOBAL T_ACI_RETURN cmhSIM_Read_AD     ( );

EXTERN void percentCSTAT_indication(T_ACI_ENTITY_ID_MSG     entityId, 
                                    T_ACI_ENTITY_STATE_MSG  entityState);

EXTERN void cmhSMS_ReadCbDtaDwnl ( T_SIM_MMI_INSERT_IND *sim_mmi_insert_ind );

GLOBAL void cmhSIM_Get_CSP();
GLOBAL void cmhSIM_Read_CSP();
#ifdef SIM_PERS
GLOBAL void cmhSIM_WriteDefaultValue_DCK();
#endif

/* Implements Measure#32: Row 118, 119, 980, 986, 987, 1036 & 1037 */
EXTERN void cmhMM_mcc_mnc_print( CHAR           *oper,
                                 SHORT          mcc,
                                 SHORT          mnc);
EXTERN void cmhSMS_sdu_buf_print ( U8     *buf,
                                   USHORT offset );

GLOBAL UBYTE cmhSIM_isplmnmodebit_set();

EXTERN BOOL aci_get_cb_cmd (RAT_ID cmd, T_ACI_CMD_SRC src);

GLOBAL T_ACI_CMD_MODE aci_cmd_src_mode_get(T_ACI_CMD_SRC src_id);
GLOBAL void aci_cmd_src_mode_set(UBYTE src_id,T_ACI_CMD_MODE mode);

/*==== MACROS =====================================================*/

EXTERN T_VOID_FUNC rat_fptr; /* new global function pointer */
EXTERN UBYTE srcId_cb;
EXTERN USHORT used_sources;

#define IS_SRC_USED(x)   ((used_sources >> x) & 0x01)
#define IS_SRC_BT(x)   ((used_sources >> (x + 8)) & 0x01)


/* we set the src id only when the src id NEQ local. Bec in ASC test we intentionally 
   set the src from TST to LCL for some purpose, but the srcId_cb should remain TST */

/* new macro, calling function. function is defined in cmh_f.c, prototype above */
#define R_AT(cmd,src)\
  if (aci_get_cb_cmd (cmd, src)) \
    (*rat_fptr)

#define ACI_ERR_DESC( errCls, errNr )\
  {aciErrDesc=(((errCls)<<16) | ((errNr)&0xFFFF));}

/*==== EXPORT =====================================================*/
#ifdef CMH_F_C

GLOBAL T_CMH_PRM      cmhPrm[OWN_SRC_MAX];
GLOBAL T_ACI_ALS_MOD  ALSlock = ALS_MOD_NOTPRESENT;
GLOBAL T_ACI_ERR_DESC aciErrDesc = (ULONG)-1; 

#else

EXTERN T_CMH_PRM      cmhPrm[OWN_SRC_MAX];
EXTERN T_ACI_ALS_MOD  ALSlock;
EXTERN T_ACI_ERR_DESC aciErrDesc;

EXTERN const T_VOID_FUNC RATJmpTbl[RAT_MAX][CMD_MODE_MAX];  /* SMI/MFW & ATI */

#endif  /* of #ifndef CMH_F_C */

/* Implements Measure 25 */
EXTERN UBYTE cmh_getAlphabetCb ( UBYTE dcs );

/* Implements Measure#32: Row 105, 1115 & 1151 */
EXTERN const char * const ffff_str;
/* Implements Measure#32: Row 110 & 981  */
EXTERN const char * const ef_plmn_id;
/* Implements Measure#32: Row 971, 976, 1023 & 1072 */
EXTERN const char * const ef_clng_id;
/* Implements Measure#32: Row 60 & 1039  */
EXTERN const char * const ef_mssup_id;
/* Implements Measure#32: Row 119, 980, 987, 1037 */
EXTERN const char * const format_03x02x_str;
/* Implements Measure#32: Row 118, 986, 1036 */
EXTERN const char * const format_03x03x_str;
/* Implements Measure#32: Row 969, 1027, 1239 & 1240 */
EXTERN const char * const num_112_str;
EXTERN const char * const num_911_str;
/* Implements Measure#32: Row 972, 1024 & 1041 */
EXTERN char * const au_str;
/* Implements Measure#32: Row 1245, 1277 & 1312 */
EXTERN const char * const format_2X_str;
/* Implements Measure#32: Row 73, 74, 103, 114 & 1071 */
EXTERN const char * const gsm_com_path;
EXTERN const char * const gsm_com_redialmode_path;
EXTERN const char * const gsm_com_alslock_path;
EXTERN const char * const gsm_com_rfcap_path;

#endif  /* CMH_H */

/*==== EOF =======================================================*/

#ifndef ACI_BAT_CB_H
#define ACI_BAT_CB_H


#ifdef  ACI_BAT_CB_GLOBALS
#define ACI_BAT_CB_EXT         /* for the owner of a global */
#else
#define ACI_BAT_CB_EXT extern  /* for the user of a global  */
#endif

#include "aci_cmh.h"
#ifdef GPRS
#include "gaci_cmh.h"
#endif /* GPRS */

#ifdef FF_SAT_E
ACI_BAT_CB_EXT T_ACI_SATA_ADD Addparm;   /* ATI maintenance for %SAT responses - Not supported by BAT */
#endif


GLOBAL void rBAT_OK(
  T_ACI_AT_CMD cmd);

GLOBAL void rBAT_CONNECT(
  T_ACI_AT_CMD cmdId,
  T_ACI_BS_SPEED speed,
  SHORT cId,
  BOOL flow_cntr);

GLOBAL void rBAT_PlusCME(
  T_ACI_AT_CMD cmdId,
  T_ACI_CME_ERR err);

GLOBAL void rBAT_NO_CARRIER(
  T_ACI_AT_CMD cmdId,
  SHORT cId);

GLOBAL void rBAT_PlusCRING(
  T_ACI_CRING_MOD mode,
  T_ACI_CRING_SERV_TYP type1,
  T_ACI_CRING_SERV_TYP type2);

GLOBAL void rBAT_PlusDR(
  T_ACI_DR_TYP type);

GLOBAL void rBAT_PlusCR(
  T_ACI_CRING_SERV_TYP service);

GLOBAL void rBAT_PlusCPIN(
  T_ACI_CPIN_RSLT code);

GLOBAL void rBAT_PlusCOPS(
  SHORT lastIdx,
  T_ACI_COPS_OPDESC *operLst);

GLOBAL void rBAT_PercentCOPS(
  SHORT lastIdx,
  T_ACI_COPS_OPDESC *operLst);

GLOBAL void rBAT_PlusCREG(
  T_ACI_CREG_STAT status,
  USHORT lac,
  USHORT cid);

GLOBAL void rBAT_PlusCCWA(
  T_ACI_CLSSTAT *clsStat,
  CHAR *number,
  T_ACI_TOA *type,
  U8 validity,
  T_ACI_CLASS class_type,
#ifdef NO_ASCIIZ
  T_ACI_PB_TEXT *alpha);
#else
  CHAR *alpha);
#endif

GLOBAL void rBAT_PlusCLIP(
  T_ACI_CLIP_STAT stat,
  CHAR *number,
  T_ACI_TOA * type,
  U8 validity,
  CHAR *subaddr,
  T_ACI_TOS *satype,
#ifdef NO_ASCIIZ
  T_ACI_PB_TEXT *alpha);
#else
  CHAR *alpha);
#endif /*NO_ASCIIZ*/

EXTERN void rBAT_PlusCOLP(
  T_ACI_COLP_STAT stat,
  CHAR *number,
  T_ACI_TOA *type,
  CHAR *subaddr,
  T_ACI_TOS *satype,
#ifdef NO_ASCIIZ
  T_ACI_PB_TEXT *alpha);
#else
  CHAR *alpha);
#endif /*NO_ASCIIZ*/

GLOBAL void rBAT_PlusCRING_OFF(
    SHORT cId);

GLOBAL void rBAT_PlusFCO         (/* add necessary parameters */);
GLOBAL void rBAT_PlusFIS         (/* add necessary parameters */);
GLOBAL void rBAT_PlusFTI         (/* add necessary parameters */);
GLOBAL void rBAT_PlusFCS         (/* add necessary parameters */);
GLOBAL void rBAT_PlusFCI         (/* add necessary parameters */);

GLOBAL void rBAT_PlusCMS(
  T_ACI_AT_CMD cmdId,
  T_ACI_CMS_ERR err,
  T_EXT_CMS_ERROR *conc_error);

GLOBAL void rBAT_PlusCSMS(
  T_ACI_CSMS_SERV service,
  T_ACI_CSMS_SUPP mt,
  T_ACI_CSMS_SUPP mo,
  T_ACI_CSMS_SUPP bm);

GLOBAL void rBAT_PlusCMGS(
  T_MNSMS_SUBMIT_CNF *mnsms_submit_cnf);

GLOBAL void rBAT_PlusCMSS(
  T_MNSMS_SUBMIT_CNF *mnsms_submit_cnf);

GLOBAL void rBAT_PlusCMGW(
  UBYTE index,
  UBYTE numSeg);

GLOBAL void rBAT_PlusCDS(
  T_MNSMS_STATUS_IND *mnsms_status_ind);

GLOBAL void rBAT_PlusCMGC(
  T_MNSMS_COMMAND_CNF *mnsms_command_cnf);

GLOBAL void rBAT_PlusCMGD(void);

GLOBAL void rBAT_PlusCMGR(
  T_MNSMS_READ_CNF *mnsms_read_cnf,
  T_ACI_CMGR_CBM *cbm);

GLOBAL void rBAT_PercentCMGR(
  T_MNSMS_READ_CNF *mnsms_read_cnf,
  T_ACI_CMGR_CBM *cbm);

GLOBAL void rBAT_PlusCMGL(
  T_MNSMS_READ_CNF *mnsms_read_cnf);

GLOBAL void rBAT_PercentCMGL(
  T_MNSMS_READ_CNF *mnsms_read_cnf);

GLOBAL void rBAT_PlusCMTI(
  T_ACI_SMS_STOR mem,
  UBYTE index);

GLOBAL void rBAT_PlusCMT(
  T_MNSMS_MESSAGE_IND *msg);

GLOBAL void rBAT_PlusCBM(
  T_MMI_CBCH_IND *mmi_cbch_ind);

GLOBAL void rBAT_PlusCPMS(
  T_ACI_SMS_STOR_OCC *mem1,
  T_ACI_SMS_STOR_OCC *mem2,
  T_ACI_SMS_STOR_OCC *mem3);

GLOBAL void rBAT_PlusFHT         (U16 len, U8 *hdlc);
GLOBAL void rBAT_PlusFHR         (U16 len, U8 *hdlc);
GLOBAL void rBAT_PlusFSA         (U8 c_sub_str, U8 *sub_str);
GLOBAL void rBAT_PlusFPA         (U8 c_spa_str, U8 *spa_str);
GLOBAL void rBAT_PlusFPW         (U8 c_pw_str, U8 *pw_str);
GLOBAL void rBAT_PlusFET         (/* add necessary parameters */);
GLOBAL void rBAT_PlusFVO         (/* add necessary parameters */);
GLOBAL void rBAT_PlusFPO         (/* add necessary parameters */);
GLOBAL void rBAT_PlusFPI         (U8 c_id_str, U8 *id_str);
GLOBAL void rBAT_PlusFNF         (U16 len, U8 *nsf);
GLOBAL void rBAT_PlusFNS         (U8 c_nsf, U8 *nsf);
GLOBAL void rBAT_PlusFNC         (U16 len, U8 *nsc);
GLOBAL void rBAT_PlusFHS         (/* add necessary parameters */);
GLOBAL void rBAT_PlusFPS         (/* add necessary parameters */);
GLOBAL void rBAT_PlusFTC         (/* add necessary parameters */);

GLOBAL void rBAT_PlusILRR(
  T_ACI_BS_SPEED speed,
  T_ACI_BS_FRM format,
  T_ACI_BS_PAR parity);

GLOBAL void rBAT_BUSY(
  T_ACI_AT_CMD cmdId,
  SHORT cId);

GLOBAL void rBAT_NO_ANSWER(
  T_ACI_AT_CMD cmdId,
  SHORT cId);

GLOBAL void rBAT_PercentSIMREM(
  T_ACI_SIMREM_TYPE srType);

GLOBAL void rBAT_PlusCLIR(
  T_ACI_CLIR_MOD mode,
  T_ACI_CLIR_STAT stat);

GLOBAL void rBAT_PercentCOLR(
  T_ACI_COLR_STAT stat);

GLOBAL void rBAT_PlusCSSI(
  T_ACI_CSSI_CODE code,
  SHORT index);

GLOBAL void rBAT_PlusCSSU(
  T_ACI_CSSU_CODE code,
  SHORT index,
  CHAR *number,
  T_ACI_TOA *type,
  CHAR *subaddr,
  T_ACI_TOS *satype);

GLOBAL void rBAT_PlusCUSD(
  T_ACI_CUSD_MOD m,
  T_ACI_USSD_DATA *ussd,
  SHORT dcs);

GLOBAL void rBAT_PlusCCFC (T_ACI_CCFC_SET* setting);

GLOBAL void rBAT_PlusCLCK(
  T_ACI_CLSSTAT *clsStat);

GLOBAL void rBAT_PlusCIMI(
  CHAR *imsi);

GLOBAL void rBAT_PercentSATI ( SHORT len, UBYTE* satCmd );
GLOBAL void rBAT_PercentSATE (SHORT len, UBYTE* satCmd);

GLOBAL void rBAT_PercentKSIR(
  T_ACI_KSIR *ksStat);

GLOBAL void rBAT_PercentCPI(
  SHORT cId,
  T_ACI_CPI_MSG msgType,
  T_ACI_CPI_IBT ibt,
  T_ACI_CPI_TCH tch,
  USHORT cause);

GLOBAL void rBAT_PercentCTYI(
  T_ACI_CTTY_NEG neg,
  T_ACI_CTTY_TRX trx);

GLOBAL void rBAT_PlusCNUM(
  T_ACI_CNUM_MSISDN *msisdn,
  UBYTE num);

GLOBAL void rBAT_PlusCPOL(
  SHORT startIdx,
  SHORT lastIdx,
  T_ACI_CPOL_OPDESC *operLst,
  SHORT usdNtry);

GLOBAL void rBAT_PlusCCCM(
  LONG *ccm);

GLOBAL void rBAT_PercentCTV(void);

#ifdef SIM_TOOLKIT
GLOBAL void rBAT_PercentSATN (SHORT len, UBYTE* satCmd, T_ACI_SATN_CNTRL_TYPE  cntrl_type);
#ifdef FF_SAT_E
GLOBAL void rBAT_PercentSATA (SHORT cId, LONG rdlTimeout_ms, T_ACI_SATA_ADD* addParm);
#else
GLOBAL void rBAT_PercentSATA (SHORT cId, LONG rdlTimeout_ms);
#endif
#endif /* SIM_TOOLKIT */

GLOBAL void rBAT_sms_ready(void);

GLOBAL void rBAT_phb_status(
  T_ACI_PB_STAT status);

GLOBAL void rBAT_PercentSIMINS(
  T_ACI_CME_ERR err);

GLOBAL void rBAT_PlusCRSM(
  SHORT sw1,
  SHORT sw2,
  SHORT rspLen,
  UBYTE *rsp);

GLOBAL void rBAT_PlusCSIM(
  SHORT len,
  UBYTE *rsp);

GLOBAL void rBAT_PercentCCBS(
  T_ACI_CCBS_IND ind,
  T_ACI_CCBS_STAT status,
  T_ACI_CCBS_SET *setting,
  BOOL intermediate_result);

GLOBAL void rBAT_PlusCCWV(
  T_ACI_CCWV_CHRG charging);

GLOBAL void rBAT_PercentCNAP(
  T_callingName *NameId,
  T_ACI_CNAP_STATUS status);

GLOBAL void rBAT_SignalSMS(
  UBYTE state);

GLOBAL void rBAT_PlusCLAN(
  T_ACI_LAN_SUP *CLang);

GLOBAL void rBAT_PlusCLAE(
  T_ACI_LAN_SUP *CLang);

#ifdef FF_PS_RSSI
GLOBAL void rBAT_PercentCSQ(
  UBYTE rssi,
  UBYTE ber,
  UBYTE actlevel,
  UBYTE min_access_level);
#else
GLOBAL void rBAT_PercentCSQ(
  UBYTE rssi,
  UBYTE ber,
  UBYTE actlevel);
#endif

GLOBAL void rBAT_PercentALS(
  T_ACI_ALS_MOD ALSmode);

#ifdef FF_TIMEZONE
GLOBAL void rBAT_PlusCTZV(
  S32 timezone);
#else
GLOBAL void rBAT_PlusCTZV(
  UBYTE *timezone);
#endif

#ifdef GPRS
GLOBAL void rBAT_PercentCREG(
  T_ACI_CREG_STAT status,
  USHORT lac,
  USHORT cid,
  T_ACI_P_CREG_GPRS_IND gprs_ind,
  U8     rt);

GLOBAL void rBAT_PlusCGACT(
  SHORT link_id);

GLOBAL void rBAT_PlusCGDATA(
  SHORT link_id);

GLOBAL void rBAT_PlusCGANS(
  SHORT link_id);

GLOBAL void rBAT_PlusCGEREP(
  T_CGEREP_EVENT event,
  T_CGEREP_EVENT_REP_PARAM *param);

GLOBAL void rBAT_PlusCGREG(
  T_CGREG_STAT stat,
  USHORT lac,
  USHORT ci);

GLOBAL void rBAT_changedQOS(
  SHORT cid,
  T_PS_qos *qos);

GLOBAL void rBAT_PercentSNCNT(
  UBYTE c_id,
  ULONG octets_uplink,
  ULONG octets_downlink,
  ULONG packets_uplink,
  ULONG packets_downlink);

GLOBAL void rBAT_PercentCGREG(
  T_P_CGREG_STAT stat,
  USHORT lac,
  USHORT ci,
  BOOL bActiveContext);
#endif

GLOBAL void rBAT_PercentEM(
  T_EM_VAL *val_tmp);

GLOBAL void rBAT_PercentEMET(
  T_DRV_SIGNAL_EM_EVENT *Signal);

GLOBAL void rBAT_PercentEMETS(
  UBYTE entity);

GLOBAL void rBAT_PercentCPNUMS(
  UBYTE element_index,
  UBYTE index_level,
  CHAR *alpha_tag,
  CHAR *number,
  BOOL premium_flag,
  BOOL network_flag);

GLOBAL void rBAT_PercentCPVWI(
  UBYTE flag_set, 
  USHORT line);

GLOBAL void rBAT_PercentCPROAM(
  UBYTE roam_status);

GLOBAL void rBAT_PlusCIEV(
  T_ACI_MM_CIND_VAL_TYPE sCindValues,
  T_ACI_MM_CMER_VAL_TYPE sCmerSettings);

GLOBAL void rBAT_PercentRDL(
  T_ACI_CC_REDIAL_STATE state);

#ifdef TI_PS_FF_AT_P_CMD_RDLB
GLOBAL void rBAT_PercentRDLB(
  T_ACI_CC_RDL_BLACKL_STATE state);
#endif /* TI_PS_FF_AT_P_CMD_RDLB */

GLOBAL void rBAT_PercentCCCN(
  T_ACI_FAC_DIR tDirection,
  SHORT cId,
  T_MNCC_fac_inf *fie);

GLOBAL void rBAT_PercentCSSN(
  T_ACI_FAC_DIR tDirection,
  T_ACI_FAC_TRANS_TYPE tType,
  T_MNCC_fac_inf *fie);

GLOBAL void rBAT_PercentCSTAT(
  T_ACI_STATE_MSG msgType);

GLOBAL void rBAT_Z(void);

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
GLOBAL void rBAT_PercentCPRSM(
  T_ACI_CPRSM_MOD mode);
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */

GLOBAL void rBAT_PercentCTZV(
  T_MMR_INFO_IND *mmr_info_ind,
  S32 timezone);

GLOBAL void rBAT_PercentCNIV(
  T_MMR_INFO_IND *mmr_info_ind);

#ifdef GPRS
GLOBAL void rBAT_PercentCGEV(
  T_CGEREP_EVENT event,
  T_CGEREP_EVENT_REP_PARAM *param);
#endif

GLOBAL void rBAT_PercentCPRI(
  UBYTE gsm_ciph,
  UBYTE gprs_ciph);

GLOBAL void rBAT_PercentIMEI(T_ACI_IMEI *imei);

GLOBAL void rBAT_PercentSIMEF(T_SIM_FILE_UPDATE_IND *sim_file_update_ind);

#ifdef REL99
GLOBAL void rBAT_PercentCMGRS (UBYTE mode,
                              T_MNSMS_RETRANS_CNF * mnsms_retrans_cnf,
                              T_MNSMS_SEND_PROG_IND * mnsms_send_prog_ind);
#endif /* REL99 */

#ifdef REL99
GLOBAL void rBAT_PlusCGCMOD ( void );
#endif /* REL99 */

#endif

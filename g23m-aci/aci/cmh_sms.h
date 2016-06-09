/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SMS
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
|             short message service.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_SMS_H
#define CMH_SMS_H

#include "psa_sms.h"

/*==== CONSTANTS ==================================================*/

/* for the following constants see  GSM 11.11,  chapter 10.3.3 */
#define STAT_MASK  0x07
#define REC_UNREAD 0x03 /* message received by MS from network */
                        /* message to be read                  */
#define REC_READ   0x01 /* message received by MS from network */
                        /* message read                        */
#define STO_UNSENT 0x07 /* MS originating message              */
                        /* message to be sent                  */
#define STO_SENT   0x05 /* MS originating message              */
                        /* message sent to the network         */

/* TP-Validity-Period-Format (TP-VPF) */
#define VPF_MASK           0x18
#define VPF_NOT_PRESENT    0x00
#define VPF_RELATIVE       0x10
#define VPF_ABSOLUTE       0x18
#define VPF_ENHANCED       0x08


#define SMS_CMH_MAX_START_IDX (255)

#define MIN_SMS_PRM_LEN         28
#define MAX_SMS_PRM_ADDR_OCTETS 10 /* = MAX_SMS_ADDR_DIG/2 */
#define SIM_SMSP_V_DEST  0x01
#define SIM_SMSP_V_SCA   0x02
#define SIM_SMSP_V_PID   0x04
#define SIM_SMSP_V_DCS   0x08
#define SIM_SMSP_V_VPREL 0x10

/*
 * Value constants for bcd
 */
#define BCD_ASTSK                      0xa         /* (                              */
#define BCD_PND                        0xb         /* #                              */
#define BCD_A                          0xc         /* a                              */
#define BCD_B                          0xd         /* b                              */
#define BCD_C                          0xe         /* c                              */
#define BCD_RES                        0xf         /* reserved                       */

/*==== TYPES ======================================================*/

/*typedef struct
{
  UBYTE npi : 4;
  UBYTE ton : 3;
  UBYTE ext : 1;
} T_TON_NPI;*/

typedef struct
{
  UBYTE     length;
  UBYTE     ton_npi;
  UBYTE     addr[MAX_SMS_PRM_ADDR_OCTETS];
} T_RP_ADDRESS;

typedef struct
{
  UBYTE     no_bcd;
  UBYTE     ton_npi;
  UBYTE     addr[MAX_SMS_PRM_ADDR_OCTETS];
} T_TP_ADDRESS;

typedef struct
{
  UBYTE     par_ind;
  UBYTE     dest_no_bcd;
  UBYTE     dest_ton_npi;
  UBYTE     dest_addr[MAX_SMS_PRM_ADDR_OCTETS];
  UBYTE     sca_length;
  UBYTE     sca_ton_npi;
  UBYTE     sca_addr[MAX_SMS_PRM_ADDR_OCTETS];
  UBYTE     pid;
  UBYTE     dcs;
  UBYTE     vp_rel;
} T_ACI_SMS_SIM_PARAMS;

typedef struct
{
  UBYTE lowerMSB;
  UBYTE lowerLSB;
  UBYTE upperMSB;
  UBYTE upperLSB;
} T_ACI_CBM_SIM_MID_RANGE;

typedef struct
{
  UBYTE MSB;
  UBYTE LSB;
} T_ACI_CBM_SIM_MID_LIST;


/********************************************************************
 *
 * SMS Profile for FFS
 *
 ********************************************************************/


#define MAX_FFS_SMSPRFLS        4

#define FFS_SMSPRFL_PATH       "/gsm/sms"

#define FFS_SMSPRFL_FNAME01    "/gsm/sms/smsprfl01"
#define FFS_SMSPRFL_FNAME02    "/gsm/sms/smsprfl02"
#define FFS_SMSPRFL_FNAME03    "/gsm/sms/smsprfl03"
#define FFS_SMSPRFL_FNAME04    "/gsm/sms/smsprfl04"

#define SIZE_FFS_SMSPRFL_SCA   20
#define SIZE_FFS_SMSPRFL_MIDS  40
#define SIZE_FFS_SMSPRFL_DCSS  20
#define SIZE_FFS_SMSPRFL_VPABS 15
#define SIZE_FFS_SMSPRFL_VPENH 15

#define FFS_SMSPRFL_VLD        0x00
#define FFS_SMSPRFL_INVLD      0xFF

typedef struct ffs_EFsmsprfl_Type          /* SMS Profile                      */
{
  UBYTE vldFlag;                           /* Valid Flag                       */
  UBYTE CSCAsca[SIZE_FFS_SMSPRFL_SCA];     /* Service Center Address           */  
  UBYTE CSCAlenSca;                        /* Length of Service Center Address */
  UBYTE CSCAton;                           /* Type of Number                   */
  UBYTE CSCAnpi;                           /* Numbering Plan Identification    */
  UBYTE CSCBmode;                          /* Mode                             */
  UBYTE CSCBmids[SIZE_FFS_SMSPRFL_MIDS];   /* Message Identifier               */
  UBYTE CSCBdcss[SIZE_FFS_SMSPRFL_DCSS];   /* Data Coding Schemes              */
  UBYTE CSMPfo;                            /* First Octet                      */
  UBYTE CSMPvprel;                         /* Validity Period Relative         */
  UBYTE CSMPvpabs[SIZE_FFS_SMSPRFL_VPABS]; /* Validity Period Absolute         */
  UBYTE CSMPvpenh[SIZE_FFS_SMSPRFL_VPENH]; /* Validity Period Enhanced         */
  UBYTE CSMPpid;                           /* Protocol Identifier              */
  UBYTE CSMPdcs;                           /* Data Coding Scheme               */
  UBYTE IMSI[MAX_IMSI];                    /* IMSI                             */
} T_ACI_FFS_SMSPRFL;

#define SIZE_FSS_SMSPRFL ( SIZE_FFS_SMSPRFL_SCA   +   \
                           SIZE_FFS_SMSPRFL_MIDS  +   \
                           SIZE_FFS_SMSPRFL_DCSS  +   \
                           SIZE_FFS_SMSPRFL_VPABS +   \
                           MAX_IMSI + \
                           SIZE_FFS_SMSPRFL_VPENH + 9   )

/*==== Global Declaration =================================================*/
#ifdef CMH_SMSR_C
GLOBAL T_ACI_SMS_STAT   cmglStat = SMS_STAT_All;
#else
EXTERN T_ACI_SMS_STAT   cmglStat;
#endif /* CMH_SMSR_C */

/*==== PROTOTYPES =================================================*/

EXTERN UBYTE  cmhSMS_getNType       ( T_ACI_TOA_TON   ton         );
EXTERN BOOL   cmhSMS_findPrflId     ( UBYTE critrerium,
                                      void *elem                  );
EXTERN T_ACI_TOA_TON
              cmhSMS_getTon         ( UBYTE           ntype       );
EXTERN UBYTE  cmhSMS_getNPlan       ( T_ACI_TOA_NPI   npi         );
EXTERN T_ACI_TOA_NPI
              cmhSMS_getNpi         ( UBYTE           nplan       );
EXTERN BOOL   cmhSMS_isVpabsVld     ( T_ACI_VP_ABS*   vpabs       );
EXTERN BOOL   cmhSMS_isVpenhVld     ( T_ACI_VP_ENH*   vpenh       );
EXTERN void   cmhSMS_setVpabsPsa    ( T_tp_vp_abs*    psaVp,
                                      T_ACI_VP_ABS*   cmhVp       );
EXTERN void   cmhSMS_setVpenhPsa    ( T_tp_vp_enh*    psaVp,
                                      T_ACI_VP_ENH*   cmhVp       );
EXTERN void   cmhSMS_setVpabsCmh    ( T_ACI_VP_ABS*   cmhVp,
                                      T_tp_vp_abs*    psaVp       );
EXTERN void   cmhSMS_setVpenhCmh    ( T_ACI_VP_ENH*   cmhVp,
                                      T_tp_vp_enh*    psaVp       );
EXTERN UBYTE  cmhSMS_getAdrStr      ( CHAR*           pStr,
                                      UBYTE           maxIdx,
                                      UBYTE*          pBcd,
                                      UBYTE           numDigits   );
EXTERN void   cmhSMS_getAdrBcd      ( UBYTE*          pBcd,
                                      UBYTE*          pNumDigits,
                                      UBYTE           maxDigits,
                                      CHAR*           pStr        );
EXTERN T_ACI_RETURN   cmhSMS_packAlphaNumAddr( CHAR*          da, 
                                               T_tp_da*       da_addr);
EXTERN BOOL   cmhSMS_getMemPsa      ( T_ACI_SMS_STOR  inMem,
                                      UBYTE*          outMem      );
EXTERN void   cmhSMS_getMemCmh      ( UBYTE           inMem,
                                      T_ACI_SMS_STOR* outMem      );
EXTERN void   cmhSMS_expdSmsPp      ( UBYTE           byte_offset,
                                      UBYTE           dcs,
                                      UBYTE*          source,
                                      UBYTE           source_len,
                                      UBYTE*          dest,
                                      UBYTE*          dest_len    );
EXTERN void   cmhSMS_rdcSmsPp       ( UBYTE           byte_offset,
	                                  UBYTE           dcs,
                                      UBYTE*          source,
                                      UBYTE           source_len,
                                      UBYTE*          dest,
                                      UBYTE*          dest_len    );
EXTERN void   cmhSMS_expdSmsCb      ( UBYTE           dcs,
                                      UBYTE*          source,
                                      UBYTE           source_len,
                                      UBYTE*          dest,
                                      UBYTE*          dest_len    );
EXTERN void   cmhSMS_getStatCmh     ( UBYTE           inStat,
                                      T_ACI_SMS_STAT* outStat     );
EXTERN BOOL   cmhSMS_getStatPsa     ( T_ACI_SMS_STAT  inStat,
                                      UBYTE*          outStat     );
EXTERN CHAR*  cmhSMS_setToaDef      ( CHAR*           number,
                                      UBYTE*          ntype,
                                      UBYTE*          nplan       );
EXTERN SHORT  cmhSMS_getTimezone    ( UBYTE           timezone    );
EXTERN UBYTE  cmhSMS_setTimezone    ( SHORT           timezone    );
EXTERN void   cmhSMS_setStorOcc     ( T_ACI_SMS_STOR_OCC* outMem,
                                      UBYTE           inMem       );
EXTERN SHORT  cmhSMS_getNextEntry   ( SHORT           index,
                                      T_ACI_SMS_STOR  mem         );
EXTERN UBYTE  cmhSMS_getAlphabetPp  ( UBYTE           dcs         );
EXTERN SHORT  cmhSMS_InfoConfirm    ( void );
EXTERN SHORT  cmhSMS_CBMIndication  ( T_MMI_CBCH_IND      * mmi_cbch_ind );

EXTERN SHORT  cmhSMS_SMSStatRpt     ( T_MNSMS_STATUS_IND  * mnsms_status_ind);
EXTERN SHORT  cmhSMS_SMSMemory      ( T_MNSMS_MESSAGE_IND * mnsms_message_ind);
EXTERN SHORT  cmhSMS_SMSDeliver     ( T_MNSMS_MESSAGE_IND * mnsms_message_ind );
EXTERN SHORT  cmhSMS_Result         ( T_MNSMS_REPORT_IND  * mnsms_report_ind );
EXTERN SHORT  cmhSMS_SMSInitState   ( T_MNSMS_MESSAGE_IND * mnsms_message_ind );
EXTERN SHORT  cmhSMS_SMSDelCnf      ( T_MNSMS_DELETE_CNF  * mnsms_delete_cnf );
EXTERN SHORT  cmhSMS_SMRead         ( T_MNSMS_READ_CNF    * mnsms_read_cnf);
EXTERN SHORT  cmhSMS_SMSStoCnf      ( T_MNSMS_STORE_CNF   * mnsms_store_cnf );
EXTERN SHORT  cmhSMS_SMSSbmCnf      ( T_MNSMS_SUBMIT_CNF  * mnsms_submit_cnf );
EXTERN SHORT  cmhSMS_SMSCmdCnf      ( T_MNSMS_COMMAND_CNF * mnsms_command_cnf );
EXTERN SHORT  cmhSMS_SMSErrorInd    ( T_MNSMS_ERROR_IND   * mnsms_error_ind );

#ifdef REL99
EXTERN SHORT  cmhSMS_SMSSendProgInd ( T_MNSMS_SEND_PROG_IND * mnsms_send_prog_ind );
EXTERN SHORT  cmhSMS_SMSRetransCnf  ( T_MNSMS_RETRANS_CNF * mnsms_retrans_cnf );
#endif /* REL99 */

EXTERN void   cmhSMS_SMSResumeCnf   ( T_MNSMS_RESUME_CNF  * mnsms_resume_cnf );
EXTERN void   cmhSMS_SMSQueryCnf     (T_MNSMS_QUERY_CNF  * mnsms_query_cnf);

EXTERN void   cmhSMS_WrCnfCSAS      (SHORT aId);
EXTERN void   cmhSMS_RdCnfCRES      (SHORT aId);
EXTERN void   cmhSMS_InitSMSP       (SHORT aId);
EXTERN BOOL   cmhSMS_checkSIM       (void);
EXTERN BOOL   cmhSMS_checkAccess    (T_ACI_CMD_SRC srcId,
                                     T_ACI_RETURN *ret);
EXTERN T_ACI_CMS_ERR cmhSMS_GetCmsFromSim ( USHORT errCode );
GLOBAL T_ACI_CMS_ERR cmhSMS_GetCmsFromSms ( USHORT errCode );
EXTERN void   cmhSMS_ready          ( void );
EXTERN SHORT  cmhSMS_getPrfRge      ( void );
EXTERN BOOL cmhSMS_GetPrmSIM   (T_ACI_CMD_SRC srcId,
                                UBYTE         *data,
                                int           dataLen);
EXTERN BOOL cmhSMS_PutPrmSIM   (T_ACI_CMD_SRC srcId,
                                UBYTE         *data,
                                int           maxDataLen);
EXTERN BOOL cmhSMS_GetCbmirSIM (T_ACI_CMD_SRC srcId,
                                UBYTE         *data,
                                int           dataLen);
EXTERN BOOL cmhSMS_PutCbmirSIM (T_ACI_CMD_SRC srcId,
                                UBYTE         *data,
                                int           maxDataLen);
EXTERN BOOL cmhSMS_GetCbmiSIM  (T_ACI_CMD_SRC srcId,
                                UBYTE         *data,
                                int           dataLen);
EXTERN BOOL cmhSMS_PutCbmiSIM (T_ACI_CMD_SRC srcId,
                                UBYTE         *data,
                                int           maxDataLen);
#ifdef SIM_TOOLKIT
EXTERN BOOL cmhSMS_GetCbDtaDwnlSIM  (T_ACI_CMD_SRC srcId,
                                     UBYTE         *data,
                                     int           dataLen);
#endif /* of SIM_TOOLKIT */


EXTERN UBYTE* cmhSMS_decodeMsg (T_sms_sdu *sms_sdu,
                                T_rp_addr* rp_addr,
                                UBYTE vt_mti);

EXTERN void cmhSMS_codeMsg     (T_sms_sdu *sms_sdu,
                                UBYTE tp_vt_mti,
                                T_rp_addr* sc_addr,
                                UBYTE tp_mti,
                                UBYTE* decoded_pdu);

EXTERN BOOL cmhSMS_cpyDeliver  (T_ACI_CMGL_SM  * sm,
                                T_sms_sdu * sms_sdu);

EXTERN BOOL cmhSMS_cpySubmit   (T_ACI_CMGL_SM  * sm,
                                T_sms_sdu * sms_sdu);
/* Implements Measure # 110 */
EXTERN BOOL cmhSMS_cpyMsgIndReadCnf (T_ACI_CMGL_SM  *sm, 
                                     UBYTE          *status, 
                                     T_sms_sdu      *sms_sdu,
                                     UBYTE          rec_num);
/* Implements measure 147,148, 149 */
EXTERN void cmhSMS_sendConfigureReq (BOOL v_cmms_val);
EXTERN BOOL cmhSMS_cpyStatInd  (T_ACI_CDS_SM  * sm,
                                T_MNSMS_STATUS_IND *mnsms_status_ind);

EXTERN void cmhSMS_getPhbEntry (UBYTE *buf,
                                T_ACI_PB_TEXT *alpha,
                                T_ACI_SMS_STAT status);

EXTERN void cmhSMS_codeDelRep  (T_sms_sdu * sms_sdu,
                                T_rp_addr * sc_addr);

EXTERN UBYTE CodeRPAddress     (UBYTE *buf,
                                UBYTE  numDigits,
                                UBYTE ton,
                                UBYTE npi,
                                UBYTE *bcd);

EXTERN UBYTE CodeTPAddress     (UBYTE *buf,
                                UBYTE  numDigits,
                                UBYTE ton,
                                UBYTE npi,
                                UBYTE *bcd);

EXTERN UBYTE DecodeRPAddress   (UBYTE *c_num,
                                UBYTE *ton,
                                UBYTE *npi,
                                UBYTE *bcd,
                                UBYTE *buf);

EXTERN UBYTE DecodeTPAddress   (UBYTE *c_num,
                                UBYTE *ton,
                                UBYTE *npi,
                                UBYTE *bcd,
                                UBYTE *buf);

EXTERN void cmhSMS_fillTpSubmit(T_TP_SUBMIT*    tp_submit,
                                T_ACI_CMD_SRC   srcId,
                                UBYTE           msgType,
                                UBYTE           mr,
                                T_tp_da*        da_addr,
                                T_ACI_SM_DATA*  data,
                                UBYTE           septets,
                                T_ACI_UDH_DATA* udh);

EXTERN void cmhSMS_fillTpDeliver(T_TP_DELIVER*   tp_deliver,
                                 T_ACI_CMD_SRC   srcId,
                                 UBYTE           msgType,
                                 T_tp_oa*        oa_addr,
                                 T_ACI_SM_DATA*  data,
                                 UBYTE           septets,
                                 T_ACI_UDH_DATA* udh );

EXTERN void cmhSMS_fillTpCommand(T_TP_COMMAND*   tp_command,
                                UBYTE           fo,
                                UBYTE           ct,
                                UBYTE           mr,
                                UBYTE           pid,
                                UBYTE           mn,
                                T_tp_da*        da_addr,
                                T_ACI_CMD_DATA* data,
                                T_ACI_UDH_DATA* udh);

/* ACI-SPR-17004: RFU bits have to be ignored (GSM 11.11) */ 
EXTERN void cmhSMS_removeStatusRFUBits ( UBYTE* status );

EXTERN T_ACI_CPRSM_MOD cmhSMS_convertDeliverStatusToACI( UBYTE status );

#ifdef FF_CPHS
EXTERN BOOL cmhSMS_voice_mail_ind( T_sms_sdu *sms_sdu);
#endif /* FF_CPHS */                                

EXTERN void cmhSMS_resetMtDsCnmiParam(void);
EXTERN BOOL cmhSMS_storePduToSim( T_ACI_CMD_SRC  srcId,
                                           UBYTE          stat,
                                           T_ACI_SM_DATA  *pdu );

EXTERN BOOL cmhSMS_SMSQueryType (T_sms_sdu *sms_sdu, UBYTE *msg_type);
EXTERN void cmhSMS_SendDelete_Req ( UBYTE  index, UBYTE  status );
#ifdef FF_CPHS_REL4
EXTERN BOOL cmhSMS_chk_SpclMsg( T_MNSMS_MESSAGE_IND *mnsms_message_ind );
#endif
#endif /* CMH_SMS_H */

/*==== EOF =======================================================*/

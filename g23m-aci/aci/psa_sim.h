/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PSA
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
|  Purpose :  Definitions for the protocol stack adapter
|             Subscriber Identity Module ( SIM )
+-----------------------------------------------------------------------------
*/

#ifndef PSA_SIM_H
#define PSA_SIM_H

/*==== CONSTANTS ==================================================*/
#define PIN_LEN       (8)         /* PIN length in bytes */
#define MIN_PIN_LEN   (4)         /* Marcus: Issue 1589: 28/01/2003: Minimum PIN length in bytes */
#define PUK_LEN       (8)         /* PUK length in bytes */
#define ICC_LEN       (10)        /* chip card identifier length in bytes */
#define SRV_TAB_LEN   MAX_SRV_TBL /* service table length in bytes */
#define CBM_ID_LEN    (10)        /* CBM id length in bytes */
#define DPER_KEY_LEN  (16)        /* de-pers. key length in bytes */
#define ACC_MAX       (8)         /* maximum number of simultanious
                                     SIM access */
#define SRV_ALLOC_ACTIV (0x03)    /* SIM service allocated and activated */

#define NO_ENTRY      (-1)        /* not a valid entry */

/* Masking for SIM service table: */
#define NO_ALLOCATED              0
#define ALLOCATED_AND_DEACTIVATED 2
#define ALLOCATED_AND_ACTIVATED   3

/* CPHS_CSP PLMN mode bit*/
#define CPHS_CSP_PLMN_MODE_BIT_OFF 0x00
#define CPHS_CSP_PLMN_MODE_BIT_ON  0X01

#define UBYTES_PER_PLMN           3

typedef enum
{
  SRV_CHV1_Disable = 1,  /* #1  CHV1 disable function                                     */
  SRV_ADN,               /* #2  Abbreviated Dialling Numbers (ADN)                        */
  SRV_FDN,               /* #3  Fixed Dialling Numbers (FDN)                              */
  SRV_SMS_Storage,       /* #4  Short Message Storage (SMS)                               */
  SRV_AOC,               /* #5  Advice of Charge (AoC)                                    */
  SRV_CCP,               /* #6  Capability Configuration Parameters (CCP)                 */
  SRV_PLMN_Select,       /* #7  PLMN selector                                             */
  SRV_RFU1,              /* #8  RFU                                                       */
  SRV_MSISDN,            /* #9  MSISDN                                                    */
  SRV_EXT1,              /* #10 Extension1                                                */
  SRV_EXT2,              /* #11 Extension2                                                */
  SRV_SMS_Parms,         /* #12 SMS Parameters                                            */
  SRV_LDN,               /* #13 Last Number Dialled (LND)                                 */
  SRV_CBM_Ident,         /* #14 Cell Broadcast Message Identifier                         */
  SRV_GrpLvl1,           /* #15 Group Identifier Level 1                                  */
  SRV_GrpLvl2,           /* #16 Group Identifier Level 2                                  */
  SRV_SrvProvName,       /* #17 Service Provider Name                                     */
  SRV_SDN,               /* #18 Service Dialling Numbers (SDN)                            */
  SRV_EXT3,              /* #19 Extension3                                                */
  SRV_RFU2,              /* #20 RFU                                                       */
  SRV_VCGS,              /* #21 VGCS Group Identifier List (EFVGCS and EFVGCSS)           */
  SRV_VBS,               /* #22 VBS Group Identifier List (EFVBS and EFVBSS)              */
  SRV_EMLPP,             /* #23 enhanced Multi Level Precedence and Pre emption Service   */
  SRV_AutoEMLPP,         /* #24 Automatic Answer for eMLPP                                */
  SRV_DtaDownlCB,        /* #25 Data download via SMS CB                                  */
  SRV_DtaDownlPP,        /* #26 Data download via SMS PP                                  */
  SRV_MnuSel,            /* #27 Menu selection                                            */
  SRV_CalCntrl,          /* #28 Call control                                              */
  SRV_ProActSIM,         /* #29 Proactive SIM                                             */
  SRV_CBMIdRnge,         /* #30 Cell Broadcast Message Identifier Ranges                  */
  SRV_BDN,               /* #31 Barred Dialling Numbers (BDN)                             */
  SRV_EXT4,              /* #32 Extension4                                                */
  SRV_DePersCK,          /* #33 De personalization Control Keys                           */
  SRV_CoOpNwL,           /* #34 Co operative Network List                                 */
  SRV_SMS_StatRep,       /* #35 Short Message Status Reports                              */
  SRV_NwIndAlMS,         /* #36 Network's indication of alerting in the MS                */
  SRV_MOSMCtrlSIM,       /* #37 Mobile Originated Short Message control by SIM            */
  SRV_GPRS,              /* #38 GPRS                                                      */
  SRV_RFU3,              /* #39 Image (IMG)                                               */
  SRV_RFU4,              /* #40 SoLSA (Support of Local Service Area)                     */
  SRV_USSDsupportInCC,   /* #41 USSD string data object supported in Call Control         */
  SRV_No_42,             /* #42 RUN AT COMMAND command                                    */
  SRV_No_43,             /* #43 User controlled PLMN Selector with Access Technology      */
  SRV_No_44,             /* #44 Operator controlled PLMN Selector with Access Technology  */
  SRV_No_45,             /* #45 HPLMN Selector with Access Technology                     */
  SRV_No_46,             /* #46 CPBCCH Information                                        */
  SRV_No_47,             /* #47 Investigation Scan                                        */
  SRV_No_48,             /* #48 Extended Capability Configuration Parameters              */
  SRV_No_49,             /* #49 MExE                                                      */
  SRV_No_50,             /* #50 RPLMN last used Access Technology                         */
  SRV_PNN,               /* #51 PLMN Network Name                                         */
  SRV_OPL,               /* #52 Operator PLMN List                                        */
  SRV_No_53,             /* #53 Mailbox Dialling Numbers                                  */ 
  SRV_No_54,             /* #54 Message Waiting Indication Status                         */
  SRV_No_55,             /* #55 Call Forwarding Indication Status                         */
  SRV_No_56,             /* #56 Service Provider Display Information                      */
  SRV_No_57,             /* #57 Multimedia Messaging Service (MMS)                        */
  SRV_No_58,             /* #58 Extension 8                                               */
  SRV_No_59              /* #59 MMS User Connectivity Parameters                          */
} T_SIM_SRV;

typedef enum                      /* SIM status */
{
  NO_VLD_SS = 0,                  /* not a valid SIM status */
  SS_OK,                          /* SIM is OK */
  SS_INV,                         /* SIM is invalid */
  SS_BLKD,                        /* SIM is blocked */
  SS_URCHB                        /* SIM is unreachable */
} T_SIM_SIMST;

typedef enum                      /* PIN status */
{
  NO_VLD_PS = 0,                  /* not a valid PIN status */
  PS_RDY,                         /* ready, no PIN is requested */
  PS_PIN1,                        /* PIN 1 is requested */
  PS_PIN2,                        /* PIN 2 is requested */
  PS_PUK1,                        /* PUK 1 is requested */
  PS_PUK2                         /* PUK 2 is requested */

} T_SIM_PINST;

typedef enum                      /* PIN1 enable/disable status */
{
  NO_VLD_PEDS = 0,                /* not a valid PED status */
  PEDS_ENA,                       /* PIN 1 enabled */
  PEDS_DIS                        /* PIN 1 disabled */

} T_SIM_PEDST;

typedef enum                      /* SIM access type */
{
  NO_VLD_ACT = 0,                 /* not a valid access type */
  ACT_RD_DAT,                     /* read a datafield */
  ACT_WR_DAT,                     /* write a datafield */
  ACT_RD_REC,                     /* read a record */
  ACT_WR_REC,                     /* write a record */
  ACT_INC_DAT                     /* increment a datafield */

} T_SIM_ACTP;

/*==== TYPES ======================================================*/
typedef struct SIMSetPrm
{
  UBYTE     actProc;                /* activation procedure */
  UBYTE     STKprof[MAX_STK_PRF];   /* SIM toolkit profile */
  UBYTE     PINType;                /* type of PIN */
  CHAR      curPIN[PIN_LEN];        /* current PIN */
  CHAR      newPIN[PIN_LEN];        /* new PIN */
  CHAR      unblkKey[PUK_LEN];      /* unblocking key */
/*
** This flag is the customisation flag, introuduced for the Cust1 MMI. It is defaulted to 0, and is
** changed by the AT%CUST=n command. If the %CUST command is not received, the ACI and
** other afffected entities will continue to behave as normal
*/
  UBYTE     cust_mode;
/*
** This flag is the STK Call or Short Message Control flag, introuduced for the Cust1 MMI.
** It is defaulted to 1, and is changed by the AT%SATCC=n command. If the %SATCC command is not received,
** the ACI entity will continue to behave as normal. If CC or SM Control By SIM is disabled it will be re-enabled
** automatically, on receipt of a Terminal Response from the MMI.
*/
  UBYTE     sat_cc_mode;
#ifdef FF_DUAL_SIM
  UBYTE     SIM_Selected; /* stores the currently selected SIM number*/
#endif /*FF_DUAL_SIM*/
} T_SIM_SET_PRM;

typedef struct SIMAccPrm
{
  UBYTE     ntryUsdFlg;             /* flags entry usage */
  UBYTE     accType;                /* type of access */
  BOOL      v_path_info;            /* Indicates whether path_info variable has valid values */
  T_path_info  path_info;           /* Contains the whole path to the EF  */
  USHORT    reqDataFld;             /* requested datafield identifier */
  USHORT    dataOff;                /* datafield offset */
  UBYTE     recNr;                  /* record number */
  BOOL      check_dataLen;          /* has size of data to be checked against size of exch buffer ? */
  UBYTE     dataLen;                /* data length */
  UBYTE   * exchData;               /* points to exchange data buffer */
  UBYTE     recMax;                 /* maximum records */
  USHORT    errCode;                /* error code */
  void    (*rplyCB)(SHORT aId);     /* points to reply call-back */
} T_SIM_ACC_PRM;

typedef struct SIMTrnsAccPrm
{
  UBYTE     cmd;                    /* access command */
  USHORT    reqDataFld;             /* requested datafield identifier */
  UBYTE     p1;                     /* parameter 1 */
  UBYTE     p2;                     /* parameter 2 */
  UBYTE     p3;                     /* parameter 3 */
  USHORT    dataLen;                /* data length in bytes */
  UBYTE   * transData;              /* points to data buffer */
} T_SIM_TRNS_ACC_PRM;

typedef struct
{
  UBYTE   sw1;                      /* SIM result code 1 */
  UBYTE   sw2;                      /* SIM result code 2 */
  USHORT  rspLen;                   /* length of response data */
  UBYTE * rsp;                      /* ponter to response data */
} T_SIM_TRNS_RSP_PRM;

#ifdef FF_SAT_E 
typedef struct
{
  UBYTE dtiConn;                   /* dti connection qualifier */
  UBYTE dtiUnit;                   /* dti connection unit */
  UBYTE chnId;                     /* move to sat_bip_chn channel id */
  UBYTE bipConn;                   /* bip connection qualifier */
  UBYTE genRes;                    /* general result */
  UBYTE addRes;                    /* additional result */
} T_SIM_SAT_CHN;

#endif /* FF_SAT_E */ 

#ifdef FF_SAT_E
typedef struct
{
  T_SIM_SAT_CHN sat_chn_prm;
  void (*dti_cb)(UBYTE dtiConn, UBYTE chnId);      /* holds callback for DTI estb */  
  void (*bip_cb)(UBYTE bipConn, UBYTE chnId);/* holds callback for BIP estb */
} T_SIM_DTI_CH_PRM;

#endif /* F_SAT_E */


typedef struct
{
  UBYTE plmn[UBYTES_PER_PLMN]; /* Packed PLMN as stored on the SIM */
  UBYTE pnn_rec_num;
  USHORT lac1;
  USHORT lac2;
} T_opl;

typedef struct
{
  UBYTE num_rcd; /* Number of records in EFopl */
  BOOL opl_status; /* Status of OPL records retrieval */
  T_opl opl_rcd[OPL_MAX_RECORDS];
} T_opl_field;//EONS

/*
 * Compare this with the similiar struct T_pnn_name in psa_mm.h
 */
typedef struct
{
  UBYTE v_plmn;                         /* valid flag */
  UBYTE long_len;                       /* length of operator long name */
  UBYTE long_ext_dcs;                   /* octet 3 of IEI */
  UBYTE long_name [MAX_LONG_OPER_LEN-1];/* long name for operator, no '\0' */
  UBYTE shrt_len;                       /* length of operator short name */
  UBYTE shrt_ext_dcs;                   /* octet 3 of IEI */
  UBYTE shrt_name [MAX_SHRT_OPER_LEN-1];/* short name for operator, no '\0' */
} T_pnn;

typedef struct
{
  UBYTE num_rcd;
  BOOL pnn_status;
  T_pnn pnn_rcd[PNN_MAX_RECORDS];
} T_pnn_field;

typedef struct
{
  UBYTE len;                       /* length of atr */
  UBYTE data[MAX_SIM_ATR];         /* atr data      */
} T_SIM_ATR;

typedef struct SIMShrdParm
{
  T_OWN     owner;                  /* identifies the used set */
  SHORT     aId;                    /* access identifier */
  T_SIM_SET_PRM setPrm[OWN_SRC_MAX];    /* possible sets */
  T_SIM_ACC_PRM atb[ACC_MAX];       /* table of access parameter */
#ifdef FF_DUAL_SIM
  UBYTE     SIM_Powered_on;         /* stores the currently powered SIM number*/
  UBYTE     SIM_Selection;           /* flag to indicate if SIM selection is taking place*/
#endif /*FF_DUAL_SIM*/
  UBYTE     PINStat;                /* status of PIN requirement */
  UBYTE     pn1Cnt;                 /* PIN 1 counter  */
  UBYTE     pn2Cnt;                 /* PIN 2 counter  */
  UBYTE     pk1Cnt;                 /* PUK 1 counter  */
  UBYTE     pk2Cnt;                 /* PUK 2 counter  */
  UBYTE     pn1Stat;                /* PIN 1 status */
  UBYTE     pn2Stat;                /* PIN 2 status */
  UBYTE     PINQuery;               /* CPIN? when CFUN=0 */
  UBYTE     crdPhs;                 /* phase of card */
  UBYTE     SIMStat;                /* status of SIM card */
  UBYTE     PEDStat;                /* PIN1 enable/disable status */
  UBYTE     crdFun;                 /* SIM card functionality */
  UBYTE     srvTab[SRV_TAB_LEN];    /* SIM service table */
  T_imsi_field imsi;                /* IMSI */
  UBYTE     PLMN_Mode_Bit;          /* PLMN mode bit in EF_CSP */
  UBYTE     mnc_len;                /* length of MNC in IMSI (2 or 3 digits)*/
  USHORT    rslt;                   /* result of SIM operation */
  UBYTE     synCs;                  /* SIM synchronisation cause */
  BOOL      ciSIMEnabled;           /* current ciphering indicator state */
  int       fuRef;                  /* Reference for File Update */
  BOOL      imei_blocked;
#ifdef FF_SAT_E  
  T_SIM_DTI_CH_PRM *sim_dti_chPrm;  /* points to sim dti channel parameters */
#endif /* FF_SAT_E */  
  T_opl_field opl_list;  /* Operator PLMN list (for EONS) */
  T_pnn_field pnn_list;  /* Operator PNN  list (for EONS) */
  UBYTE     sat_class_e_dti_id;     /* DTI ID for SAT class E */
  T_SIM_ATR atr;                    /* Answer to Reset */
  UBYTE     overall_cust_mode;  /* Overall Cust Mode, this is set when the SIM is initially activated,
                                                       it is incumbent on the system that once set, every ACI channel
                                                       will use the same customisation mode */
  T_ACI_SIMEF_MODE SIMEFMode[CMD_SRC_MAX];  /* SIMEF mode for each command source */
  T_ACI_CLASS classFDN;             /* FDN classtype during SIM_ACTIVATE_REQ / SIM_ACTIVATE_CNF */
  T_ACI_PB_STAT pb_stat;            /* SIM phonebook status */
} T_SIM_SHRD_PRM;



typedef enum
{
  ACI_INIT_TYPE_ALL         = 0,   /* init all parameters */
  ACI_INIT_TYPE_SOFT_OFF           /* init only parameters for soft power off */
} T_ACI_INIT_TYPE;


/*==== PROTOTYPES =================================================*/

EXTERN BOOL psaSIM_hasCustModeBeenSet(void);

EXTERN SHORT psaSIM_VerifyPIN       ( void );
EXTERN SHORT psaSIM_ChangePIN       ( void );
EXTERN SHORT psaSIM_DisablePIN      ( void );
EXTERN SHORT psaSIM_EnablePIN       ( void );
EXTERN SHORT psaSIM_UnblockCard     ( void );
EXTERN void  psaSIM_SyncSIM         ( void );
EXTERN SHORT psaSIM_AccessSIMData   ( void );
#ifdef FF_DUAL_SIM
EXTERN SHORT psaSIM_SelectSIM       ( void );
#endif /*FF_DUAL_SIM*/
EXTERN SHORT psaSIM_ActivateSIM     ( void );
EXTERN void  psaSIM_Init            ( T_ACI_INIT_TYPE init_type );
EXTERN void  psaSIM_InitAtbNtry     ( SHORT idx );
EXTERN void  psaSIM_CloseAtb        ( USHORT error );
EXTERN SHORT psaSIM_atbNewEntry     ( void );
EXTERN CHAR* psaSIM_cnvrtIMSI2ASCII ( CHAR * imsiBuf );
EXTERN void psaSIM_decodeIMSI (UBYTE* imsi_field, UBYTE imsi_c_field, CHAR* imsi_asciiz);
EXTERN void psaSIM_encodeIMSI (CHAR* imsi_asciiz, UBYTE* imsi_c_field, UBYTE* imsi_field);
#ifdef SIM_PERS_OTA
EXTERN void aci_slock_ota_init();
#endif


#ifdef TRACING
EXTERN  void psaSIM_shrPrmDump      ( void );
#endif /* TRACING */

EXTERN BOOL  psaSIM_ChkSIMSrvSup    ( UBYTE srvNr );

EXTERN SHORT psaSIM_TrnsSIMAccess   ( T_SIM_TRNS_ACC_PRM * prm );

#ifdef FF_SAT_E
EXTERN void  psaSIM_SATBIPChn       ( T_SIM_SAT_CHN chnInf,
                                      void (*cb)(UBYTE bipConn, UBYTE chnId));
EXTERN void  psaSIM_SATChn          ( T_SIM_SAT_CHN chnInf,
                                      void (*cb)(UBYTE dtiConn, UBYTE chnId));
EXTERN void  psaSIM_EvDatAvail      ( BOOL evStat );
EXTERN void  psaSIM_Bip_Req         ( void );
EXTERN void  psaSIM_Bip_Config_Req  ( );
EXTERN void  psaSIM_Dti_Req         ( ULONG link_id );
#endif /* #ifdef FF_SAT_E */

/*==== EXPORT =====================================================*/

#ifdef PSA_SIMF_C

GLOBAL T_SIM_SHRD_PRM simShrdPrm;

#else

EXTERN T_SIM_SHRD_PRM simShrdPrm;

#endif /* PSA_SIMF_C */



#endif /* PSA_SIM_H */


#ifdef CPHS_C
UBYTE cphs_mb_ext_record_num[4] ; 
#else
EXTERN UBYTE cphs_mb_ext_record_num[4] ; 
#endif


/*==== EOF =======================================================*/

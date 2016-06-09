/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SMS
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
|             Short Message Service ( SMS )
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_SMS_H
#define PSA_SMS_H

#include "psa.h"

#define TRACING

/*==== CONSTANTS ==================================================*/

#define MAX_SIM_ME_ENTRIES 256

#define TP_VP_RELATIVE_DEFAULT 0xA7

#define BM0 0 /* CBM indication routing type 0 */
#define BM1 1 /* CBM indication routing type 1 */
#define BM2 2 /* CBM indication routing type 2 */
#define BM3 3 /* CBM indication routing type 3 */

#define DEF_DCS_RANGE 0xFF   /* DCS range default value            */
#define DEF_MID_RANGE 0xFFFF /* MID range default value            */

#ifdef SIM_TOOLKIT
#define SMS_READ_SIM_SMSP   4
#define SMS_READ_SIM_CBMIR  3
#define SMS_READ_SIM_CBMI   2
#define SMS_READ_SIM_CBMID  1
#define SMS_READ_SIM_CMPL   0

#else
#define SMS_READ_SIM_SMSP   3
#define SMS_READ_SIM_CBMIR  2
#define SMS_READ_SIM_CBMI   1
#define SMS_READ_SIM_CMPL   0
#endif /* of SIM_TOOLKIT */

#define TEST_STR_LEN (80)  /* max. length of test parameter string */

/*==== TYPES ======================================================*/



typedef enum                      /* service status */
{                                 
  NO_VLD_SMS = 0,                 /* not a valid SMS status */
  SMS_IDL,                        /* SMS idle */
  SMS_RD_REQ,                     /* SMS read request */
  SMS_STR_REQ,                    /* SMS store request */
  SMS_DEL_REQ,                    /* SMS delete request */
  SMS_CMD_REQ,                    /* SMS command request */
  SMS_SND_REQ                     /* SMS send request */
} T_SMS_SMSST;

typedef struct entStatus2          /* entity status */
{
  T_ACI_CMD_SRC  entOwn;          /* entity owner */
  T_ACI_AT_CMD   curCmd;          /* current command processing */
} T_ENT_STAT2;

typedef struct SMSSetParm
{
  UBYTE     prflId;               /* profile ID equals recNr (starts from 1)*/
  UBYTE     isCopy;               /* copy of parameter set */
  UBYTE     numOfRefs;            /* number of sources which refer this */
  UBYTE     record;               /* record used for actual operation */
  T_rp_addr sca;                  /* service center address */
  UBYTE     msgType;              /* type of message */
  UBYTE     vpRel;                /* relative validity period */
  T_tp_vp_abs  vpAbs;             /* absolute validity period */
  T_tp_vp_enh  vpEnh;             /* enhanced validity period */
  UBYTE     pid;                  /* protocol identifier */
  UBYTE     dcs;                  /* data coding scheme */
    
  T_ACI_CMGF_MOD  CMGFmode;

} T_SMS_SET_PRM;


typedef struct SMSParameter
{
  UBYTE  simTotal;                /* total number of SIM storage   */
  UBYTE  simUsed;                 /* used number of SIM storage (counter)   */
  UBYTE  meTotal;                 /* total number of ME storage    */
  UBYTE  meUsed;                  /* used number of ME storage  (counter)  */
  UBYTE  smsParamRecLen;          /* record length of EF(SMSP) */
  UBYTE  smsParamMaxRec;          /* number of records in EF(SMSP) */
  UBYTE  snd_msg_ref;             /* last used message reference (for +CMGC) */
} T_ACI_SMS_PARAMETER;


typedef struct SMSCBMParameter
{
  T_OWN  cbchOwner;               /* identifies the used set for a
                                     CBCH request */
  UBYTE  cbmHndl;                 /* CBCH message handling */
  UBYTE  cbmMode;                 /* CBCH message type modus */
  USHORT msgId[MAX_IDENTS];       /* CBCH message identifier */
  UBYTE  dcsId[MAX_IDENTS];       /* CBCH data coding scheme */
  UBYTE  cbmFoundIds;             /* actual found IDs */
  UBYTE  cbmSIMmaxId;             /* number IDs storable on SIM */
  UBYTE  cbmSIMmaxIdRge;          /* number ID ranges storable on SIM */
  UBYTE  IMSI [MAX_IMSI];         /* IMSI */
#ifdef SIM_TOOLKIT
  USHORT CBDtaDwnlIdent[MAX_IDENTS_SAT]; /* Identifiers for SAT CB data Dwn */
  UBYTE  CBDtaDwnlFoundIds;             /* actual found IDs */
  UBYTE  cbmSIMmaxSATId;             /* number SAT IDs storable on SIM */
#endif /* of SIM_TOOLKIT */
#ifdef FF_HOMEZONE
  UBYTE  hzMode;                  /* Activation mode of homezone feature */
  UBYTE  hzDcs;                   /* data coding sceme for homezone CBM  */
  UBYTE  hzTimeout;               /* timeout period for homezone CBM     */
#endif /* FF_HOMEZONE */
} T_SMS_CBM_PARAMETER;


/* this structure holds pointers to data to encode */
typedef struct TpDataUnit        
{
  T_TP_SUBMIT  *tp_submit;
  T_TP_DELIVER *tp_deliver;
  T_TP_COMMAND *tp_command;
  T_rp_addr sc_addr;
} T_TP_DATA_UNIT;


typedef struct SMSShrdParm
{
  T_OWN  owner;                   /* identifies the used set */

  T_SMS_SET_PRM *pSetPrm[OWN_SRC_MAX];/* possible sets */
  USHORT rslt;                    /* result of operation */ 
#if defined DEBUG_ACI
  char   testPrm[TEST_STR_LEN];   /* test parameter */
#endif
  UBYTE  smsStat;                 /* SMS status */
  UBYTE  rdMode;                  /* SMS read mode */
  UBYTE  mtHndl;                  /* mt message handling */
  UBYTE  srHndl;                  /* status report handling */
  UBYTE  accessEnabled;           /* access condx for SMS commands */
  SHORT  prmRdSeq;                /* Status of SMS Parameters read */

  UBYTE  mem1;                    /* prefered memory for mem1 (in PSA type)*/
  UBYTE  mem2;                    /* prefered memory for mem2 (in PSA type)*/
  UBYTE  mem3;                    /* prefered memory for mem3 (in PSA type)*/ 
  UBYTE  index;                   /* Index of the memory location to be deleted */
  UBYTE  status;                  /* This is a delete flag, depending on which
                                     multiple recods of the same status like
                                     all read, all sent, all unsent are
                                     deleted. */
  UBYTE  cnma_ack_expected;       /* +CNMA acknowlegdement expected */
#ifdef REL99
  UBYTE  auto_repeat_flag;           /* Store whether auto retransmission is
                                        enabled or not */
  BOOL   is_msg_present_for_retrans; /* Store whether message is available for 
                                        manual retransmission */
#endif
  T_ACI_CMD_SRC uiInternalSmsStorage;    /* indicates to the SMSStoCnf() --> no return values */

  T_ACI_CMD_SRC smsSrcId;          /* The ID if the source interested in unsolicited SMS indications */

  T_ACI_CSMS_SERV CSMSservice;    /* +CSMS parameters */ 
  T_ACI_CSMS_SUPP CSMSmt;
  T_ACI_CSMS_SUPP CSMSmo;
  T_ACI_CSMS_SUPP CSMSbm;

  T_ACI_CNMI_MT  CNMImt;          /* +CNMI parameters */
  T_ACI_CNMI_BM  CNMIbm;
  T_ACI_CNMI_DS  CNMIds;

  
  T_ENT_STAT2     
         smsEntStat;              /* */

  T_ACI_SMS_PARAMETER 
         aci_sms_parameter;       /* */

  T_SMS_CBM_PARAMETER
         cbmPrm;                  /* */

  T_TP_DATA_UNIT tpdu;

  T_ACI_CMGL_SM *pDecMsg;         /* pointer to decoded message */

#ifdef SIM_TOOLKIT
  char   fuRef;                   /* Reference for File Update */
#endif /* of SIM_TOOLKIT */

  /* points to reply call-back */
  union
  {
    T_CMSS_FCT *cmss;
    T_CMGS_FCT *cmgs;
    T_CMGC_FCT *cmgc;
    T_CMGR_FCT *cmgr;
    T_CMGW_FCT *cmgw;
    T_CMGD_FCT *cmgd;
    T_CMGMDU_FCT *cmgmdu;
#ifdef REL99
    T_CMGRS_FCT *cmgrs;
#endif
  } rplyCB;

  /* points to CMS error call-back */
  T_ERROR_FCT *errorCB;

#ifdef FF_MMI_RIV
  T_ACI_PERC_SMBS_MOD  perccmgf_smbs_mode; /* Enable SMBS: force presenting of PDU to SMBS */
#endif /* of FF_MMI_RIV */
  UBYTE  CMMSmode;               /* Mode of CMMS command */

#ifdef FF_ATI_BAT

  /*
  *   !!! For test purposes only !!! 
  *       easier method of conveying alphanumeric data from BAT to ATI.
  */
  UBYTE alpha_len;
  CHAR alpha[BAT_MAX_CMT_ALPHA+1];

#endif

#ifdef FF_CPHS_REL4
  T_ACI_MWIS_MWI MWISdata;
#endif

} T_SMS_SHRD_PRM;



/*==== PROTOTYPES =================================================*/

void  psaSMS_Init ( void );
void  psaSMS_InitParams ( void );

/*==== EXPORT =====================================================*/

#ifdef PSA_SMSF_C

GLOBAL T_SMS_SHRD_PRM      smsShrdPrm;
#else

EXTERN T_SMS_SHRD_PRM      smsShrdPrm;
#endif /* PSA_SMSF_C */

EXTERN SHORT psaSMS_ReadReq ( UBYTE mem_type,  USHORT rec_num, 
                              UBYTE read_mode, T_ACI_SMS_STAT stat );

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
/* ACI-ENH-19450: Added for %CPRMS command */ 
EXTERN void psaSMS_PauseReq ();
EXTERN void psaSMS_ResumeReq ();
EXTERN void psaSMS_QueryReq ( UBYTE query_type );
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */

#ifdef FF_HOMEZONE
EXTERN SHORT psaMMI_homezone_req( void );
#endif /* FF_HOMEZONE */

#endif /* PSA_SMS_H */

/*==== EOF =======================================================*/

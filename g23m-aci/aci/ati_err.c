/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  ATI
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
|  Purpose :  AT Command Interpreter: Error handling.
+-----------------------------------------------------------------------------
*/

/*********************************/
/* General Error Handling in ACI */
/*********************************/

#ifndef ATI_ERR_C
#define ATI_ERR_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#undef DUMMY_ATI_STRINGS

#include "aci_all.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "l4_tim.h"

#include "aci.h"
#include "aci_mem.h"

#include "aci_lst.h"
#include "ati_int.h"

#ifdef FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /*FF_ATI_BAT*/

LOCAL CHAR *AT_Fail_Error(void);

EXTERN T_ACI_ERR_DESC aciErrDesc;

LOCAL CHAR ErrBuff[MAX_CMD_LEN];

typedef struct
{
  CHAR        *msg;
  CHAR    *nmb;
  AtErrCode   code;
} AtErrTable;

LOCAL const AtErrTable tabAtError[]=
{
  "OK",          "0",   atOk,
  "CONNECT",     "1",   atConnect,
  "RING",        "2",   atRing,
  "NO CARRIER",  "3",   atNoCarrier,
  "NO DIALTONE", "6",   atNoDialtone,
  "BUSY",        "7",   atBusy,
  "NO ANSWER",   "8",   atNoAnswer,
  "CONNECT 9600","15",  atConnect1,
  /* this has to be the last entry */
  "ERROR",       "4",   atError
};

GLOBAL CHAR *disable_or_numeric(const CHAR* key,LONG e) /* e is actually enum...*/
{
  UBYTE srcId = srcId_cb;
  
  if(ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Disable)
  {
    cmdAtError(atError);
  }
  else if(ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Numeric)
  {
    sprintf(cmdErrStr,"%s ERROR: %d",key,e);
  }
  return cmdErrStr;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cmdExtError        |
+--------------------------------------------------------------------+

  PURPOSE : handle extended errors

*/

LOCAL const struct T_ATI_EXT_ERR /* error code - error message mapping */
{
  const CHAR*         msg;  /* error message */
  const T_ACI_EXT_ERR code; /* error code    */
  const ULONG displ_code;  /* error code to be display */
}

err[] =
{
"parameter not allowed",                  EXT_ERR_Parameter,       EXT_ERR_Parameter,
"data corrupted",                         EXT_ERR_DataCorrupt,     EXT_ERR_DataCorrupt,
"internal error",                         EXT_ERR_Internal,        EXT_ERR_Internal,
"call table full",                        EXT_ERR_CallTabFull,     EXT_ERR_CallTabFull,
"service table full",                     EXT_ERR_SrvTabFull,      EXT_ERR_SrvTabFull,
"call not found",                         EXT_ERR_CallNotFound,    EXT_ERR_CallNotFound,
"no data-call supported",                 EXT_ERR_NoDataCallSup,   EXT_ERR_NoDataCallSup,
"one call on hold",                       EXT_ERR_OneCallOnHold,   EXT_ERR_OneCallOnHold,
"hold call not supported for this type",  EXT_ERR_CallTypeNoHold,  EXT_ERR_CallTypeNoHold,
"number not allowed by FDN",              EXT_ERR_FdnCheck,        EXT_ERR_FdnCheck,
"number not allowed by BDN",              EXT_ERR_BdnCheck,        EXT_ERR_BdnCheck,
"parallel USSD not supported",            EXT_ERR_ParallelUSSD,    EXT_ERR_ParallelUSSD,
"fax minimum speed condition",            EXT_ERR_FaxMinSpeedCond, EXT_ERR_FaxMinSpeedCond,
"conflict with command details",          EXT_ERR_CmdDetailsSAT,   EXT_ERR_CmdDetailsSAT,
"IMEI illegal",                           EXT_ERR_IMEICheck,       EXT_ERR_IMEICheck,
"not allowed by ALS-Lock",                EXT_ERR_AlsLock,         EXT_ERR_AlsLock,
#ifdef REL99
"Last failed message not present",        EXT_ERR_FailedMsgNotPresent,EXT_ERR_FailedMsgNotPresent,
#endif
#if defined FF_EOTD
"Command not supported",                                        EXT_ERR_LCS_CmdNotSup,          0,
"Command not recognised/out of range",                          EXT_ERR_LCS_CmdNotRec,          1,
"LCS Client ID not recognised/out of range",                    EXT_ERR_LCS_CLPSClientNotRec,   2,
"Interval attribute out of range",                              EXT_ERR_LCS_IntervalNotSup,     3,
"Repeat attribute not supported/ out of range",                 EXT_ERR_LCS_RepeatNotSup,       4,
"Send request type not recognised / out of range",              EXT_ERR_LCS_SendReqTyNotRec,    2,
"User confirmation request type not recognised / out of range", EXT_ERR_LCS_UsConfReqTyNotRec,  3,
"LCS Client ID not recognised/out of range",                    EXT_ERR_LCS_CLSRClientIdNotRec, 4,
"Circuit switched call number out of range",                    EXT_ERR_LCS_CSCallNumNotSup,    5,
#endif /* FF_EOTD */

#ifdef SIM_PERS
"BlockedNetwork lock - PIN required",            EXT_ERR_BlockedNetworkPersPinReq,	       EXT_ERR_BlockedNetworkPersPinReq,
"BlockedNetwork lock - PUK required",            EXT_ERR_BlockedNetworkPersPukReq,          EXT_ERR_BlockedNetworkPersPukReq,
"Busy Error",                                  EXT_ERR_Busy,       EXT_ERR_Busy,                                     /* Penalty timer is running */
"MEPD not present",			   EXT_ERR_NoMEPD,	   EXT_ERR_NoMEPD,									 /* Penalty timer is running */
#endif

"error unknown",                          EXT_ERR_Unknown,         EXT_ERR_Unknown,
/* this has to be the last entry */
"other error",                            EXT_ERR_NotPresent,      0xFFFFFFFF
};

GLOBAL CHAR *cmdExtError (T_ACI_EXT_ERR e)
{
  SHORT idx=0;
  T_ACI_ERR_DESC aed;
  UBYTE srcId = srcId_cb;

  if ( e EQ EXT_ERR_NotPresent )
  {
    if (((aed = qAT_ErrDesc()) >> 16) EQ ACI_ERR_CLASS_Ext)
    {
      e = (T_ACI_EXT_ERR)(aed & 0xFFFF);
    }
    else
    {
      e = EXT_ERR_Unknown;
    }
  }
  *ErrBuff = '\0';
  cmdErrStr = ErrBuff;

  if (ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Verbose OR
      ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Numeric)
  {
    /* search the array for the index of the notified failure */
    while ( err[idx].code NEQ EXT_ERR_NotPresent AND
            err[idx].code NEQ e )
    {
       idx++;
    }
  }
  else /* Catch some Extended Error Codes and map them to more suitable codes */
  {
    switch (e)
    {
      case EXT_ERR_CallTabFull:
      case EXT_ERR_FdnCheck:
      case EXT_ERR_BdnCheck:
        return(cmdAtError(atNoCarrier));
    }
  }
  if (ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Verbose)
  {
    strcat(cmdErrStr,"+EXT ERROR: ");
    strcat( cmdErrStr, err[idx].msg );
  }
  return(disable_or_numeric("+EXT", err[idx].displ_code));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cmdCmeError        |
+--------------------------------------------------------------------+

  PURPOSE : handle Mobile Equipment Errors

*/
#ifdef GPRS
#define GPRS_ERR(a,b) a,b,
#else
#define GPRS_ERR(a,b)
#endif  /* GPRS */

LOCAL const struct T_ATI_CME_ERR /* error code - error message mapping */
{
  const CHAR*         msg;  /* error message */
  const T_ACI_CME_ERR code; /* error code    */
}
  error[] =
{
"phone failure",                        CME_ERR_PhoneFail,
"no connection to phone",               CME_ERR_NoConnect,
"phone-adaptor link reserved",          CME_ERR_LinkRes,
"operation not allowed",                CME_ERR_OpNotAllow,
"operation not supported",              CME_ERR_OpNotSupp,
"PH-SIM PIN required",                  CME_ERR_PhSimPinReq,
"SIM not inserted",                     CME_ERR_SimNotIns,
"SIM PIN required",                     CME_ERR_SimPinReq,
"SIM PUK required",                     CME_ERR_SimPukReq,
"SIM failure",                          CME_ERR_SimFail,
"SIM busy",                             CME_ERR_SimBusy,
"SIM wrong",                            CME_ERR_SimWrong,
"incorrect password",                   CME_ERR_WrongPasswd,
"SIM PIN2 required",                    CME_ERR_SimPin2Req,
"SIM PUK2 required",                    CME_ERR_SimPuk2Req,
"memory full",                          CME_ERR_MemFull,
"invalid index",                        CME_ERR_InvIdx,
"not found",                            CME_ERR_NotFound,
"memory failure",                       CME_ERR_MemFail,
"text string too long",                 CME_ERR_TxtToLong,
"invalid characters in text string",    CME_ERR_InvalidTxtChar,
"dial string too long",                 CME_ERR_DialToLong,
"invalid characters in dial string",    CME_ERR_InvDialChar,
"no network service",                   CME_ERR_NoServ,
"network timeout",                      CME_ERR_Timeout,
"network not allowed - emergency calls only", CME_ERR_LimServ,
"Network lock - PIN required",  CME_ERR_NetworkPersPinReq,                      /* PIN to change network personalisation required */
"Network lock - PUK required",  CME_ERR_NetworkPersPukReq,                    /* network personalisation PUK is required */
"Subnetwork lock - PIN required",  CME_ERR_NetworkSubsetPersPinReq,       /* keycode to change nw subset personalisation required */
"Subnetwork lock - PUK required",  CME_ERR_NetworkSubsetPersPukReq,     /* network subset  personalisation PUK is required */
"Provider lock - PIN required",  CME_ERR_ProviderPersPinReq,                     /* keycode to change service provider personal. required */
"Provider lock - PUK required",  CME_ERR_ProviderPersPukReq,                   /* service provider personalisation PUK is required */
"Corporate lock - PIN required",  CME_ERR_CorporatePersPinReq,                /* keycode to change corporate personalisation required */
"Corporate lock - PUK required",  CME_ERR_CorporatePersPukReq,              /* corporate personalisation PUK is required */
"unknown",                              CME_ERR_Unknown,                                    /* ATTENTION! If you insert new entries before CME_ERR_Unknown, remember to increase NBR_CME_NORM_ERR */
GPRS_ERR("illegal MS",                  CME_ERR_GPRSBadMs)
GPRS_ERR("illegal ME",                  CME_ERR_GPRSBadMe)
GPRS_ERR("GPRS service not allowed",    CME_ERR_GPRSNoService)
GPRS_ERR("PLMN not allowed",            CME_ERR_GPRSBadPlmn)
GPRS_ERR("Location not allowed",        CME_ERR_GPRSBadLoc)
GPRS_ERR("Roaming not allowed in Location Area",   CME_ERR_GPRSNoRoam)
GPRS_ERR("GPRS service option not supported",      CME_ERR_GPRSSerOptNsup)
GPRS_ERR("requested service option not subscribed",CME_ERR_GPRSSerOptNsub)
GPRS_ERR("service option temporarily out of order",CME_ERR_GPRSSerOptOOO)
GPRS_ERR("unspecified GPRS error",      CME_ERR_GPRSUnspec)
GPRS_ERR("PDP authorisation error",     CME_ERR_GPRSPdpAuth)
GPRS_ERR("invalid module class",        CME_ERR_GPRSBadModClass)
"failed to abort",                      CME_ERR_FailedToAbort,
"ACM reset needed",                     CME_ERR_AcmResetNeeded,
"sim extension memory full",            CME_ERR_SimNoExtAvail,
"sim reset needed",                     CME_ERR_SimResetNeeded,
"Aborted by Network",                 CME_ERR_AbortedByNetwork,
/* this has to be the last entry */
"other error",                          CME_ERR_NotPresent
};

GLOBAL CHAR *cmdCmeError (T_ACI_CME_ERR e)
{
  SHORT idx=0;
  T_ACI_ERR_DESC aed;
  UBYTE srcId = srcId_cb;

  *ErrBuff = '\0';     /* initialization */
  cmdErrStr = ErrBuff;

  if ( e EQ CME_ERR_NotPresent )
  {
    if (((aed = qAT_ErrDesc()) >> 16) EQ ACI_ERR_CLASS_Cme)
    {
      e = (T_ACI_CME_ERR)(aed & 0xFFFF);
    }
    else
    {
      e = CME_ERR_Unknown;
    }
  }
  else if( e EQ CME_ERR_Unknown )
  {
    return(AT_Fail_Error());
  }

  if (ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Verbose)
  {
    strcat(cmdErrStr,"+CME ERROR: ");

    /* search for the description of the notified error */
    while ( error[idx].code NEQ CME_ERR_NotPresent AND
            error[idx].code NEQ e )
    {
      idx++;
    }
    strcat( cmdErrStr, error[idx].msg );
  }
  return(disable_or_numeric("+CME",e));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cmdCmsError        |
+--------------------------------------------------------------------+

  PURPOSE : handle Short Message Errors

*/

LOCAL const struct T_ATI_CMS_ERR /* error code - error message mapping */
{
  const CHAR*         msg;  /* error message */
  const T_ACI_CMS_ERR code; /* error code    */
}
  cms_err[] =
{
  "unassigned (unallocated) number",               CMS_ERR_UnAllocNum,
  "operator determined barring",                   CMS_ERR_OpDetermBarr,
  "call barred",                                   CMS_ERR_CallBarr,
  "short message transfer rejected",               CMS_ERR_TransReject,
  "destination out of service",                    CMS_ERR_DestOutOfServ,
  "unidentified subscriber",                       CMS_ERR_UnidentSubsc,
  "facility rejected",                             CMS_ERR_FacReject,
  "unknown subscriber",                            CMS_ERR_UnKnownSubsc,
  "network out of order",                          CMS_ERR_NetOutOfOrder,
  "temporary failure",                             CMS_ERR_TempFail,
  "congestion",                                    CMS_ERR_Congestion,
  "resources unavailable, unspecified",            CMS_ERR_ResUnAvail,
  "requested facility not subscribed",             CMS_ERR_FacNotSubscr,
  "requested facility not implemented",            CMS_ERR_FacNotImpl,
  "invalid short message transfer ref. value",     CMS_ERR_TransRefInval,
  "semantically incorrect message",                CMS_ERR_InValSM,
  "invalid mandatory information",                 CMS_ERR_InValManInfo,
  "message type non-existent or not implemented",  CMS_ERR_MsgTypNotExist,
  "message not compatible with SM protocol state", CMS_ERR_MsgNotCompatible,
  "information element non-existent or not impl.", CMS_ERR_InfoElemNotImpl,
  "protocol error, unspecified",                   CMS_ERR_ProtErr,
  "interworking, unspecified",                     CMS_ERR_InterWrkUnSpec,
  "telematic interworking not supported",          CMS_ERR_TlmtkNotSup,
  "short message type 0 not supported",            CMS_ERR_SM0NotSup,
  "cannot replace short message",                  CMS_ERR_CantReplceSM,
  "unspecified TP-PID error",                      CMS_ERR_UnSpecPIDErr,
  "data coding scheme (alphabet) not supported",   CMS_ERR_DcsNotSup,
  "message class not supported",                   CMS_ERR_MsgClassNotSup,
  "unspecified TP-DCS error",                      CMS_ERR_UnSpecTpDcs,
  "command cannot be actioned",                    CMS_ERR_CmdNotAct,
  "command unsupported",                           CMS_ERR_CmdUnSup,
  "unspecified TP-Command error",                  CMS_ERR_UnSpecTpCmd,
  "TPDU not supported",                            CMS_ERR_TpduUnSup,
  "SC busy",                                       CMS_ERR_ScBsy,
  "no SC subscription",                            CMS_ERR_NoScSubsc,
  "SC system failure",                             CMS_ERR_ScSysFail,
  "invalid SME address",                           CMS_ERR_InValSme,
  "destination SME barred",                        CMS_ERR_DestSmeBarr,
  "SM rejected-duplicate SM",                      CMS_ERR_SmRejctDuplSm,
  "TP-VPF not supported",                          CMS_ERR_SmTPVPFNotSup,
  "TP-VP not supported",                           CMS_ERR_SmTPVPNotSup,
  "SIM SMS storage full",                          CMS_ERR_SimSmsStorFull,
  "no SMS storage capability in SIM",              CMS_ERR_NoStorInSim,
  "error in MS",                                   CMS_ERR_ErrInMs,
  "memory capacity exceeded",                      CMS_ERR_MemCabExcee,
  "unspecified error cause",                       CMS_ERR_UnSpecErr,
  "ME failure",                                    CMS_ERR_MeFail,
  "SMS service of ME reserved",                    CMS_ERR_ServRes,
  "operation not allowed",                         CMS_ERR_OpNotAllowed,
  "operation not supported",                       CMS_ERR_OpNotSup,
  "invalid PDU mode parameter",                    CMS_ERR_InValPduMod,
  "invalid text mode parameter",                   CMS_ERR_InValTxtMod,
  "SIM not inserted",                              CMS_ERR_SimNotIns,
  "SIM PIN required",                              CMS_ERR_SimPinReq,
  "PH-SIM PIN required",                           CMS_ERR_PhSimPinReq,
  "SIM failure",                                   CMS_ERR_SimFail,
  "SIM busy",                                      CMS_ERR_SimBsy,
  "SIM wrong",                                     CMS_ERR_SimWrong,
  "SIM PUK required",                              CMS_ERR_SimPukReq,
  "SIM PIN2 required",                             CMS_ERR_SimPin2Req,
  "SIM PUK2 required",                             CMS_ERR_SimPuk2Req,
  "memory failure",                                CMS_ERR_MemFail,
  "invalid memory index",                          CMS_ERR_InValMemIdx,
  "memory full",                                   CMS_ERR_MemFull,
  "SMSC address unknown",                          CMS_ERR_SmscAdrUnKnown,
  "no network service",                            CMS_ERR_NoNetServ,
  "network timeout",                               CMS_ERR_NetTimeOut,
  "no +CNMA acknowledegment expected",             CMS_ERR_NoCnmaAckExpect,
  "unknown error",                                 CMS_ERR_UnknownErr,
  "failed to abort",                               CMS_ERR_FailedToAbort,
  /* this must be the last entry */
  "other error",                                   CMS_ERR_NotPresent
};

#define NBR_CMS_ERR 69
#define MAX_CMS_LTH 60

GLOBAL char* cmdCmsError ( T_ACI_CMS_ERR e )
{
  int i = 0;
  T_ACI_ERR_DESC aed;
  UBYTE srcId = srcId_cb;  

  if ( e EQ CMS_ERR_NotPresent )
  {
    if (((aed = qAT_ErrDesc()) >> 16) EQ ACI_ERR_CLASS_Cms)
    {
      e = (T_ACI_CMS_ERR)(aed & 0xFFFF);
    }
    else                        /* no SMS error code */
    {
      e = CMS_ERR_UnknownErr;
    }
  }

  {
    *ErrBuff = '\0';        /* build message */
    cmdErrStr = ErrBuff;

    if ( ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Verbose )
    {
      strcat( cmdErrStr, "+CMS ERROR: " );

      while ( cms_err[i].code NEQ CMS_ERR_NotPresent AND
              cms_err[i].code NEQ e )
      {
        i++;
      }
      strcat( cmdErrStr, cms_err[i].msg );
    }
    else if ( ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Disable )
    {
      strcat ( cmdErrStr, "ERROR" );
    }
    else if ( ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Numeric )
    {
      sprintf ( cmdErrStr, "+CMS ERROR: %d", e );
    }
  }
  return cmdErrStr;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCEER         |
+--------------------------------------------------------------------+

  PURPOSE : Extented error report
*/

typedef struct
  {
    char        * desc;
    T_ACI_CEER    rpt;
  } CEER_TABLE;

/* Defined by standards CEER cause description table */
const CEER_TABLE StdCEERDesc[]=
{
   {"unassigned number",                  CEER_Unassign },
   {"no route to destination",            CEER_NoRoute },
   {"channel unacceptable",               CEER_ChanUnaccept },
   {"operator determined barring",        CEER_Barred },
   {"normal call clearing",               CEER_CallClear },
   {"user busy",                          CEER_UserBusy },
   {"no user responding",                 CEER_NoResponse },
   {"user alerting, no answer",           CEER_AlertNoAnswer },
   {"call rejected",                      CEER_CallReject },
   {"number changed",                     CEER_NumChanged },
   {"non selected user clearing",         CEER_UserClear },
   {"destination out of order",           CEER_DestOutOfOrder },
   {"invalid number format",              CEER_NumFormat },
   {"facility rejected",                  CEER_FacilityReject },
   {"response to status enquiry",         CEER_StatusEnquiry },
   {"normal, unspecified",                CEER_Unspecified },
   {"no channel available",               CEER_NoChanAvail },
   {"network out of order",               CEER_NetworkOutOfOrder },
   {"temporary failure",                  CEER_TempFailure },
   {"switching equipment congestion",     CEER_SwitchCongest },
   {"access information discarded",       CEER_InfoDiscard },
   {"requested channel unavailable",      CEER_ReqChanUnavail },
   {"recources unavailable",              CEER_ResourceUnavail },
   {"quality of service unavailable",     CEER_QOS_Unavail },
   {"requested facility unsubscribed",    CEER_FAC_Unsubscr },
   {"incoming calls barred within CUG",   CEER_BarredInCUG },
   {"bearer capability not authorized",   CEER_BearerCapNotAuth },
   {"bearer capability not available",    CEER_BearerCapUnavail },
   {"service not available",              CEER_ServUnavail },
   {"bearer service not implemented",     CEER_BearerNotImpl },
   {"ACM reached ACM maximum",            CEER_ACM_Max },
   {"facility not implemented",           CEER_FAC_NotImpl },
   {"only restricted bearer cap. avail.", CEER_RestrictBearerCap },
   {"service not implemented",            CEER_ServNotImpl },
   {"invalid TI",                         CEER_InvalidTI },
   {"no member of CUG",                   CEER_UserNotInCUG },
   {"incompatible destination",           CEER_IncompatDest },
   {"invalid transit network selection",  CEER_InvalidTransNet },
   {"incorrect message",                  CEER_IncorrMsg },
   {"invalid mandatory information",      CEER_InvalidMandInfo },
   {"message type not implemented",       CEER_MsgTypeNotImpl },
   {"message type incompatible",          CEER_MsgTypeIncomp },
   {"info element not implemented",       CEER_InfoElemNotImpl },
   {"conditional info element error",     CEER_CondInfoElem },
   {"message incompatible",               CEER_MsgIncomp },
   {"recovery on time expiry",            CEER_Timer },
   {"protocol error",                     CEER_Protocol },
   {"interworking error",                 CEER_Interworking },
   {"bearer service not available",       CEER_ReqBearerServNotAvail },
   {"no TI available",                    CEER_NoTransIdAvail },
   {"timer 303 expiry",                   CEER_Timer303 },
   {"establishment failure",              CEER_EstabFail },
   {"no error",                           CEER_NoError },
   {"operation failed",                   CEER_Failed },
   {"timeout",                            CEER_Timeout },
   {"bearer service not compatible",      CEER_BearerServNotCompat },
#ifdef GPRS
   {"unsuccessful GPRS attach",           CEER_FailedGPRSAttach },
   {"unsuccessful PDP context activation",CEER_FailedGPRSContextAct },
   {"GPRS detach",                        CEER_GPRSDetach },
   {"GPRS PDP context deactivation",      CEER_GPRSDeactivation },
#endif /* GPRS */
   /* these have  to be the last entries */
   {"no error",                           CEER_NotPresent },
   /* last entry for at+ceer */
   { NULL,                                CEER_NotPresent}
};

/* Proprietary ACI CEER cause description table */
const CEER_TABLE pACI_CEERDesc[]=
{
   {"ACM reached ACM maximum", (T_ACI_CEER)P_CEER_ACMMaxReachedOrExceeded },
   {"number not in FDN list",  (T_ACI_CEER) P_CEER_InvalidFDN },
   /* these have  to be the last entries */
   {"no error",                            CEER_NotPresent },
   /* last entry for at+ceer */
   { NULL,                                 CEER_NotPresent}
};

/* Proprietary MM CEER cause description table */
const CEER_TABLE pMM_CEERDesc[]=
{
  {"IMSI not present in HLR", (T_ACI_CEER)P_MM_CEER_IMSINotInHLR},
  {"no service",              (T_ACI_CEER)P_MM_CEER_NoService },
  /* these have  to be the last entries */
  {"no error",                            CEER_NotPresent },
  /* last entry for at+ceer */
  { NULL,                                 CEER_NotPresent}
};

/* Proprietary SS CEER cause description table */
const CEER_TABLE pSS_CEERDesc[]=
{
  {"Unknown Subscriber",     (T_ACI_CEER)P_SS_CEER_UnknownSubscriber },
  {"Illegal Subscriber",                           (T_ACI_CEER)P_SS_CEER_IllegalSubscriber },
  {"Bearer service not provisioned",               (T_ACI_CEER)P_SS_CEER_BearerSvcNotProv },
  {"Teleservice not provisioned",                  (T_ACI_CEER)P_SS_CEER_TeleSvcNotProv },
  {"Illegal Equipment",                            (T_ACI_CEER)P_SS_CEER_IllegalEquipment },
  {"Call barred",                                  (T_ACI_CEER)P_SS_CEER_CallBarred },
  {"Illegal supplementary service operation",  (T_ACI_CEER)P_SS_CEER_IllegalSSOperation },
  {"SS error status",       (T_ACI_CEER)P_SS_CEER_SSerrStatus },
  {"SS not available",       (T_ACI_CEER)P_SS_CEER_SSNotAvail },
  {"SS subscript violation",  (T_ACI_CEER)P_SS_CEER_SSSubsViolation },
  {"SS incompatible",                              (T_ACI_CEER)P_SS_CEER_SSIncomp },
  {"Facility not supported", (T_ACI_CEER)P_SS_CEER_FacNotSupported },
  {"Absent subscriber",   (T_ACI_CEER)P_SS_CEER_AbsentSubs },
  {"System failure",   (T_ACI_CEER)P_SS_CEER_SystenFail },
  {"Data Missing",                                 (T_ACI_CEER)P_SS_CEER_DataMissing },
  {"Unexpected Data Value",                        (T_ACI_CEER)P_SS_CEER_UnexpectData },
  {"Password registration failure",                (T_ACI_CEER)P_SS_CEER_PwdRegFail },
  {"Negative password check",  (T_ACI_CEER)P_SS_CEER_NegPwdCheck},
  {"Number of password attempts violation",  (T_ACI_CEER)P_SS_CEER_NumPwdViolation },
  {"Unknown alphabet",                             (T_ACI_CEER)P_SS_CEER_UnknownAlpha },
  {"Unstructured supplementary service data busy", (T_ACI_CEER)P_SS_CEER_UssdBusy },
  {"Number of multiparty participants exceeded",   (T_ACI_CEER)P_SS_CEER_MaxNumMptyExceed },
  {"Resource not available", (T_ACI_CEER)P_SS_CEER_ResourceNotAvail },

  /* these have  to be the last entries */
  {"no error",                            CEER_NotPresent },
  /* last entry for at+ceer */
  { NULL,                                 CEER_NotPresent}
};
#define MAX_CEER_DESC_LTH (50)
#define MAX_CEER_VAL_LTH (11)

GLOBAL void getCeerDesc(USHORT cause, char *desc)
{
  SHORT  idx;
  int    val;

  TRACE_FUNCTION("getCeerDesc()");

  memset(desc,0,MAX_CEER_DESC_LTH);

  if ( GET_CAUSE_DEFBY(cause) EQ DEFBY_STD )
  {
    TRACE_EVENT_P1("Cause: %d, defined by standard", cause);
    val = GET_CAUSE_VALUE(cause);

    if ( val EQ NOT_PRESENT_8BIT )
    {
      val = CEER_NotPresent;
    }
    TRACE_EVENT("Get desc from Standard CEER desc table");
    for( idx = 0; StdCEERDesc[idx].desc NEQ NULL; idx++ )
    {
      if ( StdCEERDesc[idx].rpt EQ val )
      {
        memcpy(desc, StdCEERDesc[idx].desc, strlen(StdCEERDesc[idx].desc) );
        return;
      }
    }
  }

  if ( GET_CAUSE_DEFBY(cause) EQ DEFBY_CONDAT )
  {
    if ( GET_CAUSE_ORIGIN_ENTITY(cause) EQ MNCC_ACI_ORIGINATING_ENTITY )
    {
      TRACE_EVENT_P1("Cause: %d, is ACI proprietary", cause);
      val = GET_CAUSE_VALUE(cause);

      if ( val EQ NOT_PRESENT_8BIT )
      {
        val = P_CEER_NotPresent;
      }
      TRACE_EVENT("Get desc from proprietary ACI CEER desc table");
      for( idx = 0; pACI_CEERDesc[idx].desc NEQ NULL; idx++ )
      {
        if ( pACI_CEERDesc[idx].rpt EQ val )
        {
          memcpy(desc, pACI_CEERDesc[idx].desc, strlen(pACI_CEERDesc[idx].desc) );
          return;
        }
      }
    }
    else if ( GET_CAUSE_ORIGIN_ENTITY(cause) EQ MM_ORIGINATING_ENTITY )
    {
      TRACE_EVENT_P1("Cause: %d, is MM proprietary", cause);
      val = GET_CAUSE_VALUE(cause);

      if ( val EQ NOT_PRESENT_8BIT )
      {
        val = P_MM_CEER_NotPresent;
      }
      TRACE_EVENT("Get desc from proprietary MM CEER desc table");
      for( idx = 0; pMM_CEERDesc[idx].desc NEQ NULL; idx++ )
      {
        if ( pMM_CEERDesc[idx].rpt EQ val )
        {
          memcpy(desc, pMM_CEERDesc[idx].desc, strlen(pMM_CEERDesc[idx].desc) );
          return;
        }
      }
    }
    else if ( GET_CAUSE_ORIGIN_ENTITY(cause) EQ SS_ORIGINATING_ENTITY )
    {
      TRACE_EVENT_P1("Cause: %d, is SS proprietary", cause);
      val = GET_CAUSE_VALUE(cause);

      if ( val EQ NOT_PRESENT_8BIT )
      {
        val = P_SS_CEER_NotPresent;
      }
      TRACE_EVENT("Get desc from proprietary SS CEER desc table");
      for( idx = 0; pSS_CEERDesc[idx].desc NEQ NULL; idx++ )
      {
        if ( pSS_CEERDesc[idx].rpt EQ val )
        {
          memcpy(desc, pSS_CEERDesc[idx].desc, strlen(pSS_CEERDesc[idx].desc) );
          return;
        }
      }
    }
  }
}


GLOBAL T_ATI_RSLT atPlusCEER(char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("atPlusCEER()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params=BAT_CMD_SET_PLUS_CEER;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_set_plus_ceer = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else /* no FF_ATI_BAT */
  {
    char   desc[MAX_CEER_DESC_LTH];
    T_ACI_RETURN ret;
    USHORT cause;

    ret=qAT_PlusCEER( (T_ACI_CMD_SRC)srcId, &cause );
    if (ret EQ AT_CMPL)
    {
      /* check if description text is available*/
      getCeerDesc(cause, desc);

      if ( desc[0] EQ 0 )
      {
        /* no description text is available */
        sprintf( g_sa, "+CEER: %d,%d,%d,%d",
                 GET_CAUSE_DEFBY(cause), GET_CAUSE_ORIGSIDE(cause),
                 GET_CAUSE_ORIGIN_ENTITY(cause), GET_CAUSE_VALUE(cause));
      }
      else
      {
        /* description text is available */
        sprintf( g_sa, "+CEER: %d,%d,%d,%d,%s",
                 GET_CAUSE_DEFBY(cause), GET_CAUSE_ORIGSIDE(cause),
                 GET_CAUSE_ORIGIN_ENTITY(cause), GET_CAUSE_VALUE(cause),
                 desc );
      }
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      return ATI_CMPL;
    }
    else
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }
  }
#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCSSD      |
+--------------------------------------------------------------------+

  PURPOSE : Supplementary Service Diagnostic 
*/

GLOBAL T_ATI_RSLT atPercentCSSD(char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret;
  UBYTE ss_diag;
  CHAR *me  =  "%CSSD: ";

  TRACE_FUNCTION("atPercentCSSD()");

  ret = qAT_PercentCSSD( (T_ACI_CMD_SRC)srcId, &ss_diag);

  if (ret EQ AT_CMPL)
  {
    sprintf( g_sa, "%s%d", me, ss_diag );
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return ATI_CMPL;
  }
  else
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cmdCeerError       |
+--------------------------------------------------------------------+

  PURPOSE : handle CEER Errors

*/

GLOBAL CHAR *cmdCeerError(USHORT cause)
{
  T_ACI_ERR_DESC aed;
  char desc[MAX_CEER_DESC_LTH];
  UBYTE srcId = srcId_cb;
  
  *ErrBuff = '\0';        /* build message */
  cmdErrStr = ErrBuff;

  if ( GET_CAUSE_VALUE(cause) EQ NOT_PRESENT_8BIT )
  {
    if (((aed = qAT_ErrDesc()) >> 16) EQ ACI_ERR_CLASS_Ceer)
      cause = (T_ACI_CEER)(aed & 0xFFFF);
    else
      cause = CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_ACI_ORIGINATING_ENTITY,
                       CEER_NoError);
  }
  if (ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Verbose)
  {
    sprintf(cmdErrStr, "+CEER ERROR: %d,%d,%d,%d,",
            GET_CAUSE_DEFBY(cause), GET_CAUSE_ORIGSIDE(cause),
            GET_CAUSE_ORIGIN_ENTITY(cause), GET_CAUSE_VALUE(cause));

    getCeerDesc(cause, desc);

    if ( desc[0] NEQ 0 )
    {
      strcat( cmdErrStr, desc);
    }

  }
  return(disable_or_numeric("+CEER",cause));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cmdAtError         |
+--------------------------------------------------------------------+

  PURPOSE : handle standard AT Errors

*/

GLOBAL CHAR *cmdAtError (AtErrCode e)
{
  SHORT idx=0;

  *ErrBuff = '\0';
  cmdErrStr = ErrBuff;

  /* search for the description of the notified error */
  while ( tabAtError[idx].code NEQ atError AND
          tabAtError[idx].code NEQ e )
  {
    idx++;
  }
  sprintf(cmdErrStr,(at.s1415.atV) ? tabAtError[idx].msg : tabAtError[idx].nmb);
  return cmdErrStr;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : AT_FailError       |
+--------------------------------------------------------------------+

  PURPOSE : General handling of errors (either CME CMS CEER or EXT errors)
            when function returns AT_FAIL

*/
#define ACI_GET_ERR_CLASS() ((aciErrDesc & 0xFFFF0000) >> 16)
#define ACI_GET_ERR_VALUE()  (aciErrDesc & 0x0000FFFF)
#define ACI_RESET_ERR_DESC() (aciErrDesc = 0xFFFFFFFF)
#define ACI_VALID_ERR_DESC() (aciErrDesc NEQ 0xFFFFFFFF)

LOCAL CHAR *AT_Fail_Error(void)
{
  T_ACI_ERR_CLASS err_class;

  if ( ACI_VALID_ERR_DESC() )
  {
    err_class = (T_ACI_ERR_CLASS)ACI_GET_ERR_CLASS();
    switch(err_class)
    {
      case(ACI_ERR_CLASS_NotPresent):
        break;
      case(ACI_ERR_CLASS_Cme):
        {
          T_ACI_CME_ERR l_err;

          l_err = (T_ACI_CME_ERR)ACI_GET_ERR_VALUE();
          ACI_RESET_ERR_DESC();      /* needs to be reinitialized */
          return(cmdCmeError (l_err));
        }
      case(ACI_ERR_CLASS_Cms):
        {
          T_ACI_CMS_ERR l_err;

          l_err = (T_ACI_CMS_ERR)ACI_GET_ERR_VALUE();
          ACI_RESET_ERR_DESC();      /* needs to be reinitialized */
          return(cmdCmsError (l_err));
        }
      case(ACI_ERR_CLASS_Ceer):
        {
          T_ACI_CEER l_err;

          l_err = (T_ACI_CEER)ACI_GET_ERR_VALUE();
          ACI_RESET_ERR_DESC();      /* needs to be reinitialized */
          /* return(cmdCeerError((USHORT)err)); */
          return(cmdCeerError(l_err));
        }
      case(ACI_ERR_CLASS_Ext):
        {
          T_ACI_EXT_ERR l_err;

          l_err = (T_ACI_EXT_ERR)ACI_GET_ERR_VALUE();
          ACI_RESET_ERR_DESC();      /* needs to be reinitialized */
          return(cmdExtError (l_err));
        }
      default:
        break;
    }
  }
  *ErrBuff = '\0';
  cmdErrStr = ErrBuff;

  return (cmdCmeError(CME_ERR_NotPresent));
}

#endif /* ATI_ERR_C */

/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SSS
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
|  Purpose :  This module provides the set functions related to the
|             protocol stack adapter for SS.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_SSS_C
#define CMH_SSS_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci.h"
#include "aci_cmh.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#ifdef UART
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#include "ksd.h"
#include "psa.h"
#include "psa_ss.h"
#include "psa_sim.h"
#include "psa_sms.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_ss.h"
#include "cmh_sim.h"
#include "cmh_sms.h"
#include "cmh_cc.h"

#ifdef SIM_TOOLKIT
#include "psa_cc.h"
#include "psa_sat.h"
#include "cmh_sat.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "cmh_cc.h"
#endif /* of SIM_TOOLKIT */

#include "psa_util.h"
#ifdef GPRS
#include "gaci_cmh.h"
#endif /* GPRS */
#include "phb.h"


#include "aci_ext_pers.h"
#include "aci_slock.h"

#ifdef SIM_PERS
#include "general.h"  // inluded for UINT8 compilation error in sec_drv.h
#include "sec_drv.h"

EXTERN T_SEC_DRV_CONFIGURATION *cfg_data;
EXTERN T_SIM_MMI_INSERT_IND *last_sim_mmi_insert_ind;
#endif

#ifdef FF_PHONE_LOCK
 EXTERN  T_OPER_RET_STATUS aci_ext_set_phone_lock_satus(T_SEC_DRV_EXT_PHONE_LOCK_TYPE type,T_SEC_DRV_EXT_PHONE_LOCK_STATUS status,const char *passwd);
 EXTERN T_OPER_RET_STATUS aci_ext_set_phone_lock_key(const char *pOldKey,const char *pNewKey);
#endif

#if defined (SIM_PERS) || defined (FF_PHONE_LOCK)
 EXTERN T_ACI_RETURN cmhSS_check_oper_result(T_OPER_RET_STATUS result); 
#endif

/*==== CONSTANTS ==================================================*/
#define AT_TOA_DEF_INT    (129)  /* toa default for international no.*/
#define AT_TOA_DEF_NAT    (145)  /* toa default for national no.*/
#define AT_TOS_DEF        (128)  /* tos default */
#define AT_CFNRY_DEF_TIME (20)   /* default no reply time */




/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
#ifdef TI_PS_FF_AT_P_CMD_CUSCFG
EXTERN T_ACI_CUSCFG_PARAMS cuscfgParams;
#endif
GLOBAL SHORT Ext_USSD_Res_Pending_sId;
GLOBAL T_CUSDR_EXT_USSD_RES Ext_USSD_Res_Pending = CUSDR_EXT_USSD_RES_Not_Pending;

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : sAT_PlusCCFC             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CCFC AT command
            which is responsible to set the parameters for call
            forwarding supplementary services.

            <reason> : reason for CF.
            <mode>   : CF mode.
            <number> : number to be forwarded to.
            <type>   : type of number.
            <class>  : class of calls.
            <subaddr>: subaddress of number.
            <satype> : type of subaddress.
            <time>   : no reply time.
*/

GLOBAL T_ACI_RETURN sAT_PlusCCFC  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CCFC_RSN reason,
                                    T_ACI_CCFC_MOD mode,
                                    CHAR*          number,
                                    T_ACI_TOA*     type,
                                    T_ACI_CLASS    class_type,
                                    CHAR*          subaddr,
                                    T_ACI_TOS*     satype,
                                    SHORT          time)
{
  SHORT sId1, sId2;         /* holds service id */
  UBYTE ssCd;               /* holds ss code */
  UBYTE bst1, bst2;         /* holds basic service type */
  UBYTE bs1, bs2;           /* holds basic service */
  UBYTE ton, tos;           /* holds type of number/subaddress */
  UBYTE npi, oe;            /* holds numbering plan/odd.even indicator */
  BOOL  mltyTrnFlg = FALSE; /* flags for multi transaction */
  T_CLPTY_PRM *ccfc_cldPty;

 #ifdef SIM_TOOLKIT
  T_ACI_RETURN   retVal;
 #endif /* of SIM_TOOLKIT */
 
  TRACE_FUNCTION ("sAT_PlusCCFC");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  if( cmhPrm[srcId].ssCmdPrm.mltyTrnFlg NEQ 0 )
  {
    TRACE_EVENT("ssCmdPrm.mltyTrnFlg NEQ 0 !!! CCFC busy");
    return( AT_BUSY );
  }

/*
 *-------------------------------------------------------------------
 * check parameter <time>
 *-------------------------------------------------------------------
 */
  if( time EQ ACI_NumParmNotPresent )
  {
    time = AT_CFNRY_DEF_TIME;
  }
  else if (time < CCFC_TIME_MIN AND time >= 1)
  {
    /* 07.07 allows [1...30] for time while
       09.02 allows [5...30]. So map [1..4] to 5 */
    time = CCFC_TIME_MIN;
  }
  else if( time > CCFC_TIME_MAX OR time < CCFC_TIME_MIN )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  /* Implements Measure # 85 */
  if(!cmhSS_checkCCFC_RSN(reason, &ssCd))
  {
    return( AT_FAIL );
  }     
/*
 *-------------------------------------------------------------------
 * check parameter <class>
 *-------------------------------------------------------------------
 */
  if( !cmhSS_CheckClass( class_type, &bs1, &bst1, &bs2, &bst2, &mltyTrnFlg ))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/* SIM TOOLKIT & FDN HANDLING */

    retVal = cmhSS_CF_SAT_Handle( srcId, reason, mode, number, type, class_type, subaddr, satype, time);

    if( retVal NEQ AT_CMPL )
        return( retVal );

/*
 *-------------------------------------------------------------------
 * check parameter <class> against possible ALS-Lock
 *-------------------------------------------------------------------
 */
  if ((ALSlock EQ ALS_MOD_SPEECH     AND class_type EQ  CLASS_AuxVce) OR
      (ALSlock EQ ALS_MOD_AUX_SPEECH AND class_type NEQ CLASS_AuxVce))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_AlsLock );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * get a new service table entry
 *-------------------------------------------------------------------
 */
  sId1 = psaSS_stbNewEntry();

  if( sId1 EQ NO_ENTRY )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_SrvTabFull );
    return( AT_FAIL );
  }

  sId2 = NO_ENTRY;

  if( mltyTrnFlg )
  {
    ssShrdPrm.stb[sId1].ntryUsdFlg = TRUE;

    sId2 = psaSS_stbNewEntry();

    if( sId2 EQ NO_ENTRY )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_SrvTabFull );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * check parameter <mode>
 *-------------------------------------------------------------------
 */
  switch( mode )
  {
    /*
     *---------------------------------------------------------------
     * enable, disable and erase call forwarding service
     *---------------------------------------------------------------
     */
    case( CCFC_MOD_Disable    ):
    case( CCFC_MOD_Enable     ):
    case( CCFC_MOD_Erasure    ):

      CCD_START;

      if( mode EQ CCFC_MOD_Disable )
        psaSS_asmDeactivateSS( ssCd, bst1, bs1 );

      else if( mode EQ CCFC_MOD_Enable )
        psaSS_asmActivateSS( ssCd, bst1, bs1 );

      else if( mode EQ CCFC_MOD_Erasure )
        psaSS_asmEraseSS( ssCd, bst1, bs1 );

      ssShrdPrm.stb[sId1].ntryUsdFlg = TRUE;
      ssShrdPrm.stb[sId1].ssCode     = ssCd;
      ssShrdPrm.stb[sId1].srvOwn     = (T_OWN)srcId;
      ssShrdPrm.stb[sId1].ClassType  = class_type;  /* the complete classes defined by the user */

      /* start first transaction */
      ssShrdPrm.stb[sId1].curCmd        = AT_CMD_CCFC;

      cmhSS_flagTrn( sId1, &(cmhPrm[srcId].ssCmdPrm.mltyTrnFlg));
      psaSS_NewTrns(sId1);

      if( mltyTrnFlg )
      {
        if( mode EQ CCFC_MOD_Disable )
          psaSS_asmDeactivateSS( ssCd, bst2, bs2 );

        else if( mode EQ CCFC_MOD_Enable )
          psaSS_asmActivateSS( ssCd, bst2, bs2 );

        else if( mode EQ CCFC_MOD_Erasure )
          psaSS_asmEraseSS( ssCd, bst2, bs2 );

        ssShrdPrm.stb[sId2].ntryUsdFlg = TRUE;
        ssShrdPrm.stb[sId2].ssCode     = ssCd;
        ssShrdPrm.stb[sId2].srvOwn     = (T_OWN)srcId;
        ssShrdPrm.stb[sId2].ClassType  =class_type;  /* the complete classes defined by the user */

        /* start second transaction */
        ssShrdPrm.stb[sId2].curCmd = AT_CMD_CCFC;

        cmhSS_flagTrn( sId2, &(ssShrdPrm.mltyTrnFlg)); /* only for the second one */
        cmhSS_flagTrn( sId2, &(cmhPrm[srcId].ssCmdPrm.mltyTrnFlg));

        psaSS_NewTrns(sId2);
      }

      CCD_END;
      break;

    /*
     *---------------------------------------------------------------
     * register call forwarding service
     *---------------------------------------------------------------
     */
    case( CCFC_MOD_Register   ):

      if(number EQ NULL)
      {
        TRACE_EVENT("ERROR: cannot register CCFC without a number");
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }
      MALLOC(ccfc_cldPty, sizeof(T_CLPTY_PRM));
      cmhCC_init_cldPty( ccfc_cldPty );

      cmh_bldCalPrms( number, ccfc_cldPty );

      /* check for TOA default */
      if(type EQ NULL)
      {
        ton = ccfc_cldPty->ton;
        npi = ccfc_cldPty->npi;
      }
      else if(type->ton EQ TON_NotPresent OR
              type->npi EQ NPI_NotPresent)
      {
        ton = ccfc_cldPty->ton;
        npi = ccfc_cldPty->npi;
      }
      else
      {
        ton = type->ton;
        npi = type->npi;
      }

      /* check for TOS default */
      if(satype EQ NULL)
      {
        tos = TOS_X213;
        oe  = OEI_EVEN;
      }
      else if(satype->tos EQ TOS_NotPresent OR
              satype->oe  EQ OE_NotPresent)
      {
        tos = TOS_X213;
        oe  = OEI_EVEN;
      }
      else
      {
        tos = satype -> tos;
        oe  = satype -> oe;
      }

      if(subaddr EQ NULL           AND
         ccfc_cldPty->sub[0] NEQ 0) /* if not set by user AND number contained a subaddress */
      {
        subaddr = (CHAR *)ccfc_cldPty->sub;
      }

      CCD_START;

      psaSS_asmRegisterSS( ssCd, bst1, bs1,
                           ton, npi, (UBYTE*)ccfc_cldPty->num, tos, oe, (UBYTE *)subaddr, (UBYTE)time );

      ssShrdPrm.stb[sId1].ntryUsdFlg = TRUE;
      ssShrdPrm.stb[sId1].ssCode     = ssCd;
      ssShrdPrm.stb[sId1].srvOwn     = (T_OWN)srcId;

      /* start first transaction */
      ssShrdPrm.stb[sId1].curCmd        = AT_CMD_CCFC;
      cmhSS_flagTrn( sId1, &(cmhPrm[srcId].ssCmdPrm.mltyTrnFlg));
      psaSS_NewTrns(sId1);

      if( mltyTrnFlg )
      {
        psaSS_asmRegisterSS( ssCd, bst2, bs2,
                             ton, npi, (UBYTE*)ccfc_cldPty->num, tos, oe, (UBYTE *)subaddr, (UBYTE)time );

        ssShrdPrm.stb[sId2].ntryUsdFlg = TRUE;
        ssShrdPrm.stb[sId2].ssCode     = ssCd;
        ssShrdPrm.stb[sId2].srvOwn     = (T_OWN)srcId;

        /* start second transaction */
        ssShrdPrm.stb[sId2].curCmd = AT_CMD_CCFC;
        cmhSS_flagTrn( sId2, &(ssShrdPrm.mltyTrnFlg)); /* only for the second one */
        cmhSS_flagTrn( sId2, &(cmhPrm[srcId].ssCmdPrm.mltyTrnFlg));
        psaSS_NewTrns(sId2);
      }
      MFREE(ccfc_cldPty);
      CCD_END;
      break;

    /*
     *---------------------------------------------------------------
     * unexpected mode
     *---------------------------------------------------------------
     */
    case( CCFC_MOD_NotPresent ):
    default:

      ssShrdPrm.stb[sId1].ntryUsdFlg = FALSE;
      if( sId2 NEQ NO_ENTRY ) ssShrdPrm.stb[sId2].ntryUsdFlg = FALSE;

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  {
  T_ACI_CLOG cmdLog;        /* holds logging info */

  cmdLog.atCmd                = AT_CMD_CCFC;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = sId1+1;
  cmdLog.cmdPrm.sCCFC.srcId   = srcId;
  cmdLog.cmdPrm.sCCFC.reason  = reason;
  cmdLog.cmdPrm.sCCFC.mode    = mode;
  cmdLog.cmdPrm.sCCFC.number  = number;
  cmdLog.cmdPrm.sCCFC.type    = type;
  cmdLog.cmdPrm.sCCFC.class_type = class_type;
  cmdLog.cmdPrm.sCCFC.subaddr = subaddr;
  cmdLog.cmdPrm.sCCFC.satype  = satype;
  cmdLog.cmdPrm.sCCFC.time    = time;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : sAT_PlusCLCK             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CLCK AT command
            which is responsible to set the parameters for call
            barring supplementary services.

            <fac>    : CB facility.
            <mode>   : CB mode.
            <passwd> : password for CB facility.
            <class_type>: class of calls.
*/
LOCAL BOOL check_sat_cb_pwd_required(UBYTE ssCd, CHAR  *passwd)
{
#ifdef SIM_TOOLKIT
  SHORT ss_id = NO_ENTRY;
  T_OWN owner;

  /* Special case: there was an SS (by STK) transaction running. 
     It returned get password, handle the password for this transaction */

  TRACE_FUNCTION ("check_sat_ss_pwd_required ()");

  /* satShrdPrm.sId_pwd_requested will be reset by any mnss_end_ind
  to be sure it has been reset.
  This means: user has to give password right after error has been
  shown. */
  ss_id = satShrdPrm.sId_pwd_requested;
  if( ss_id EQ NO_ENTRY)
  {
    return(FALSE);
  }

  if( ssShrdPrm.stb[ss_id].srvOwn NEQ OWN_SRC_SAT )
  {
    satShrdPrm.sId_pwd_requested = NO_ENTRY;
    return(FALSE);
  }
  TRACE_EVENT("Password required during a SAT session");
  
  if( ssShrdPrm.stb[ss_id].curCmd NEQ ((T_ACI_AT_CMD)KSD_CMD_CB) )
  {
    TRACE_EVENT_P1("Wrong command context: %d", ssShrdPrm.stb[ss_id].curCmd);
    satShrdPrm.sId_pwd_requested = NO_ENTRY;
    return(FALSE);
  }
  owner = ssShrdPrm.stb[ss_id].srvOwn;

  if(ssShrdPrm.stb[ss_id].ssCode EQ ssCd)
  { 
    /* store for later use (as usual)*/
    strncpy
    (
      (char *) cmhPrm[owner].ssCmdPrm.CXXXpwd,
      (char *) passwd,
      MAX_PWD_NUM
    );
    cmhPrm[owner].ssCmdPrm.CXXXpwd[MAX_PWD_NUM] = '\0';
    psaSS_asmVerifyPWD( cmhPrm[owner].ssCmdPrm.CXXXpwd);
    psaSS_CntTrns(ss_id);
    satShrdPrm.sId_pwd_requested = NO_ENTRY;
    return(TRUE); /* as pwd only needed for SAT session*/
  }
  else
  {
    TRACE_EVENT_P1("Wrong Service type: %d", ssCd);
  }
  satShrdPrm.sId_pwd_requested = NO_ENTRY;
#endif /* SIM_TOOLKIT */
  return(FALSE);
}

/*
+------------------------------------------------------------------------------
|  Function    : sAT_PlusCLCK
+------------------------------------------------------------------------------
|  Description : This is the functional counterpart to the +CLCK AT command
|            which is responsible to set the parameters for call
|            barring supplementary services. also fo locking and unlocking ME for categories 
|            Networl lock, Network Subset lock, Service Provider Lock, Corporate Lock, SIM Lock
|
|  Parameters  : <fac>    : CB facility.
|                      <mode>   : CB mode.
|                      <passwd> : password for CB facility.
|                      <class_type>: class of calls.
|
|  Return      :  AT_FAIL    -      execution of command failed 
|         AT_CMPL   -      execution of command completed 
|         AT_EXCT   -      execution of command is in progress 
|         AT_BUSY   -      execution of command is rejected due
|
+------------------------------------------------------------------------------
*/

GLOBAL T_ACI_RETURN sAT_PlusCLCK  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_FAC fac,
                                    T_ACI_CLCK_MOD mode,
                                    CHAR  *        passwd,
                                    T_ACI_CLASS    class_type)
{
  SHORT sId1, sId2;        /* holds service id  */
  UBYTE ssCd;                  /* holds ss code     */
  UBYTE bst1, bst2;         /* holds basic service type */
  UBYTE bs1,  bs2;           /* holds basic service */
  BOOL  mltyTrnFlg = FALSE; /* flags for multi transaction */
  #ifdef SIM_PERS
  T_SIMLOCK_TYPE slocktype;
  T_SIMLOCK_STATUS rlockstatus;
  #endif
  SHORT psasim_ret;
  BOOL  done = FALSE;
#ifdef SIM_PERS
  T_ACI_CME_ERR err_code = CME_ERR_NotPresent; /* code holding the correct error code calculated */
  UBYTE cmdBuf;                 /* buffers current command */
#endif
#ifdef SIM_PERS
  T_OPER_RET_STATUS fcResetStat;  /*holds Failure Counter Reset Return Status Introduced on 11/03/2005*/
  T_OPER_RET_STATUS masterunlockStat;
  T_SIMLOCK_STATUS retSlStatus; /* holds return code */
#endif
#ifdef FF_PHONE_LOCK
   T_OPER_RET_STATUS phone_lock_oper_ret_stat;
#endif
  T_ACI_RETURN   retVal;

  TRACE_FUNCTION ("sAT_PlusCLCK ()");
   
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter <fac>
 *-------------------------------------------------------------------
 */
  /* Implements Measure # 166 */
#ifdef SIM_PERS
  if((fac EQ FAC_Fc) OR ( fac EQ FAC_Fcm) OR ( fac EQ FAC_Mu)  OR (fac EQ FAC_Mum))
  {
    ssCd = NOT_PRESENT_8BIT;
  }
  else
#endif
  if( !cmhSS_getSSCd(fac, &ssCd) )
  {
    return( AT_FAIL );
  }
/*
 *-------------------------------------------------------------------
 * if action is related to SS
 *-------------------------------------------------------------------
 */
  if( ssCd NEQ NOT_PRESENT_8BIT )
  {
  /*
   *-------------------------------------------------------------------
   * check parameter <passwd>
   *-------------------------------------------------------------------
   */
    if( passwd AND strlen( passwd ) > MAX_PWD_NUM )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }

  /*
   *-------------------------------------------------------------------
   * check parameter <class>
   *-------------------------------------------------------------------
   */
    if( !cmhSS_CheckCbClass( class_type, &bs1, &bst1, &bs2, &bst2, &mltyTrnFlg ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return( AT_FAIL );
    }

    if( cmhPrm[srcId].ssCmdPrm.mltyTrnFlg NEQ 0 )
      return( AT_BUSY );

  /*
   *-------------------------------------------------------------------
   * check combination of <mode> and <fac>
   *-------------------------------------------------------------------
   */
    if((fac EQ FAC_Ab OR fac EQ FAC_Ag OR fac EQ FAC_Ac)
       AND mode NEQ CLCK_MOD_Unlock )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return( AT_FAIL );
    }

    /*** Shangai's labs tessession fix: adapted to this version from 1.3.3 (TCS 2.0) ****/
      /* Special case: there was an SS (by STK) transaction running. 
         It returned get password, handle the password for this transaction */
    done = check_sat_cb_pwd_required(ssCd, passwd);
    if(done)
    {
      return(AT_CMPL); /* as pwd only needed for SAT session*/
    }
    /* End fix */


/* SIM TOOLKIT & FDN HANDLING */
    retVal = cmhSS_Call_Barr_SAT_Handle( srcId, mode, fac, passwd, class_type);

    if( retVal NEQ AT_CMPL )
        return( retVal );
  
  /*
   *-------------------------------------------------------------------
   * get a new service table entry
   *-------------------------------------------------------------------
   */
    sId1 = psaSS_stbNewEntry();

    if( sId1 EQ NO_ENTRY )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_SrvTabFull );
      return( AT_FAIL );
    }

    sId2 = NO_ENTRY;

    if( mltyTrnFlg )
    {
      ssShrdPrm.stb[sId1].ntryUsdFlg = TRUE;
      sId2 = psaSS_stbNewEntry();

      if( sId2 EQ NO_ENTRY )
      {
        ssShrdPrm.stb[sId1].ntryUsdFlg = FALSE;
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_SrvTabFull );
        return( AT_FAIL );
      }
    }

  /*
   *-------------------------------------------------------------------
   * check parameter <mode>
   *-------------------------------------------------------------------
   */
    switch( mode  )
    {
      /*
       *---------------------------------------------------------------
       * lock and unlock call barring service
       *---------------------------------------------------------------
       */
      case( CLCK_MOD_Unlock ):
      case( CLCK_MOD_Lock   ):

        CCD_START;

        if( mode EQ CLCK_MOD_Unlock )
          psaSS_asmDeactivateSS( ssCd, bst1, bs1 );

        else if( mode EQ CLCK_MOD_Lock )
          psaSS_asmActivateSS( ssCd, bst1, bs1 );

        ssShrdPrm.stb[sId1].ntryUsdFlg = TRUE;
        ssShrdPrm.stb[sId1].ssCode     = ssCd;
        ssShrdPrm.stb[sId1].srvOwn     = (T_OWN)srcId;

        /* start first transaction */
        ssShrdPrm.stb[sId1].curCmd        = AT_CMD_CLCK;
        cmhSS_flagTrn( sId1, &(cmhPrm[srcId].ssCmdPrm.mltyTrnFlg));
        psaSS_NewTrns(sId1);

        if( mltyTrnFlg )
        {
          if( mode EQ CLCK_MOD_Unlock )
            psaSS_asmDeactivateSS( ssCd, bst2, bs2 );

          else if( mode EQ CLCK_MOD_Lock )
            psaSS_asmActivateSS( ssCd, bst2, bs2 );

          ssShrdPrm.stb[sId2].ntryUsdFlg = TRUE;
          ssShrdPrm.stb[sId2].ssCode      = ssCd;
          ssShrdPrm.stb[sId2].srvOwn     = (T_OWN)srcId;

          /* start second transaction */
          ssShrdPrm.stb[sId2].curCmd = AT_CMD_CLCK;
          cmhSS_flagTrn( sId2, &(ssShrdPrm.mltyTrnFlg)); /* only for the second one */
          cmhSS_flagTrn( sId2, &(cmhPrm[srcId].ssCmdPrm.mltyTrnFlg));
          psaSS_NewTrns(sId2);
        }
        CCD_END;
        break;

      /*
       *---------------------------------------------------------------
       * unexpected mode
       *---------------------------------------------------------------
       */
      case( CLCK_MOD_NotPresent ):
      default:

        ssShrdPrm.stb[sId1].ntryUsdFlg = FALSE;
        if( sId2 NEQ NO_ENTRY ) ssShrdPrm.stb[sId2].ntryUsdFlg = FALSE;

        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
    }

  /*
   *-------------------------------------------------------------------
   * store password for later use
   *-------------------------------------------------------------------
   */
    if( passwd )
    {
      strncpy
      (
        (char *) cmhPrm[srcId].ssCmdPrm.CXXXpwd,
        (char *) passwd,
        MAX_PWD_NUM
      );
      cmhPrm[srcId].ssCmdPrm.CXXXpwd[MAX_PWD_NUM] = '\0';
    }
  }

  /*
   *-------------------------------------------------------------------
   * if action is related to SIM
   *-------------------------------------------------------------------
   */
  else
  {
      if( simEntStat.curCmd NEQ AT_CMD_NONE )
      return( AT_BUSY );

    switch (fac)
    {
      /*
       *---------------------------------------------------------------
       * access PIN 1
       *---------------------------------------------------------------
       */
      case FAC_Sc:
        switch( mode )
        {
          case( CLCK_MOD_Unlock ):
          case( CLCK_MOD_Lock   ):
            if (simShrdPrm.PINStat EQ PS_PUK1)
            {
              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPukReq);
              return( AT_FAIL );
            }
            if (!psaSIM_ChkSIMSrvSup(SRV_CHV1_Disable))
            {
              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
              return( AT_FAIL );
            }
            if( !passwd OR (strlen( passwd ) > PIN_LEN) )
            {
              ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
              return( AT_FAIL );
            }
            if( strlen( passwd ) < MIN_PIN_LEN OR   
                strlen( passwd ) > PIN_LEN        )
            {
              ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
              return( AT_FAIL );
            }
            cmhSIM_FillInPIN ( passwd, simShrdPrm.setPrm[srcId].curPIN, PIN_LEN );
            simShrdPrm.setPrm[srcId].PINType = PHASE_2_PIN_1;
            simEntStat.curCmd     = AT_CMD_CLCK;
            simShrdPrm.owner = (T_OWN)srcId;
            simEntStat.entOwn =  srcId;

            if( mode EQ CLCK_MOD_Lock )
            {
              psasim_ret = psaSIM_EnablePIN( );
            }
            else /* if( mode EQ CLCK_MOD_Unlock ) */
            {
              psasim_ret = psaSIM_DisablePIN( );
            }

            if(psasim_ret < 0)
            {
              TRACE_EVENT( "FATAL RETURN psaSIM in +CLCK" );
              ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
              return( AT_FAIL );
            }
            break;

          default:

            ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
            return( AT_FAIL );
        }
        break;

      /*
       *---------------------------------------------------------------
       * access fixed dialling feature
       *---------------------------------------------------------------
       */
      case FAC_Fd:
    if (!psaSIM_ChkSIMSrvSup(SRV_FDN))
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
          return( AT_FAIL );
        }
        /* 
          Check the SIM PIN States before sending any request to SIM 
        */
        if ( simShrdPrm.PINStat EQ PS_PUK2 )
        {
          TRACE_EVENT("sAT_PlusCLCK_ME(): SIM PIN-2 Bolcked PUK-2 Required");
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPuk2Req );
          return( AT_FAIL );
        }
       
        /* 
          According to the standards 27.007 Sec 7.4 for facility "FDN": 
          if PIN2 authentication has not been done during the current session,
          PIN2 is required as <passwd>; Below condition will do the
          same to check to see whether <passwd> is required / not. 
        */

        if ( simShrdPrm.pn2Stat EQ PS_PIN2  OR 
             simShrdPrm.pn2Stat EQ NO_VLD_PS )
        { 
          if ( simShrdPrm.PINStat EQ PS_RDY OR
               simShrdPrm.PINStat EQ PS_PIN2 )
          {
            simShrdPrm.PINStat = PS_PIN2;          
          }
          else
          {
            TRACE_EVENT("sAT_PlusCLCK_ME(): SIM PIN State is NOT Valid");
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimFail );
            return( AT_FAIL );
          }
        
          if( !passwd OR ( strlen( passwd ) > PIN_LEN OR
               strlen( passwd ) < MIN_PIN_LEN ) )
          {
            TRACE_EVENT("sAT_PlusCLCK_ME(): SIM PIN-2 Password is Omitted");
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPin2Req );
            return( AT_FAIL );
          }
        }

         /* 
         * test class_type whether it is supported and store it temporarily for further use 
         */      
        if( class_type EQ CLASS_NotPresent OR    /* default case */
            class_type EQ CLASS_VceDatFax     )  /* case 7       */
        {
          simShrdPrm.classFDN = CLASS_VceDatFax;
        }
        else if ( class_type EQ CLASS_VceDatFaxSms  ) /* case 15 */
        {
          simShrdPrm.classFDN = CLASS_VceDatFaxSms;  /* set global class type */
        }
        else  /* parameter not supported */
        {
          TRACE_EVENT( "class type not supported, +CLCK rejected" );
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotSupp );
          return( AT_FAIL );        
        }
        simShrdPrm.setPrm[srcId].actProc = (mode EQ CLCK_MOD_Lock)?
                                             SIM_FDN_ENABLE:SIM_FDN_DISABLE;

        simEntStat.curCmd = AT_CMD_CLCK;
        simShrdPrm.owner = (T_OWN)srcId;
        simEntStat.entOwn =  srcId;

        if( psaSIM_ActivateSIM() < 0 )   /* activate SIM card */
        {
          TRACE_EVENT( "FATAL RETURN psaSIM in +CLCK" );
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
          return( AT_FAIL );
        }

        /* store PIN 2 for later use */
        /* if( passwd )  ... already checked in the beginning! */
        cmhSIM_FillInPIN ( passwd, simShrdPrm.setPrm[srcId].curPIN, PIN_LEN );

        break;

      /*
       *---------------------------------------------------------------
       * lock ALS setting with PIN2
       *---------------------------------------------------------------
       */
      case FAC_Al:

        if( !passwd OR ( strlen( passwd ) > PIN_LEN     OR
                         strlen( passwd ) < MIN_PIN_LEN ) )
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
          return( AT_FAIL );
        }

        cmhSIM_FillInPIN ( passwd, simShrdPrm.setPrm[srcId].curPIN, PIN_LEN );

        simShrdPrm.setPrm[srcId].actProc = (mode EQ CLCK_MOD_Lock)?
                                             SIM_ALS_LOCK:SIM_ALS_UNLOCK;

        simShrdPrm.setPrm[srcId].PINType = PHASE_2_PIN_2;

        simEntStat.curCmd = AT_CMD_CLCK;
        simShrdPrm.owner = (T_OWN)srcId;
        simEntStat.entOwn =  srcId;

        if( psaSIM_VerifyPIN() < 0 )   /* check PIN2 card */
        {
          TRACE_EVENT( "FATAL RETURN psaSIM in +CLCK" );
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
          return( AT_FAIL );
        }

        break;

      /*
       *---------------------------------------------------------------
       * ME Personalisation
       *---------------------------------------------------------------
       */
  #ifdef SIM_PERS
      case FAC_Pn:
      case FAC_Pu:
      case FAC_Pp:
      case FAC_Pc:
      case FAC_Ps:
      case FAC_Pf:
      case FAC_Bl:
                
        switch (fac)
        {
          case FAC_Pn: slocktype = SIMLOCK_NETWORK;          break;
          case FAC_Pu: slocktype = SIMLOCK_NETWORK_SUBSET;   break;
          case FAC_Pp: slocktype = SIMLOCK_SERVICE_PROVIDER; break;
          case FAC_Pc: slocktype = SIMLOCK_CORPORATE;        break;
          case FAC_Ps: slocktype = SIMLOCK_SIM;              break;
          case FAC_Pf: slocktype = SIMLOCK_FIRST_SIM;        break;
	  case FAC_Bl: slocktype = SIMLOCK_BLOCKED_NETWORK;        break;
          default: slocktype = SIMLOCK_NETWORK;
        }

       /* 
        * Check whether sim is inserted or not before trying to lock/Unlock 
        */
       
       if(simShrdPrm.SIMStat EQ SS_OK) 
        {
          rlockstatus = (mode EQ CLCK_MOD_Lock)?
          aci_slock_lock(slocktype, passwd):
          aci_slock_unlock(slocktype, passwd);
       }
      else 
       {
          /*
           * Sim is not present: unlocking is allowed if No_SIM_Unlock flag is OFF
          */
         if(!aci_slock_set_CFG())
         {
            ACI_ERR_DESC( ACI_ERR_CLASS_Ext,EXT_ERR_NoMEPD); 
            return( AT_FAIL );
         }

         if( (mode EQ CLCK_MOD_Unlock) AND !(cfg_data->Flags & SEC_DRV_HDR_FLAG_No_SIM_Unlock))
         {
            MFREE(cfg_data);
            rlockstatus = aci_slock_unlock(slocktype, passwd);        
         }
        else
        {
           MFREE(cfg_data);
           ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimNotIns);    
           return( AT_FAIL );
        }
      }
      cmdBuf = AT_CMD_CLCK ; 
      if ( ( (mode EQ CLCK_MOD_Lock) AND (rlockstatus EQ SIMLOCK_ENABLED) ) OR
              ( (mode EQ CLCK_MOD_Unlock) AND (rlockstatus EQ SIMLOCK_DISABLED) ) )
        {
            return (AT_CMPL);
        }
        else
        {
          if (rlockstatus EQ SIMLOCK_BLOCKED) /* Special error description on blocked personalisation */
           {
             if(!aci_slock_set_CFG())
                {
                  ACI_ERR_DESC( ACI_ERR_CLASS_Ext,EXT_ERR_NoMEPD);    
                  return( AT_FAIL );
                }
               aci_set_cme_error(slocktype);
               MFREE(cfg_data);
           }
          else if(rlockstatus EQ SIMLOCK_WAIT)
            {
               simEntStat.curCmd = AT_CMD_CLCK;
               simShrdPrm.owner = (T_OWN)srcId;
               simEntStat.entOwn = srcId;
              AciSLockShrd.check_lock = SIMLOCK_CHECK_LOCK;
              AciSLockShrd.lock_type = slocktype;
              AciSLockShrd.lock_passwd = passwd;
              
              return( AT_EXCT);
            }
          else if(rlockstatus EQ SIMLOCK_BUSY)
          {
            ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Busy );
          }
          else 
          {
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_WrongPasswd );
          }
            return( AT_FAIL );
        }

/*
       *---------------------------------------------------------------
       * FC RESET Handling
       Added on 11/03/2005
       *---------------------------------------------------------------
*/
     case FAC_Fc:
      {
      fcResetStat=aci_slock_reset_fc(passwd);
      simEntStat.curCmd     = AT_CMD_CLCK;     
      if (fcResetStat EQ OPER_SUCCESS)
      {
          if( !aci_slock_set_CFG())
              {
                ACI_ERR_DESC( ACI_ERR_CLASS_Ext,EXT_ERR_NoMEPD);    
                return( AT_FAIL );
              }
             aci_slock_init();
             AciSLockShrd.check_lock = SIMLOCK_CHECK_RESET_FC;           
             retSlStatus = aci_slock_checkpersonalisation(AciSLockShrd.current_lock);
             switch(retSlStatus)
             {
               case  SIMLOCK_ENABLED  :
			     return( AT_CMPL);
             case  SIMLOCK_WAIT :
               return (AT_EXCT);
             default : 
               return( AT_FAIL);
           }
         }
         else 
         {
           cmdBuf = simEntStat.curCmd;
           simEntStat.curCmd = AT_CMD_NONE;

           if (fcResetStat EQ OPER_WRONG_PASSWORD)
           {
              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_WrongPasswd );
              aci_slock_send_RAT(cmdBuf,CME_ERR_WrongPasswd);		
           }
           else if (fcResetStat EQ OPER_BUSY)
           {
              ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Busy );
              aci_slock_send_RAT(cmdBuf,err_code);
           }
           else 
           {
              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
              aci_slock_send_RAT(cmdBuf,CME_ERR_Unknown);
           }
          return( AT_FAIL );
       }
     } 
    case FAC_Fcm:
     {
           fcResetStat=aci_slock_reset_fc(passwd);
           return(cmhSS_check_oper_result(fcResetStat));
     } 
     case( FAC_Mu ): 
     {
       masterunlockStat = aci_slock_master_unlock(passwd); 
       simEntStat.curCmd     = AT_CMD_CLCK; 
       if( masterunlockStat EQ OPER_SUCCESS )  
       {
          if(!aci_slock_set_CFG())
          {
                 ACI_ERR_DESC( ACI_ERR_CLASS_Ext,EXT_ERR_NoMEPD);     
                 return( AT_FAIL );
          }
              aci_slock_init();
              AciSLockShrd.check_lock = SIMLOCK_CHECK_MASTER_UNLOCK;           
              retSlStatus = aci_slock_checkpersonalisation(AciSLockShrd.current_lock);
              switch(retSlStatus)
              {
                case  SIMLOCK_ENABLED  :
                  return( AT_CMPL);
                case  SIMLOCK_WAIT :
                 return (AT_EXCT);
                default : 
                  return( AT_FAIL);
              } 
       }
      else
      {
          cmdBuf = simEntStat.curCmd;
          simEntStat.curCmd = AT_CMD_NONE;
          if( masterunlockStat EQ OPER_WRONG_PASSWORD )
          {
              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_WrongPasswd );
              aci_slock_send_RAT(cmdBuf,CME_ERR_WrongPasswd);	  
          }
         else 
          {
              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
              aci_slock_send_RAT(cmdBuf,CME_ERR_Unknown);
          }
        return( AT_FAIL );
      }
     }
    case( FAC_Mum ): 
    {
       masterunlockStat = aci_slock_master_unlock(passwd); 
       return(cmhSS_check_oper_result(masterunlockStat));
    }
#endif  /* SIM_PERS */
#ifdef FF_PHONE_LOCK
    case ( FAC_Pl ):
      {
          phone_lock_oper_ret_stat = aci_ext_set_phone_lock_satus(PHONE_LOCK,(T_SEC_DRV_EXT_PHONE_LOCK_STATUS)mode,passwd); 	 
         return(cmhSS_check_oper_result(phone_lock_oper_ret_stat));
     }

   case ( FAC_Apl ):
      { 
          phone_lock_oper_ret_stat = aci_ext_set_phone_lock_satus(AUTO_LOCK,(T_SEC_DRV_EXT_PHONE_LOCK_STATUS)mode,passwd); 
         return(cmhSS_check_oper_result(phone_lock_oper_ret_stat));
	  	
       }
 #endif
      /*
       *---------------------------------------------------------------
       * Error handling
       *---------------------------------------------------------------
       */
      default:
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
        return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  {
  T_ACI_CLOG cmdLog;        /* holds logging info */

  cmdLog.atCmd                = AT_CMD_CLCK;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = (ssCd EQ NOT_PRESENT_8BIT)?
                                   ACI_NumParmNotPresent:/*lint -e(644)*/sId1+1;
  cmdLog.cmdPrm.sCLCK.srcId   = srcId;
  cmdLog.cmdPrm.sCLCK.fac     = fac;
  cmdLog.cmdPrm.sCLCK.mode    = mode;
  cmdLog.cmdPrm.sCLCK.passwd  = passwd;
  cmdLog.cmdPrm.sCLCK.class_type = class_type;
    rAT_PercentCLOG( &cmdLog );
  }
#endif
  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : sAT_PlusCPWD             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPWD AT command
            which is responsible to change the password for call
            barring supplementary services.

            <fac>    : CB facility.
            <oldpwd> : current password.
            <newpwd> : new password.
*/

GLOBAL T_ACI_RETURN sAT_PlusCPWD  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_FAC fac,
                                    CHAR  *        oldpwd,
                                    CHAR  *        newpwd)
{
  T_ACI_RETURN retCd = AT_FAIL;   /* holds return code */
  UBYTE ssCd;                     /* holds ss code */
  SHORT sId;                      /* holds service id */
  T_OPER_RET_STATUS phone_lock_oper_ret_stat; 
  
  TRACE_FUNCTION ("sAT_PlusCPWD");

/*
 *-------------------------------------------------------------------
 * check password parameter
 *-------------------------------------------------------------------
 */
  if( !newpwd OR !oldpwd )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter <fac>
 *-------------------------------------------------------------------
 */
  switch( fac )
  {
    /* All Baring Services share the same password so only SS_CD_ALL_CBSS is valid,
       everything else will be rejected by the network! */
    case( FAC_Ao ):
    case( FAC_Oi ):
    case( FAC_Ox ):
    case( FAC_Ai ):
    case( FAC_Ir ):
    case( FAC_Ab ):
    case( FAC_Ag ):
    case( FAC_Ac ): ssCd = SS_CD_ALL_CBSS;   break;

    case( FAC_Sc ):
    case( FAC_P2 ):
#ifdef SIM_PERS
    case( FAC_Pn ):
    case( FAC_Pu ):
    case( FAC_Pc ):
    case( FAC_Pp ):
    case( FAC_Pf ):
    case( FAC_Ps ):
    case( FAC_Bl ):
#endif
#ifdef FF_PHONE_LOCK
    case(FAC_Pl):
    case(FAC_Apl):
#endif
		
                   ssCd = NOT_PRESENT_8BIT; break;

    case( FAC_NotPresent  ):
    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * if password relates to SS
 *-------------------------------------------------------------------
 */
  if( ssCd NEQ NOT_PRESENT_8BIT )
  {
  /*
   *-------------------------------------------------------------------
   * check parameter <oldpwd>
   *-------------------------------------------------------------------
   */
    if( strlen( newpwd ) > MAX_PWD_NUM OR
        strlen( oldpwd ) > MAX_PWD_NUM    )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }

  /*
   *-----------------------------------------------------------------
   * get a new service table entry to interrogate SS
   *-----------------------------------------------------------------
   */
    sId = psaSS_stbNewEntry();

    if( sId EQ NO_ENTRY )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_SrvTabFull );
      return( AT_FAIL );
    }

  /*
   *-----------------------------------------------------------------
   * set up  facility information element
   *-----------------------------------------------------------------
   */
    CCD_START;

    psaSS_asmRegisterPWD( ssCd );
  /*
   *---------------------------------------------------------------
   * declare service table entry as used and the owner of the service
   *---------------------------------------------------------------
   */
    ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
    ssShrdPrm.stb[sId].ssCode     = ssCd;
    ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;

  /*
   *-----------------------------------------------------------------
   * start a new transaction
   *-----------------------------------------------------------------
   */
    ssShrdPrm.stb[sId].curCmd = AT_CMD_CPWD;
    psaSS_NewTrns(sId);

    CCD_END;

  /*
   *-----------------------------------------------------------------
   * store passwords for later use
   *-----------------------------------------------------------------
   */
    strncpy
    (
      (char *) cmhPrm[srcId].ssCmdPrm.CXXXpwd,
      (char *) oldpwd,
      MAX_PWD_NUM
    );
    cmhPrm[srcId].ssCmdPrm.CXXXpwd[MAX_PWD_NUM] = '\0';

    strncpy
    (
      (char *) cmhPrm[srcId].ssCmdPrm.CXXXnewPwd,
      (char *) newpwd,
      MAX_PWD_NUM
    );
    cmhPrm[srcId].ssCmdPrm.CXXXnewPwd[MAX_PWD_NUM] = '\0';

    strncpy
    (
      (char *) cmhPrm[srcId].ssCmdPrm.CXXXnewPwd2,
      (char *) newpwd,
      MAX_PWD_NUM
    );
    cmhPrm[srcId].ssCmdPrm.CXXXnewPwd2[MAX_PWD_NUM] = '\0';
    retCd = AT_EXCT;
  }
  else
  {
    /*
     *-------------------------------------------------------------------
     * if password relates to SIM
     *-------------------------------------------------------------------
     */
    switch (fac)
    {
      /*
       *---------------------------------------------------------------
       * change PIN 1 / PIN 2
       *---------------------------------------------------------------
       */
      case( FAC_Sc ):
      case( FAC_P2 ):
        if( simEntStat.curCmd NEQ AT_CMD_NONE )
          return( AT_BUSY );

        if( strlen( newpwd ) > PIN_LEN     OR
            strlen( newpwd ) < MIN_PIN_LEN OR
            strlen( oldpwd ) > PIN_LEN     OR
            strlen( oldpwd ) < MIN_PIN_LEN    )
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
          return( AT_FAIL );
        }

        if( simEntStat.curCmd NEQ AT_CMD_NONE )
          return( AT_BUSY );

        cmhSIM_FillInPIN ( oldpwd, simShrdPrm.setPrm[srcId].curPIN, PIN_LEN );
        cmhSIM_FillInPIN ( newpwd, simShrdPrm.setPrm[srcId].newPIN, PIN_LEN );

        simShrdPrm.setPrm[srcId].PINType = (fac EQ FAC_Sc)?
                                   PHASE_2_PIN_1:PHASE_2_PIN_2;

        simEntStat.curCmd     = AT_CMD_CPWD;
        simShrdPrm.owner     = (T_OWN)srcId;
        simEntStat.entOwn     =  srcId;

        if( psaSIM_ChangePIN() < 0 )  /* change PIN */
        {
          TRACE_EVENT( "FATAL RETURN psaSIM in +CPWD" );
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
          return( AT_FAIL );
        }
        retCd = AT_EXCT;
        break;

#ifdef SIM_PERS
      case( FAC_Pn ):
      case( FAC_Pu ):
      case( FAC_Pc ):
      case( FAC_Pp ):
      case( FAC_Pf ):
      case( FAC_Ps ):
      case( FAC_Bl):
      {
        T_SIMLOCK_TYPE slocktype;
        T_OPER_RET_STATUS status;

        switch (fac)
        {
          case FAC_Pn: slocktype = SIMLOCK_NETWORK;          break;
          case FAC_Pu: slocktype = SIMLOCK_NETWORK_SUBSET;   break;
          case FAC_Pp: slocktype = SIMLOCK_SERVICE_PROVIDER; break;
          case FAC_Pc: slocktype = SIMLOCK_CORPORATE;        break;
          case FAC_Ps: slocktype = SIMLOCK_SIM;              break;
          case FAC_Pf: slocktype = SIMLOCK_FIRST_SIM;        break;
          case FAC_Bl: slocktype = SIMLOCK_BLOCKED_NETWORK;          break;
          default: slocktype = SIMLOCK_NETWORK;                   break;
         
        }

        status = aci_slock_change_password( slocktype, oldpwd, newpwd );
        if (status EQ OPER_SUCCESS)
          return(AT_CMPL);
        else if(status EQ OPER_FAIL )
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_WrongPasswd );
          return( AT_FAIL );
        }
        else if (status EQ OPER_BLOCKED)
        {
          if(!aci_slock_set_CFG())
           {
             	  ACI_ERR_DESC( ACI_ERR_CLASS_Ext,EXT_ERR_NoMEPD); 
                 return( AT_FAIL );
           }
          aci_set_cme_error(slocktype); 
          MFREE(cfg_data); 
          return( AT_FAIL );
        }
      }
      retCd = AT_CMPL;
      break;
#endif

#ifdef FF_PHONE_LOCK
   case FAC_Pl:
   case FAC_Apl:
    {
       phone_lock_oper_ret_stat = aci_ext_set_phone_lock_key(oldpwd,newpwd); 	 
       return(cmhSS_check_oper_result(phone_lock_oper_ret_stat)); 
    }
#endif
    }
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  {
    T_ACI_CLOG cmdLog;        /* holds logging info */

    cmdLog.atCmd                = AT_CMD_CPWD;
    cmdLog.cmdType              = CLOG_TYPE_Set;
    cmdLog.retCode              = retCd;
    cmdLog.cId                  = ACI_NumParmNotPresent;
    cmdLog.sId                  = (ssCd EQ NOT_PRESENT_8BIT)?
                                     ACI_NumParmNotPresent:/*lint -e(644)*/sId+1;
    cmdLog.cmdPrm.sCPWD.srcId   = srcId;
    cmdLog.cmdPrm.sCPWD.fac     = fac;
    cmdLog.cmdPrm.sCPWD.oldpwd  = oldpwd;
    cmdLog.cmdPrm.sCPWD.newpwd  = newpwd;
  
    rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( retCd );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : sAT_PlusCCWA             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CCWA AT command
            which is responsible to set the parameters for call
            waiting supplementary services.

            <mode>   : CW mode.
            <class_type>: class of call.
*/

GLOBAL T_ACI_RETURN sAT_PlusCCWA  ( T_ACI_CMD_SRC     srcId,
                                    T_ACI_CCWA_MOD    mode,
                                    T_ACI_CLASS       class_type)
{
  SHORT sId1, sId2;         /* holds service id */
  UBYTE bst1, bst2;         /* holds basic service type */
  UBYTE bs1,  bs2;          /* holds basic service */
  BOOL  mltyTrnFlg = FALSE; /* flags for multi transaction */

   T_ACI_RETURN   retVal;
 
  TRACE_FUNCTION ("sAT_PlusCCWA ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  if( cmhPrm[srcId].ssCmdPrm.mltyTrnFlg NEQ 0 )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check parameter <class>
 *-------------------------------------------------------------------
 */
  if( !cmhSS_CheckClass( class_type, &bs1, &bst1, &bs2, &bst2, &mltyTrnFlg ))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/* SIM TOOLKIT & FDN HANDLING */
  if (mode NEQ CCWA_MOD_NotInterrogate)  /* mode should be valid */
  {
    retVal = cmhSS_CW_SAT_Handle( srcId, (T_ACI_CCFC_MOD)mode, class_type);

    if( retVal NEQ AT_CMPL )
        return( retVal );
  }


/*
 *-------------------------------------------------------------------
 * get a new service table entry to interrogate SS
 *-------------------------------------------------------------------
 */
  sId1 = psaSS_stbNewEntry();

  if( sId1 EQ NO_ENTRY )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_SrvTabFull );
    return( AT_FAIL );
  }

  sId2 = NO_ENTRY;

  if( mltyTrnFlg )
  {
    ssShrdPrm.stb[sId1].ntryUsdFlg = TRUE;

    sId2 = psaSS_stbNewEntry();

    if( sId2 EQ NO_ENTRY )
    {
      ssShrdPrm.stb[sId1].ntryUsdFlg = FALSE;
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_SrvTabFull );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * check parameter <mode>
 *-------------------------------------------------------------------
 */
  switch( mode )
  {
    /*
     *---------------------------------------------------------------
     * lock and unlock call barring service
     *---------------------------------------------------------------
     */
    case( CCWA_MOD_Disable ):
    case( CCWA_MOD_Enable  ):

      CCD_START;

      if( mode EQ CCWA_MOD_Disable )
        psaSS_asmDeactivateSS( SS_CD_CW, bst1, bs1 );

      else if( mode EQ CCWA_MOD_Enable )
        psaSS_asmActivateSS( SS_CD_CW, bst1, bs1 );

      ssShrdPrm.stb[sId1].ntryUsdFlg = TRUE;
      ssShrdPrm.stb[sId1].ssCode     = SS_CD_CW;
      ssShrdPrm.stb[sId1].srvOwn     = (T_OWN)srcId;

      /* start first transaction */
      ssShrdPrm.stb[sId1].curCmd        = AT_CMD_CCWA;
      cmhSS_flagTrn( sId1, &(cmhPrm[srcId].ssCmdPrm.mltyTrnFlg));
      psaSS_NewTrns(sId1);

      if( mltyTrnFlg )
      {
        if( mode EQ CCWA_MOD_Disable )
          psaSS_asmDeactivateSS( SS_CD_CW, bst2, bs2 );

        else if( mode EQ CCWA_MOD_Enable )
          psaSS_asmActivateSS( SS_CD_CW, bst2, bs2 );

        ssShrdPrm.stb[sId2].ntryUsdFlg = TRUE;
        ssShrdPrm.stb[sId2].ssCode     = SS_CD_CW;
        ssShrdPrm.stb[sId2].srvOwn     = (T_OWN)srcId;

        /* start second transaction */
        ssShrdPrm.stb[sId2].curCmd = AT_CMD_CCWA;
        cmhSS_flagTrn( sId2, &(ssShrdPrm.mltyTrnFlg)); /* only for the second one */
        cmhSS_flagTrn( sId2, &(cmhPrm[srcId].ssCmdPrm.mltyTrnFlg));
        psaSS_NewTrns(sId2);
      }
      CCD_END;
      break;

    /*
     *---------------------------------------------------------------
     * unexpected mode
     *---------------------------------------------------------------
     */
    case( CCWA_MOD_NotInterrogate ):

      ssShrdPrm.stb[sId1].ntryUsdFlg = FALSE;
      return( AT_CMPL );

    default:

      ssShrdPrm.stb[sId1].ntryUsdFlg = FALSE;
      if( sId2 NEQ NO_ENTRY ) ssShrdPrm.stb[sId2].ntryUsdFlg = FALSE;

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  {
  T_ACI_CLOG cmdLog;        /* holds logging info */

  cmdLog.atCmd                = AT_CMD_CCWA;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = sId1+1;
  cmdLog.cmdPrm.sCCWA.srcId   = srcId;
  cmdLog.cmdPrm.sCCWA.mode    = mode;
  cmdLog.cmdPrm.sCCWA.class_type = class_type;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : sAT_PlusCUSD             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CUSD AT command
            which is responsible to invoke an unstructured
            supplementary services.

            <str> : USSD string.
            <dcs> : data coding scheme.
*/
LOCAL T_ACI_RETURN ussd_call_control_by_sim(T_ACI_CMD_SRC    srcId,
                                            T_ACI_USSD_DATA *str,
                                            SHORT            dcs)
{
  T_ACI_RETURN retCd = AT_CMPL;   /* holds return code */

#ifdef SIM_TOOLKIT
  T_sat_ussd    SATussd; /* to hold USSD string in case of SAT Control */
  T_CLPTY_PRM   *sat_ss_cldPty;

  if(!psaSIM_ChkSIMSrvSup( SRV_CalCntrl ))   /* call control by SAT enabled */
  {
    return(AT_CMPL);
  }

  if (psaSIM_ChkSIMSrvSup(SRV_USSDsupportInCC))
  {
    SATussd.dcs        = (UBYTE)dcs;
    SATussd.c_ussd_str = str->len;
    SATussd.ussd_str   = str->data;

    retCd = cmhSAT_USSDCntrlBySIM( &SATussd, (UBYTE)srcId );
  }
  else       /* USSD structure not known by SAT */
  {
    MALLOC(sat_ss_cldPty, sizeof(T_CLPTY_PRM));
    cmhCC_init_cldPty( sat_ss_cldPty );

    if( !utl_cvtGsmIra ( str -> data,
                         str->len,
                         (UBYTE *)sat_ss_cldPty->num,
                         MAX_USSD_DATA,
                         CSCS_DIR_IraToGsm ) )
    {
      TRACE_EVENT("utl_cvtGsmIra( ) returned FALSE");
      MFREE(sat_ss_cldPty);
      return( AT_FAIL );
    }
    retCd = cmhSAT_SSCntrlBySIM( sat_ss_cldPty, (UBYTE)srcId );
    MFREE(sat_ss_cldPty);
  }
  return( retCd );
#else
  return( AT_CMPL);
#endif /* of SIM_TOOLKIT */
}

GLOBAL T_ACI_RETURN sAT_PlusCUSD  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_USSD_DATA *str,
                                    SHORT            dcs)
{
  SHORT sId;            /* holds service id */
  T_ACI_RETURN retCd = AT_CMPL;   /* holds return code */
  UBYTE ussdLen;
  UBYTE *ussdString;
  UBYTE src_len;

  TRACE_FUNCTION ("sAT_PlusCUSD ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("Wrong source: %d", srcId);
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter str
 *-------------------------------------------------------------------
 */
  if( !str )
  {
    TRACE_EVENT("Empty string");
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter dcs
 *-------------------------------------------------------------------
 */
  if( dcs EQ ACI_NumParmNotPresent )

    dcs = 0;

/*
 *-------------------------------------------------------------------
 * check if there is a USSD request pending
 *-------------------------------------------------------------------
 */
  sId = psaSS_stbFindUssdReq();

  if( sId EQ NO_ENTRY )
  {
    /* check if there is another service in progress */
    if( psaSS_stbFindActSrv( sId ) NEQ NO_ENTRY )
    {
      TRACE_EVENT("Parallel USSD Transaction not allowed");
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_ParallelUSSD );
      return( AT_FAIL );
    }

    /* get new service table entry */
    sId = psaSS_stbNewEntry();
    if( sId EQ NO_ENTRY )
    {
      TRACE_EVENT("Service table is full");
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_SrvTabFull );
      return( AT_FAIL );
    }

    /* check ussd call control by sim */
    retCd = ussd_call_control_by_sim(srcId, str, dcs);
    if(retCd NEQ AT_CMPL)
    {
      TRACE_EVENT_P1("ussd_call_control_by_sim() returned with %d", retCd);

      return( retCd );
    }

    CCD_START;

    MALLOC(ussdString, MAX_USSD_STRING);

    /* Implements Measure 25 */
    /* This function is more correct than utl_getAlphabetCb as it takes care 
       of reserved codings */
    if( cmh_getAlphabetCb( (UBYTE)dcs ) EQ 0 ) /* 7bit alphabet */
    {
      src_len = (UBYTE)MINIMUM( MAX_USSD_STRING, str->len);
      ussdLen = utl_cvt8To7( str->data,
                             src_len,
                             ussdString, 0 );
      /* According to spec 23.038 section 6.1.2.3 for USSD packing, for bytes end with
       * (8*n)-1 i.e where n is 1,2,3....i.e byte 7, 15, 23 ... to be padded 
       * with carriage return <CR>(0xD) 
       */
      if ((src_len+1)%8 EQ 0)
      {
        ussdString[ussdLen-1] |= (0xD << 1);
      }
    }
    else
    {
      ussdLen = str->len;
      memcpy(ussdString, str->data, MAX_USSD_STRING);
    }

    psaSS_asmProcUSSDReq( (UBYTE)dcs, ussdString, ussdLen );

    MFREE(ussdString);

    /* start new transaction */
    ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
    ssShrdPrm.stb[sId].curCmd     = AT_CMD_CUSD;
    ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;
    psaSS_NewTrns(sId);

    CCD_END;

    retCd = AT_CMPL;


  }
  else
  {
    CCD_START;

    psaSS_asmCnfUSSDReq( (UBYTE)dcs, str->data, str->len );

    ssShrdPrm.stb[sId].ussdReqFlg = FALSE;

    /* continue existing transaction */
    psaSS_CntTrns(sId);

    CCD_END;

    retCd = AT_CMPL;
  }

  ssShrdPrm.ussdLen = 0;

  /* save ussd string for possible version 1 retry */
  if( cmh_getAlphabetCb( (UBYTE)dcs ) EQ 0 ) /* 7bit alphabet */
  {
    if( str->len <= MAX_USSD_STRING )
    {
      ssShrdPrm.ussdLen = str->len;
      memcpy( ssShrdPrm.ussdBuf, str->data, str->len );
    }
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  {
  T_ACI_CLOG   cmdLog;  /* holds logging info */

  cmdLog.atCmd                = AT_CMD_CUSD;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = retCd;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = sId+1;
  cmdLog.cmdPrm.sCUSD.srcId   = srcId;
  cmdLog.cmdPrm.sCUSD.dcs     = dcs;
  cmdLog.cmdPrm.sCUSD.str     = str;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( retCd );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : sAT_PercentCCBS          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CCBS AT command
            which is responsible to clear CCBS entries. Index value set
            to 0 indicates clear all entries.

            <idx>   : ccbs index
*/

GLOBAL T_ACI_RETURN sAT_PercentCCBS( T_ACI_CMD_SRC srcId,
                                     SHORT         idx  )
{
  SHORT sId;         /* holds service id */

  T_ACI_RETURN   retVal;
 
  TRACE_FUNCTION ("sAT_PercentCCBS ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter idx
 *-------------------------------------------------------------------
 */
  if( idx EQ ACI_NumParmNotPresent OR idx > 5 OR idx < 0 )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/* SIM TOOLKIT & FDN HANDLING */
    retVal = cmhSS_CCBS_SAT_Handle( srcId, CCBS_MOD_Erasure, idx);

    if( retVal NEQ AT_CMPL )
        return( retVal );
  

/*
 *-------------------------------------------------------------------
 * get a new service table entry to interrogate SS
 *-------------------------------------------------------------------
 */
  sId = psaSS_stbNewEntry();

  if( sId EQ NO_ENTRY )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_SrvTabFull );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter <mode>
 *-------------------------------------------------------------------
 */
  CCD_START;

  if( idx EQ 0 ) idx = KSD_IDX_NONE;

  psaSS_asmDeactivateCCBS((UBYTE)idx);

  ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
  ssShrdPrm.stb[sId].ssCode     = SS_CD_CCBS;
  ssShrdPrm.stb[sId].SSver      = MNCC_SS_VERSION_3;

  ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;

  ssShrdPrm.stb[sId].curCmd     = AT_CMD_CCBS;

  psaSS_NewTrns(sId);

  CCD_END;

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  {
  T_ACI_CLOG cmdLog;        /* holds logging info */

  cmdLog.atCmd                = AT_CMD_CCBS;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = sId+1;
  cmdLog.cmdPrm.sCCBS.srcId   = srcId;
  cmdLog.cmdPrm.sCCBS.idx     = idx;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}

#ifdef TI_PS_FF_AT_P_CMD_CSCN
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : sAT_PercentCSCN          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CSCN AT command
            which is responsible to set the switches for debugging 
            mode at terminal interface.
*/
GLOBAL T_ACI_RETURN sAT_PercentCSCN ( T_ACI_CMD_SRC srcId,
                                   T_ACI_SS_CSCN_MOD_STATE     ss_switch,
                                   T_ACI_SS_CSCN_MOD_DIRECTION ss_direction,
                                   T_ACI_CC_CSCN_MOD_STATE     cc_switch,
                                   T_ACI_CC_CSCN_MOD_DIRECTION cc_direction )
{
  TRACE_FUNCTION ("sAT_PercentCSCN ()");

  if(!cmh_IsVldCmdSrc (srcId))  /* check command source */
  {
    return( AT_FAIL );
  }

  if (ss_switch NEQ SS_CSCN_MOD_STATE_INVALID)
    cmhPrm[srcId].ssCmdPrm.CSCNss_mode.SsCSCNModeState     = ss_switch;

  if (ss_direction NEQ SS_CSCN_MOD_DIR_INVALID)
    cmhPrm[srcId].ssCmdPrm.CSCNss_mode.SsCSCNModeDirection = ss_direction;

  if (cc_switch NEQ CC_CSCN_MOD_STATE_INVALID)
    cmhPrm[srcId].ccCmdPrm.CSCNcc_mode.CcCSCNModeState     = cc_switch;

  if (cc_direction NEQ CC_CSCN_MOD_DIR_INVALID)
    cmhPrm[srcId].ccCmdPrm.CSCNcc_mode.CcCSCNModeDirection = cc_direction;

  return( AT_CMPL );
}
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

#ifdef TI_PS_FF_AT_P_CMD_CUSDR
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : sAT_PercentCUSDR         |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CUSDR AT command
            which is responsible for extended user/MMI response to n/w
            initaiated USSD.
*/
GLOBAL T_ACI_RETURN sAT_PercentCUSDR(T_ACI_CMD_SRC srcId, T_ACI_CUSDR_RES response)
{
  TRACE_FUNCTION ("sAT_PercentCUSDR()");

  if(!cmh_IsVldCmdSrc (srcId))  /* check command source */
  {
    return( AT_FAIL );
  }

  if(cuscfgParams.Ext_USSD_Response EQ CUSCFG_STAT_Disabled)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }

  if(Ext_USSD_Res_Pending EQ CUSDR_EXT_USSD_RES_Not_Pending OR Ext_USSD_Res_Pending_sId EQ (SHORT)NOT_PRESENT_8BIT)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }

  switch(response)
  {
  case CUSDR_RES_Ok:
    if(Ext_USSD_Res_Pending NEQ CUSDR_EXT_USSD_RES_Request)
    {
      psaSS_asmEmptyRslt();
      psaSS_CntTrns(Ext_USSD_Res_Pending_sId);
    }
      Ext_USSD_Res_Pending = CUSDR_EXT_USSD_RES_Not_Pending;
      Ext_USSD_Res_Pending_sId = NOT_PRESENT_8BIT;
      return(AT_CMPL);

  case CUSDR_RES_Unknown_Alphabet:
    psaSS_asmErrorRslt( Ext_USSD_Res_Pending_sId, ERR_UNKNOWN_ALPHA );
    psaSS_CntTrns(Ext_USSD_Res_Pending_sId);
    ssShrdPrm.stb[Ext_USSD_Res_Pending_sId].ntryUsdFlg = FALSE;
    Ext_USSD_Res_Pending = CUSDR_EXT_USSD_RES_Not_Pending;
    Ext_USSD_Res_Pending_sId = NOT_PRESENT_8BIT;
    return(AT_CMPL);

  case CUSDR_RES_Busy:
    psaSS_asmErrorRslt( Ext_USSD_Res_Pending_sId, ERR_USSD_BUSY );
    psaSS_EndTrns(Ext_USSD_Res_Pending_sId);
    ssShrdPrm.stb[Ext_USSD_Res_Pending_sId].ntryUsdFlg = FALSE;
    Ext_USSD_Res_Pending = CUSDR_EXT_USSD_RES_Not_Pending;
    Ext_USSD_Res_Pending_sId = NOT_PRESENT_8BIT;
    return(AT_CMPL);

  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );

  }

}
#endif /* TI_PS_FF_AT_P_CMD_CUSDR */

/*==== EOF ========================================================*/

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
|  Purpose :  This module provides the query functions related to the 
|             protocol stack adapter for SS.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_SSQ_C
#define CMH_SSQ_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "aci_ext_pers.h"    /* we are using personalisation extensions */
#include "aci_slock.h"          /* in order to asure interfaces */

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
#include "cmh.h"
#include "cmh_ss.h"
#include "cmh_sim.h"

#include "aci_ext_pers.h"
#include "aci_slock.h"

#ifdef FF_PHONE_LOCK
#include "sec_drv.h"
#endif

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== PROTOTYPES==================================================*/
/* Implements Measure 193 */
LOCAL T_ACI_RETURN cmhSS_queryHandler (T_ACI_CMD_SRC  srcId, 
                                       void           *stat,
                                       T_ACI_AT_CMD   atCmd,
                                       UBYTE          ssCode);
EXTERN T_ACI_RETURN cmhSS_check_oper_result(T_OPER_RET_STATUS result);

#ifdef FF_PHONE_LOCK
 EXTERN T_OPER_RET_STATUS aci_ext_get_phone_lock_satus(T_SEC_DRV_EXT_PHONE_LOCK_TYPE type,T_SEC_DRV_EXT_PHONE_LOCK_STATUS *status);
#endif
/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCCFC             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CCFC AT command
            which is responsible to query the parameters for call 
            forwarding supplementary services.

            <reason> : reason for CF.
            <class>  : class of basic service.
*/

GLOBAL T_ACI_RETURN qAT_PlusCCFC  ( T_ACI_CMD_SRC   srcId,
                                    T_ACI_CCFC_RSN  reason,
                                    T_ACI_CLASS     class_type  )
{
  SHORT sId;                /* holds service id */
  UBYTE ssCd;               /* holds ss code */

  T_ACI_RETURN   retVal;


  TRACE_FUNCTION ("qAT_PlusCCFC");

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
  if( !cmhSS_CheckClassInterr(class_type) )
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/* SIM TOOLKIT & FDN HANDLING */

    retVal = cmhSS_CF_SAT_Handle( srcId, reason, CCFC_MOD_Query, NULL, NULL, class_type, NULL, NULL, 0);

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
 * start first transaction 
 *-------------------------------------------------------------------
 */  
  CCD_START;

  psaSS_asmInterrogateSS( ssCd, SS_NO_PRM, SS_NO_PRM );

  ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
  ssShrdPrm.stb[sId].ssCode     = ssCd;
  ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;
  ssShrdPrm.stb[sId].ClassType  = class_type;

  ssShrdPrm.stb[sId].curCmd = AT_CMD_CCFC;
  cmhSS_flagTrn( sId, &(cmhPrm[srcId].ssCmdPrm.mltyTrnFlg));
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

    cmdLog.atCmd                    = AT_CMD_CCFC;
    cmdLog.cmdType                  = CLOG_TYPE_Query;
    cmdLog.retCode                  = AT_EXCT;
    cmdLog.cId                      = ACI_NumParmNotPresent;
    cmdLog.sId                      = sId+1;
    cmdLog.cmdPrm.qCCFC.srcId       = srcId;
    cmdLog.cmdPrm.qCCFC.reason      = reason;
    cmdLog.cmdPrm.qCCFC.class_type       = class_type;
    rAT_PercentCLOG( &cmdLog );
  }
#endif
  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCLCK             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CLCK AT command
            which is responsible to query the parameters for call 
            barring supplementary services.

            <fac>  : CB facility.
            <class>: class of basic service.
*/

GLOBAL T_ACI_RETURN qAT_PlusCLCK  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_FAC fac,
                                    T_ACI_CLASS    class_type,
                                    T_ACI_CLSSTAT *clsStat)
{
  UBYTE dummy_slockStat;
  TRACE_FUNCTION ("qAT_PlusCLCK");
  return qAT_PercentCLCK(srcId,fac,class_type, clsStat,&dummy_slockStat);
}

/*QAT_PERCENTCLCK add for Simlock in Riviear MFW

Added by Shen,Chao  April 16th, 2003
*/

GLOBAL T_ACI_RETURN qAT_PercentCLCK  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_FAC fac,
                                    T_ACI_CLASS    class_type,
                                    T_ACI_CLSSTAT *clsStat,
                                    UBYTE *simClockStat)
{
  SHORT sId;                /* holds service id */
  UBYTE ssCd;               /* holds ss code */
  T_ACI_RETURN   retVal;
#ifdef SIM_PERS
  T_SIMLOCK_TYPE slocktype;
  T_SIMLOCK_STATUS rlockstatus;
  T_OPER_RET_STATUS ret;
#endif


  TRACE_FUNCTION ("qAT_PercentCLCK");

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
 * check parameter <fac>
 *-------------------------------------------------------------------
 */  
  /* Implements Measure # 166 */
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
   * check parameter <class>
   *-------------------------------------------------------------------
   */  
    if( !cmhSS_CheckCbClassInterr(class_type))
    { 
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return( AT_FAIL );
    }

    if( cmhPrm[srcId].ssCmdPrm.mltyTrnFlg NEQ 0 )

      return( AT_BUSY );
  /*
   *-------------------------------------------------------------------
   * check not allowed <fac> (only possible with mode=0 i.e unlock)   
   *-------------------------------------------------------------------
   */  
    if(fac EQ FAC_Ab OR fac EQ FAC_Ag OR fac EQ FAC_Ac)
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return( AT_FAIL );
    }

/* SIM TOOLKIT & FDN HANDLING */

    retVal = cmhSS_Call_Barr_SAT_Handle( srcId, CLCK_MODE_QUERY, fac, NULL, class_type);

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
   * start first transaction
   *-------------------------------------------------------------------
   */ 
    CCD_START;

    psaSS_asmInterrogateSS( ssCd, SS_NO_PRM, SS_NO_PRM );

    ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
    ssShrdPrm.stb[sId].ssCode     = ssCd;
    ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;
    ssShrdPrm.stb[sId].ClassType  = class_type;

    ssShrdPrm.stb[sId].curCmd = AT_CMD_CLCK;
    cmhSS_flagTrn( sId, &(cmhPrm[srcId].ssCmdPrm.mltyTrnFlg));
    psaSS_NewTrns(sId);

    CCD_END;
  }

  /*
   *-------------------------------------------------------------------
   * if action is related to SIM
   *-------------------------------------------------------------------
   */  
  else
  {
    switch (fac)
    {
      /*
       *---------------------------------------------------------------
       * access PIN 1 status
       *---------------------------------------------------------------
       */  
      case FAC_Sc:
        clsStat -> class_type  = CLASS_NotPresent; 
        clsStat -> status = STATUS_NotPresent;

        switch( simShrdPrm.PEDStat )
        {
          case( PEDS_ENA ): clsStat -> status = STATUS_Active;
            return( AT_CMPL );
          case( PEDS_DIS ): clsStat -> status = STATUS_NotActive;
            return( AT_CMPL );
          default:
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimFail );
            return( AT_FAIL );
        }
       /* break is removed ,as case is returning before break so it is not needed */
      /*
       *---------------------------------------------------------------
       * access fixed dialling feature
       *---------------------------------------------------------------
       */
      case ( FAC_Fd ):
        clsStat -> class_type  = CLASS_NotPresent;
        clsStat -> status = STATUS_NotPresent;

        if( simShrdPrm.SIMStat EQ SS_OK )
        {
          switch( simShrdPrm.crdFun )
          {
            case( SIM_ADN_ENABLED ): 
            case( SIM_ADN_BDN_ENABLED ): clsStat -> status = STATUS_NotActive;
               return( AT_CMPL );
            case( SIM_FDN_ENABLED ): 
            case( SIM_FDN_BDN_ENABLED ): clsStat -> status = STATUS_Active;
               return( AT_CMPL );
            case( SIM_NO_OPERATION ): 
            default:

              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimFail );
              return( AT_FAIL );
          }
        }
        else
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimFail );
          return( AT_FAIL );
        }
       /* break is removed ,as case is returning before break so it is not needed */
      /*
       *---------------------------------------------------------------
       * lock ALS setting with PIN2
       *---------------------------------------------------------------
       */
      case FAC_Al:
        clsStat -> class_type  = CLASS_NotPresent; 
        clsStat -> status = STATUS_NotPresent;

        switch( ALSlock )
        {
          case( ALS_MOD_SPEECH ):
          case( ALS_MOD_AUX_SPEECH ):
            clsStat -> status = STATUS_Active;
            return( AT_CMPL );
            
          case( ALS_MOD_NOTPRESENT ):
            clsStat -> status = STATUS_NotActive;
            return( AT_CMPL );
            
          default:
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotSupp );
            return( AT_FAIL );
        }
           /* break is removed ,as case is returning before break so it is not needed */
#ifdef SIM_PERS
      /*
       *---------------------------------------------------------------
       * ME Personalisation
       *---------------------------------------------------------------
       */
            
      case FAC_Pn:
      case FAC_Pu:
      case FAC_Pp:
      case FAC_Pc:
      case FAC_Ps:
      case FAC_Pf:
      case FAC_Bl:

        clsStat -> class_type  = CLASS_NotPresent; 
        clsStat -> status = STATUS_NotPresent;
      
        switch (fac)
        {
          case FAC_Pn: slocktype = SIMLOCK_NETWORK; break;
          case FAC_Pu: slocktype = SIMLOCK_NETWORK_SUBSET; break;
          case FAC_Pp: slocktype = SIMLOCK_SERVICE_PROVIDER; break;
          case FAC_Pc: slocktype = SIMLOCK_CORPORATE; break;
          case FAC_Ps: slocktype = SIMLOCK_SIM; break;
          case FAC_Pf: slocktype = SIMLOCK_FIRST_SIM; break;
          case FAC_Bl: slocktype = SIMLOCK_BLOCKED_NETWORK; break;
          default: slocktype = SIMLOCK_NETWORK;
        }
   aci_ext_personalisation_init();
        rlockstatus = aci_personalisation_get_status(slocktype);  /* Changed to aci_personalisatio_get_status 
                                                                 from aci_ext_personalisatio_get_status on 11/03/2005 */
   aci_ext_personalisation_free();
   if (rlockstatus EQ SIMLOCK_ENABLED)
   {
          clsStat -> status = STATUS_Active;
     return( AT_CMPL );
   }
   else if (rlockstatus EQ SIMLOCK_DISABLED)
   {
     clsStat -> status = STATUS_NotActive;
          return( AT_CMPL );
   }
   else
   {
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
          return( AT_FAIL );
   }

    case( FAC_Mu )  :
    case( FAC_Mum ) :
    {
      clsStat->class_type = CLASS_NotPresent;
      clsStat->status     = STATUS_NotPresent;
      ret = aci_slock_master_unlock( NULL );
      switch (ret)
      {
        case OPER_SUCCESS:
        case OPER_WRONG_PASSWORD:
          clsStat->status = STATUS_Active;
          return( AT_CMPL );
        case OPER_NOT_ALLOWED:
          clsStat->status = STATUS_NotActive;
          return( AT_CMPL ); 
        case OPER_FAIL:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme,CME_ERR_Unknown );
          return( AT_FAIL );
        default:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme,CME_ERR_Unknown );
          return( AT_FAIL );            
      }
    }
#endif

#ifdef FF_PHONE_LOCK
   case(FAC_Pl) :
    {
      clsStat->class_type = CLASS_NotPresent;
      clsStat->status     = STATUS_NotPresent;
      ret = aci_ext_get_phone_lock_satus(PHONE_LOCK,(T_SEC_DRV_EXT_PHONE_LOCK_STATUS *)&clsStat->status);
      return(cmhSS_check_oper_result(ret));
    }
   case(FAC_Apl) :
    {
      clsStat->class_type = CLASS_NotPresent;
      clsStat->status     = STATUS_NotPresent;
      ret = aci_ext_get_phone_lock_satus(AUTO_LOCK,(T_SEC_DRV_EXT_PHONE_LOCK_STATUS *)&clsStat->status);
      return(cmhSS_check_oper_result(ret));
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

  cmdLog.atCmd                   = AT_CMD_CLCK;
  cmdLog.cmdType                 = CLOG_TYPE_Query;
  cmdLog.retCode                 = AT_EXCT;
  cmdLog.cId                     = ACI_NumParmNotPresent;
  cmdLog.sId                     = sId+1;
  cmdLog.cmdPrm.qCLCK.srcId      = srcId;
  cmdLog.cmdPrm.qCLCK.fac        = fac;
  cmdLog.cmdPrm.qCLCK.class_type = class_type;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCCWA             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CCWA AT command
            which is responsible to query the parameters for call
            waiting supplementary services.

            <class>   : class of basic service.
*/

GLOBAL  T_ACI_RETURN qAT_PlusCCWA (T_ACI_CMD_SRC  srcId, 
                                   T_ACI_CLASS    class_type)
{
  SHORT sId;         /* holds service id */

  T_ACI_RETURN   retVal;

  TRACE_FUNCTION ("qAT_PlusCCWA ()");

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
  if( !cmhSS_CheckClassInterr(class_type) )
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/* SIM TOOLKIT & FDN HANDLING */
    retVal = cmhSS_CW_SAT_Handle( srcId, CCFC_MOD_Query, class_type);

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
 * start first transaction
 *-------------------------------------------------------------------
 */ 
  CCD_START;

  psaSS_asmInterrogateSS( SS_CD_CW, SS_NO_PRM, SS_NO_PRM );

  ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
  ssShrdPrm.stb[sId].ssCode     = SS_CD_CW;
  ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;
  ssShrdPrm.stb[sId].ClassType  = class_type;

  ssShrdPrm.stb[sId].curCmd = AT_CMD_CCWA;
  cmhSS_flagTrn( sId, &(cmhPrm[srcId].ssCmdPrm.mltyTrnFlg));
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

    cmdLog.atCmd                   = AT_CMD_CCWA;
    cmdLog.cmdType                 = CLOG_TYPE_Query;
    cmdLog.retCode                 = AT_EXCT;
    cmdLog.cId                     = ACI_NumParmNotPresent;
    cmdLog.sId                     = sId+1;
    cmdLog.cmdPrm.qCCWA.srcId      = srcId;
    cmdLog.cmdPrm.qCCWA.class_type = class_type;

    rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCLIP             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CLIP AT command
            which is responsible to query the setting for calling
            line indication supplementary services.

            <stat>   : CLIP status.
*/

GLOBAL T_ACI_RETURN qAT_PlusCLIP  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CLIP_STAT * stat)
{
  /* Implements Measure 193 */
  TRACE_FUNCTION ("qAT_PlusCLIP ()");
  return (cmhSS_queryHandler (srcId, 
                              (void*)stat,
                              AT_CMD_CLIP,
                              SS_CD_CLIP) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCLIR             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CLIR AT command
            which is responsible to query the setting for calling
            line restriction supplementary services.

            <mode>   : CLIR mode.
            <stat>   : CLIR status.
*/

GLOBAL T_ACI_RETURN qAT_PlusCLIR  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CLIR_MOD * mode,
                                    T_ACI_CLIR_STAT * stat)
{
  SHORT sId;                /* holds service id */

   T_ACI_RETURN   retVal;
 
  TRACE_FUNCTION ("qAT_PlusCLIR ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

/* SIM TOOLKIT & FDN HANDLING */
    retVal = cmhSS_CLIR_SAT_Handle( srcId);

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
 * set up  facility information element
 *-------------------------------------------------------------------
 */ 
  CCD_START;

  psaSS_asmInterrogateSS( SS_CD_CLIR, SS_NO_PRM, SS_NO_PRM );
/*
 *-----------------------------------------------------------------
 * declare service table entry as used and the owner of the service
 *-----------------------------------------------------------------
 */  
  ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
  ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;

/*
 *-------------------------------------------------------------------
 * start a new transaction 
 *-------------------------------------------------------------------
 */  
  ssShrdPrm.stb[sId].curCmd = AT_CMD_CLIR;
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

   cmdLog.atCmd              = AT_CMD_CLIR;
   cmdLog.cmdType            = CLOG_TYPE_Query;
   cmdLog.retCode            = AT_EXCT;
   cmdLog.cId                = ACI_NumParmNotPresent;
   cmdLog.sId                = sId+1;
   cmdLog.cmdPrm.qCLIR.srcId = srcId;
   cmdLog.cmdPrm.qCLIR.stat  = stat;
   cmdLog.cmdPrm.qCLIR.mode  = mode;
   rAT_PercentCLOG( &cmdLog );
 }
#endif

  return( AT_EXCT );
}


GLOBAL T_ACI_RETURN qAT_PercentCLIR  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_CLIR_MOD * mode)
{
   T_CC_CMD_PRM  *pCCCmdPrm;  /* points to CC command parameters */
   TRACE_FUNCTION ("qAT_PercentCLIR ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

   pCCCmdPrm  = &cmhPrm[srcId].ccCmdPrm;   
   *mode = pCCCmdPrm->CLIRmode;
  
   return AT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCOLP             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +COLP AT command
            which is responsible to query the setting for connected
            line presentation supplementary services.

            <stat>   : COLP status.
*/

GLOBAL T_ACI_RETURN qAT_PlusCOLP  ( T_ACI_CMD_SRC srcId,
                                    T_ACI_COLP_STAT * stat)
{
  /* Implements Measure 193 */
  TRACE_FUNCTION ("qAT_PercentCOLP ()");
  return (cmhSS_queryHandler (srcId, 
                              (void*)stat,
                              AT_CMD_COLP,
                              SS_CD_COLP) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCOLR          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %COLR AT command
            which is responsible to query the setting for connected
            line restriction supplementary services.

            <stat>   : COLR status.
*/

GLOBAL T_ACI_RETURN qAT_PercentCOLR  ( T_ACI_CMD_SRC srcId )
{
  /* Implements Measure 193 */
  TRACE_FUNCTION ("qAT_PercentCOLR ()");
  return (cmhSS_queryHandler (srcId, 
                              NULL,
                              AT_CMD_COLR,
                              SS_CD_COLR));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCCBS          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CCBS AT command
            which is responsible to query the setting for connected
            line restriction supplementary services.

*/

GLOBAL T_ACI_RETURN qAT_PercentCCBS  ( T_ACI_CMD_SRC srcId )
{
  SHORT sId;                /* holds service id */

   T_ACI_RETURN   retVal;
 

  TRACE_FUNCTION ("qAT_PercentCCBS ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

/* SIM TOOLKIT & FDN HANDLING */
    retVal = cmhSS_CCBS_SAT_Handle( srcId, CCBS_MOD_Query, -1);

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
 * set up  facility information element
 *-------------------------------------------------------------------
 */ 
  CCD_START;

  psaSS_asmInterrogateSS( SS_CD_CCBS, SS_NO_PRM, SS_NO_PRM );
/*
 *-----------------------------------------------------------------
 * declare service table entry as used and the owner of the service
 *-----------------------------------------------------------------
 */  
  ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
  ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;

/*
 *-------------------------------------------------------------------
 * start a new transaction 
 *-------------------------------------------------------------------
 */  
  ssShrdPrm.stb[sId].curCmd = AT_CMD_CCBS;
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

  cmdLog.atCmd              = AT_CMD_CCBS;
  cmdLog.cmdType            = CLOG_TYPE_Query;
  cmdLog.retCode            = AT_EXCT;
  cmdLog.cId                = ACI_NumParmNotPresent;
  cmdLog.sId                = sId+1;
  cmdLog.cmdPrm.qCCBS.srcId = srcId;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCNAP          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CNAP AT command
            which is responsible to query the setting for calling
            name presentation supplementary services.

*/

GLOBAL T_ACI_RETURN qAT_PercentCNAP  ( T_ACI_CMD_SRC srcId )
{
  /* Implements Measure 193 */
  TRACE_FUNCTION ("qAT_PercentCNAP ()");
  return (cmhSS_queryHandler (srcId, 
                              NULL,
                              AT_CMD_CNAP,
                              SS_CD_CNAP) );
}


#ifdef TI_PS_FF_AT_P_CMD_CSCN
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCSCN          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CSCN AT command
            which is responsible to query the settings.

*/
GLOBAL T_ACI_RETURN qAT_PercentCSCN ( T_ACI_CMD_SRC srcId,
                                   T_ACI_SS_CSCN_MOD_STATE     *ss_switch,
                                   T_ACI_SS_CSCN_MOD_DIRECTION *ss_direction,
                                   T_ACI_CC_CSCN_MOD_STATE     *cc_switch,
                                   T_ACI_CC_CSCN_MOD_DIRECTION *cc_direction )
{
  TRACE_FUNCTION ("qAT_PercentCSCN ()");

  if(!cmh_IsVldCmdSrc (srcId))
  { /* check command source */
    return( AT_FAIL );
  }

  *ss_switch    = cmhPrm[srcId].ssCmdPrm.CSCNss_mode.SsCSCNModeState;
  *ss_direction = cmhPrm[srcId].ssCmdPrm.CSCNss_mode.SsCSCNModeDirection;
  *cc_switch    = cmhPrm[srcId].ccCmdPrm.CSCNcc_mode.CcCSCNModeState;
  *cc_direction = cmhPrm[srcId].ccCmdPrm.CSCNcc_mode.CcCSCNModeDirection;

  return( AT_CMPL);
}
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

#ifdef SIM_PERS
/*
+===========================================================+
| PROJECT : GSM-PS (6147)      MODULE  : CMH_SSQ            |
| STATE   : code               ROUTINE : qAT_PercentMEPD    |
|                                                           |
|This is the functional counterpart to the %MEPD AT command |
|which is responsible to query MEPD Configuration Data.     |
+===========================================================+
*/



GLOBAL T_ACI_RETURN qAT_PercentMEPD( T_ACI_CMD_SRC srcId, T_SUP_INFO *sup_info)
{
  T_OPER_RET_STATUS rlockstatus;

  TRACE_FUNCTION ("qAT_ PercentMEPD()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }
  rlockstatus=aci_slock_sup_info(sup_info);

  if (rlockstatus EQ OPER_SUCCESS)
  {
    return (AT_CMPL);
  }
  else if (rlockstatus EQ OPER_FAIL)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
    return( AT_FAIL );
  }
  return( AT_EXCT );
}
#endif /* SIM_PERS */




/* Implements Measure 193 */
/*
+=============================================================+
| PROJECT : GSM-PS (6147)      MODULE  : CMH_SSQ              |
| STATE   : code               ROUTINE : cmhSS_queryHandler   |
| PARAMETERS  : srcId - Source of AT Command                  |
|               stat - CLIP or COLP provisioning status       |
|               atCmd - AT Command sent for querying          |
|               ssCode - Supplementary service code           | 
| RETURN      : Query status                                  |
|This contains common code for handling CLIP, COLP, COLR, CNAP|
+=============================================================+
*/
LOCAL T_ACI_RETURN cmhSS_queryHandler (T_ACI_CMD_SRC  srcId, 
                                       void           *stat,
                                       T_ACI_AT_CMD   atCmd,
                                       UBYTE          ssCode)
{
  SHORT sId;                /* holds service id */

  T_ACI_RETURN   retVal = AT_FAIL;
 
  TRACE_FUNCTION ("cmhSS_queryHandler ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

/* SIM TOOLKIT & FDN HANDLING */
   switch(atCmd)
   {
     case AT_CMD_CLIP:
      retVal = cmhSS_CLIP_SAT_Handle(srcId);
      break;
     case AT_CMD_COLP:
      retVal = cmhSS_COLP_SAT_Handle(srcId);
      break;
    case AT_CMD_COLR:
      retVal = cmhSS_COLR_SAT_Handle(srcId);
      break;
     case AT_CMD_CNAP:
      retVal = cmhSS_CNAP_SAT_Handle(srcId);
      break;
   }

    if( retVal NEQ AT_CMPL )
    {
      return( retVal );
    }
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
 * set up  facility information element
 *-------------------------------------------------------------------
 */ 
  CCD_START;

  psaSS_asmInterrogateSS( ssCode, SS_NO_PRM, SS_NO_PRM );
/*
 *-----------------------------------------------------------------
 * declare service table entry as used and the owner of the service
 *-----------------------------------------------------------------
 */  
  ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
  ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;

/*
 *-------------------------------------------------------------------
 * start a new transaction 
 *-------------------------------------------------------------------
 */  
  ssShrdPrm.stb[sId].curCmd = atCmd;
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

  cmdLog.atCmd              = atCmd;
  cmdLog.cmdType            = CLOG_TYPE_Query;
  cmdLog.retCode            = AT_EXCT;
  cmdLog.cId                = ACI_NumParmNotPresent;
  cmdLog.sId                = sId+1;
  switch(atCmd)
  {
    case AT_CMD_CLIP:    
      cmdLog.cmdPrm.qCLIP.srcId = srcId;
      cmdLog.cmdPrm.qCLIP.stat  = (T_ACI_CLIP_STAT *)stat;
      break;
    case AT_CMD_COLP:
      cmdLog.cmdPrm.qCOLP.srcId = srcId;
      cmdLog.cmdPrm.qCOLP.stat  = (T_ACI_COLP_STAT *)stat;
      break;
    default:
      break;
  }

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}

/*==== EOF ========================================================*/

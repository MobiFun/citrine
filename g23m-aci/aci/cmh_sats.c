/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SATS
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
|             SIM application toolkit module.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef SIM_TOOLKIT

#ifndef CMH_SATS_C
#define CMH_SATS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"

#ifdef GPRS
#include "gaci_cmh.h"
#endif

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "psa_cc.h"
#include "psa_sat.h"
#include "psa_sim.h"
#include "cmh.h"
#include "cmh_sat.h"

#include "cmh_cc.h"
/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SATS                 |
| STATE   : code                  ROUTINE : sAT_PercentSATC          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %SATC AT command
            which is responsible to configure the SIM toolkit profile. 
*/

GLOBAL T_ACI_RETURN sAT_PercentSATC ( T_ACI_CMD_SRC    srcId,
                                      SHORT len, UBYTE *satCnfg )
{
  SHORT idx;          /* holds profile index */
  UBYTE *mmiMask;
  
  TRACE_FUNCTION ("sAT_PercentSATC()");
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if (!cmh_IsVldCmdSrc (srcId))
    return AT_FAIL;

/*
 *-------------------------------------------------------------------
 * check parameters
 *-------------------------------------------------------------------
 */
  if (len < 0 OR !satCnfg)
    return ( AT_FAIL );

  if (len > MAX_STK_PRF)
  {
    for (idx = MAX_STK_PRF; idx < len; idx++)
    {
      if (satCnfg[idx] NEQ 0)   /* TP bit set beyond MAX_STK_PRF? */
        return AT_FAIL;
    }
  }
/*
 *-------------------------------------------------------------------
 * update SIM toolkit profile
 *-------------------------------------------------------------------
 */
#ifdef TI_PS_FF_AT_P_CMD_CUST
 if (simShrdPrm.setPrm[srcId].cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1)
    mmiMask = satMaskCust1Prfl;
 else
#endif /* TI_PS_FF_AT_P_CMD_CUST */
    mmiMask = satMaskMMIPrfl;
    
  for( idx = 0; idx < MAX_STK_PRF; idx++ )
  {
    simShrdPrm.setPrm[srcId].STKprof[idx] =
      ((idx < len)? (satCnfg[idx] & mmiMask[idx]): 0)
       | satDefPrfl[idx];
  }
  return AT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SATS                 |
| STATE   : code                  ROUTINE : sAT_PercentSATR          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %SATR AT command
            which is responses to a previous SIM toolkit command. 
*/

GLOBAL T_ACI_RETURN sAT_PercentSATR ( T_ACI_CMD_SRC    srcId,
                                      SHORT len, UBYTE *satCmd )
{
  BOOL sendRes = TRUE; /* holds indicator whether to send terminal res or not */
  
  TRACE_FUNCTION ("sAT_PercentSATR()");
/*
 *-------------------------------------------------------------------
 * send STK command
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
    
    return( AT_FAIL );

/*
 *-------------------------------------------------------------------
 * update SIM toolkit profile
 *-------------------------------------------------------------------
 */  
  if( len > MAX_SAT_CMD_LEN OR !satCmd ) return ( AT_FAIL );

  satShrdPrm.setPrm[srcId].stkCmd = satCmd;
  satShrdPrm.setPrm[srcId].stkCmdLen = (UBYTE)len;
  satShrdPrm.owner = srcId;

#ifdef TI_PS_FF_AT_P_CMD_CUST
/*
 *-------------------------------------------------------------------
 * If Cust1 and Refresh Response Expected
 *-------------------------------------------------------------------
 */
 if ((simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1) AND
     (satShrdPrm.cust1SimRefreshRespRqd EQ TRUE))
 {
    psaSAT_SendRefreshUserRes(len, satCmd);
    satShrdPrm.cust1SimRefreshRespRqd = FALSE;
    return(AT_CMPL);
 }
#endif /* TI_PS_FF_AT_P_CMD_CUST */

  /*
   * check if MMI answers a setup event list and process it 
   */
  sendRes = cmhSAT_CheckSetEventResp( );

  if( sendRes )
  {
    if( psaSAT_STKResponse() < 0 )  /* respond to STK command */
    {
      TRACE_EVENT( "FATAL RETURN psaSIM in %%SATR" );
      return( AT_FAIL );
    }
  }
  return (AT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SATS                 |
| STATE   : code                  ROUTINE : sAT_PercentSATE          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %SATE AT command
            which envelopes a SIM toolkit command. 
*/

GLOBAL T_ACI_RETURN sAT_PercentSATE ( T_ACI_CMD_SRC    srcId,
                                      SHORT len, UBYTE *satCmd )
{
  
  TRACE_FUNCTION ("sAT_PercentSATE()");
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
    
    return( AT_FAIL );

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */  
  if( satEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * send STK command
 *-------------------------------------------------------------------
 */  
  if( len > MAX_SAT_CMD_LEN OR !satCmd ) return ( AT_FAIL );

  satShrdPrm.setPrm[srcId].stkCmd = satCmd;
  satShrdPrm.setPrm[srcId].stkCmdLen = (UBYTE)len;

  satEntStat.curCmd     = AT_CMD_SATE;
  satShrdPrm.owner = (UBYTE)srcId;
  satEntStat.entOwn = srcId;

  if( psaSAT_STKEnvelope (NULL) < 0 )  /* envelope STK command */
  {
    TRACE_EVENT( "FATAL RETURN psaSIM in %%SATE" );
    return( AT_FAIL );
  }

  return (AT_EXCT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SATS                 |
| STATE   : code                  ROUTINE : sAT_PercentSATT          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %SATT AT command
            which terminates a SAT session. 
*/

GLOBAL T_ACI_RETURN sAT_PercentSATT( T_ACI_CMD_SRC  srcId,
                                     T_ACI_SATT_CS  cause )
{
  SHORT cId, dtmfId;
  T_ACI_SAT_TERM_RESP resp_data;
  
  psaSAT_InitTrmResp( &resp_data );

  TRACE_FUNCTION ("sAT_PercentSATT()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId))

    return( AT_FAIL );


  cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_SAT_REQ, NO_VLD_CT );

  if( cId EQ NO_ENTRY )
  {
    /* Try to stop processing of proactive command SEND DTMF */
    dtmfId = cmhCC_find_call_for_DTMF();
    if( dtmfId NEQ NO_ENTRY )
    {
      if (psaCC_ctb(dtmfId)->dtmfSrc EQ OWN_SRC_SAT)
      {
        ccShrdPrm.dtmf.cnt = 0; /* to avoid double terminal response to SAT */
        psaCC_StopDTMF( dtmfId );
        psaSAT_SendTrmResp( RSLT_SESS_USR_TERM, &resp_data );
        return( AT_CMPL );
      }
      /* else it means there is a Send DTMF on process */
    }
  }

#ifdef FF_SAT_E 
  if( cId EQ NO_ENTRY )

    cId = psaCC_ctbFindCall( OWN_SRC_INV, CS_SAT_CSD_REQ, NO_VLD_CT );

  if( (cId EQ NO_ENTRY OR !psaCC_ctb(cId)->SATinv) AND
     !( satShrdPrm.opchStat NEQ OPCH_IDLE AND
       (satShrdPrm.cmdDet.cmdType EQ SAT_CMD_OPEN_CHANNEL OR
        satShrdPrm.cmdDet.cmdType EQ SAT_CMD_SEND_DATA))
      )
    {
      return( AT_FAIL );
    }
#else

  if( cId EQ NO_ENTRY OR
      !psaCC_ctb(cId)->SATinv OR
      psaCC_ctb(cId)->calStat NEQ CS_SAT_REQ)
  {
    return( AT_FAIL );
  }
#endif /* else, #ifdef FF_SAT_E */

/*
 *-------------------------------------------------------------------
 * check termination cause
 *-------------------------------------------------------------------
 */  
  switch( cause )
  {
    case( SATT_CS_UserRedialStop ):
    case( SATT_CS_EndRedial ):

      if ((cId EQ NO_ENTRY) OR 
          (cause EQ SATT_CS_UserRedialStop))
      {
        /* respond with "user cleared down..." */
        psaSAT_SendTrmResp( RSLT_USR_CLR_DWN, &resp_data );
      }
      else
      {
        /* return network error cause GSM 11.14 / 12.12.3 */
        /*
         * Use ctb here because TI compiler 1.22e may have a problem otherwise here.
         * See cmhCC_SndDiscRsn() for the details.
         */  
        cmhSAT_NtwErr ((UBYTE)((GET_CAUSE_VALUE(psaCC_ctb(cId)->nrmCs) NEQ NOT_PRESENT_8BIT) ? 
                       (psaCC_ctb(cId)->nrmCs|0x80) : ADD_NO_CAUSE));
      }
      break;
    case( SATT_CS_EndSession ):
      /* respond with "session terminated by user" */
      psaSAT_SendTrmResp( RSLT_SESS_USR_TERM, &resp_data );
      break;
  }

  if( cId NEQ NO_ENTRY ) 
    psaCC_FreeCtbNtry (cId);

#ifdef FF_SAT_E
  if( satShrdPrm.opchStat NEQ OPCH_IDLE AND
     (satShrdPrm.cmdDet.cmdType EQ SAT_CMD_OPEN_CHANNEL OR
      satShrdPrm.cmdDet.cmdType EQ SAT_CMD_SEND_DATA) )
  {
    cmhSAT_cleanupOpChnPrms();
    satShrdPrm.chnTb.chnUsdFlg = FALSE;
  }
#endif /* FF_SAT_E */

  return( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SATS                |
|                                 ROUTINE : sAT_PercentEFRSLT       |
+-------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_RETURN sAT_PercentEFRSLT (T_ACI_CMD_SRC srcId,
                                       T_ACI_EFRSLT_RES result)
{
  TRACE_FUNCTION("sAT_PercentEFRSLT()");

  if(!cmh_IsVldCmdSrc(srcId))
  {
    return(AT_FAIL);
  }

  switch(result)
  {
    case EFRSLT_RES_FAIL:
      psaSAT_FUConfirm(simShrdPrm.fuRef,SIM_FU_ERROR);
      break;

    case EFRSLT_RES_OK:
      psaSAT_FUConfirm(simShrdPrm.fuRef,SIM_FU_SUCCESS);
      break;

    default:
      ACI_ERR_DESC(ACI_ERR_CLASS_Ext,EXT_ERR_Parameter);
      return(AT_FAIL);
  }

  return(AT_CMPL);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SATS                |
|                                 ROUTINE : sAT_PercentSIMEF        |
+-------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_RETURN sAT_PercentSIMEF (T_ACI_CMD_SRC srcId,
                                      T_ACI_SIMEF_MODE mode)
{
  TRACE_FUNCTION("sAT_PercentSIMEF()");

  if(!cmh_IsVldCmdSrc(srcId))
  {
    return(AT_FAIL);
  }

  switch(mode)
  {
    case SIMEF_MODE_ON:
    case SIMEF_MODE_OFF:
      simShrdPrm.SIMEFMode[srcId]=mode;
      break;

    default:
      ACI_ERR_DESC(ACI_ERR_CLASS_Ext,EXT_ERR_Parameter);
      return(AT_FAIL);
  }
  return(AT_CMPL);
}

#endif /* #ifdef SIM_TOOLKIT */

/*==== EOF ========================================================*/
